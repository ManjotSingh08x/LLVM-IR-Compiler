#include "passes/loop_pass.h"

#include <algorithm>
#include <map>

namespace passes {

using namespace ir;

bool LoopContext::isDefinedInLoop(const Value *value, const Loop &loop) const {
    if (value->getKind() != ValueKind::Instruction) return false;
    const BasicBlock *block = static_cast<const Instruction *>(value)->getParent();
    return block != nullptr && loop.contains(block);
}

bool LoopContext::dominatesAllExits(const BasicBlock *block, const Loop &loop) const {
    for (const BasicBlock *exit : loop.exits) {
        if (!doms.dominates(block, exit)) return false;
    }
    return true;
}

BasicBlock *redirectPredecessors(Function &fn, BasicBlock *target, const std::vector<BasicBlock *> &preds,
                                 const std::string &label, BasicBlock *placeBefore) {
    BasicBlock *middle = fn.createBlock(label);
    if (placeBefore != nullptr) {
        fn.placeBlockBefore(middle, placeBefore);
    } else {
        BasicBlock *last = preds.front();
        for (BasicBlock *block : fn.getBlocks()) {
            if (std::find(preds.begin(), preds.end(), block) != preds.end()) last = block;
        }
        fn.placeBlockAfter(middle, last);
    }

    //phi entries from preds now arrive through middle, merged when they differ
    for (std::unique_ptr<Instruction> &inst : target->instructions) {
        if (inst->getOpcode() != Opcode::Phi) break;
        PhiInst *phi = static_cast<PhiInst *>(inst.get());
        std::vector<std::pair<Value *, BasicBlock *>> moved;
        std::vector<std::pair<Value *, BasicBlock *>> kept;
        for (const std::pair<Value *, BasicBlock *> &in : phi->incoming) {
            bool fromPred = std::find(preds.begin(), preds.end(), in.second) != preds.end();
            (fromPred ? moved : kept).push_back(in);
        }
        if (moved.empty()) continue;
        Value *incoming = moved.front().first;
        bool same = true;
        for (const std::pair<Value *, BasicBlock *> &in : moved) same = same && in.first == incoming;
        if (!same) {
            PhiInst *merge = new PhiInst(phi->getType(), fn.uniqueName(phi->getName()));
            merge->incoming = moved;
            middle->append(std::unique_ptr<Instruction>(merge));
            incoming = merge;
        }
        kept.push_back(std::make_pair(incoming, middle));
        phi->incoming = kept;
    }

    for (BasicBlock *pred : preds) pred->getTerminator()->replaceSuccessor(target, middle);
    middle->append(std::unique_ptr<Instruction>(new BranchInst(target)));
    return middle;
}

namespace {

//a phi reads each entry at the end of that entry's block
std::vector<std::pair<Instruction *, BasicBlock *>> usesOf(const Instruction *def, const DefUse &defUse) {
    std::vector<std::pair<Instruction *, BasicBlock *>> uses;
    for (Instruction *user : defUse.users(def)) {
        if (user->getOpcode() == Opcode::Phi) {
            for (const std::pair<Value *, BasicBlock *> &in : static_cast<PhiInst *>(user)->incoming) {
                if (in.first == def) uses.push_back(std::make_pair(user, in.second));
            }
        } else {
            uses.push_back(std::make_pair(user, user->getParent()));
        }
    }
    return uses;
}

} // namespace

bool isLCSSA(const Loop &loop, const DefUse &defUse) {
    for (BasicBlock *block : loop.blocks) {
        for (std::unique_ptr<Instruction> &inst : block->instructions) {
            for (const std::pair<Instruction *, BasicBlock *> &use : usesOf(inst.get(), defUse)) {
                if (!loop.contains(use.second)) return false;
            }
        }
    }
    return true;
}

bool formLCSSA(Function &fn, const Loop &loop, const Dominators &doms, bool &changed) {
    DefUse defUse(fn);
    bool complete = true;
    std::vector<Instruction *> defs;
    for (BasicBlock *block : fn.getBlocks()) {
        if (!loop.contains(block)) continue;
        for (std::unique_ptr<Instruction> &inst : block->instructions) {
            if (!inst->getName().empty()) defs.push_back(inst.get());
        }
    }
    for (Instruction *def : defs) {
        std::vector<std::pair<std::pair<Instruction *, BasicBlock *>, BasicBlock *>> outside;
        bool usable = true;
        for (const std::pair<Instruction *, BasicBlock *> &use : usesOf(def, defUse)) {
            if (loop.contains(use.second)) continue;
            BasicBlock *walk = use.second;
            while (walk != nullptr && !loop.contains(walk) &&
                   std::find(loop.exits.begin(), loop.exits.end(), walk) == loop.exits.end()) {
                walk = doms.idom(walk);
            }
            if (walk == nullptr || loop.contains(walk)) {
                usable = false;
                break;
            }
            outside.push_back(std::make_pair(use, walk));
        }
        if (!usable) {
            complete = false;
            continue;
        }
        std::map<BasicBlock *, PhiInst *> exitPhis;
        for (const auto &entry : outside) {
            BasicBlock *exit = entry.second;
            PhiInst *&phi = exitPhis[exit];
            if (phi == nullptr) {
                for (BasicBlock *pred : exit->predecessors) {
                    if (!doms.dominates(def->getParent(), pred)) usable = false;
                }
                if (!usable) break;
                phi = new PhiInst(def->getType(), fn.uniqueName(def->getName() + ".lcssa"));
                for (BasicBlock *pred : exit->predecessors) {
                    for (BasicBlock *succ : pred->getTerminator()->getSuccessors()) {
                        if (succ == exit) phi->addIncoming(def, pred);
                    }
                }
                exit->insertAtFront(std::unique_ptr<Instruction>(phi));
                changed = true;
            }
            Instruction *user = entry.first.first;
            if (user->getOpcode() == Opcode::Phi) {
                for (std::pair<Value *, BasicBlock *> &in : static_cast<PhiInst *>(user)->incoming) {
                    if (in.first == def && in.second == entry.first.second) in.first = phi;
                }
            } else {
                user->replaceOperand(def, phi);
            }
        }
        if (!usable) complete = false;
    }
    return complete;
}

bool LoopPass::isUsable(const Loop &loop, const Function &fn) {
    return loop.header != fn.getEntryBlock();
}

bool LoopPass::canonicalize(Function &fn, AnalysisManager &am) {
    bool changedAny = false;
    //each fix changes the cfg, so the loops are found again
    for (;;) {
        bool changed = false;
        const LoopInfo &info = am.getLoops(fn);
        for (Loop *loop : info.innermostFirst()) {
            if (!isUsable(*loop, fn)) continue;

            if (loop->preheader() == nullptr) {
                redirectPredecessors(fn, loop->header, loop->outsidePredecessors(),
                                     loop->header->getName() + ".preheader", loop->header);
                changed = true;
                break;
            }

            if (loop->latches.size() > 1) {
                redirectPredecessors(fn, loop->header, loop->latches, loop->header->getName() + ".latch", nullptr);
                changed = true;
                break;
            }

            //dedicated exits: an exit also entered from outside gets its own block
            for (BasicBlock *exit : loop->exits) {
                std::vector<BasicBlock *> inside;
                bool outside = false;
                for (BasicBlock *pred : exit->predecessors) {
                    if (loop->contains(pred)) {
                        inside.push_back(pred);
                    } else {
                        outside = true;
                    }
                }
                if (!outside) continue;
                redirectPredecessors(fn, exit, inside, exit->getName() + ".loopexit", exit);
                changed = true;
                break;
            }
            if (changed) break;
        }
        if (!changed) return changedAny;
        changedAny = true;
        fn.recomputeCFG();
        am.invalidate(fn);
    }
}

bool LoopPass::closeLoops(Function &fn, AnalysisManager &am) {
    //new phis invalidate the analyses, so loops are looked up by header each time
    std::vector<BasicBlock *> headers;
    for (Loop *loop : am.getLoops(fn).innermostFirst()) {
        if (isUsable(*loop, fn)) headers.push_back(loop->header);
    }
    bool changed = false;
    for (BasicBlock *header : headers) {
        const Loop *loop = nullptr;
        for (Loop *candidate : am.getLoops(fn).innermostFirst()) {
            if (candidate->header == header) loop = candidate;
        }
        if (loop == nullptr) continue;
        bool made = false;
        formLCSSA(fn, *loop, am.getDominators(fn), made);
        if (made) {
            changed = true;
            am.invalidate(fn);
        }
    }
    return changed;
}

bool LoopPass::run(Function &fn, Module &module, AnalysisManager &am) {
    bool changed = canonicalize(fn, am);
    if (needsLCSSA() && closeLoops(fn, am)) changed = true;
    std::set<BasicBlock *> done;
    for (;;) {
        Loop *next = nullptr;
        for (Loop *loop : am.getLoops(fn).innermostFirst()) {
            if (isUsable(*loop, fn) && done.count(loop->header) == 0) {
                next = loop;
                break;
            }
        }
        if (next == nullptr) return changed;
        done.insert(next->header);

        LoopContext ctx{ fn, module, am, am.getDominators(fn), am.getDefUse(fn), done };
        LoopResult result = runOnLoop(*next, ctx);
        if (result == LoopResult::Unchanged) continue;
        changed = true;
        if (result == LoopResult::CFGChanged) fn.recomputeCFG();
        am.invalidate(fn);
        if (result == LoopResult::CFGChanged) {
            canonicalize(fn, am);
            if (needsLCSSA()) closeLoops(fn, am);
        }
    }
}

} // namespace passes
