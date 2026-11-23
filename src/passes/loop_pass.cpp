#include "passes/loop_pass.h"

#include <algorithm>

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

    //phis: the entries from preds now arrive through middle, merged when they differ
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

bool LoopPass::isUsable(const Loop &loop, const Function &fn) {
    return loop.header != fn.getEntryBlock();
}

bool LoopPass::canonicalize(Function &fn, AnalysisManager &am) {
    bool changedAny = false;
    //each fix changes the cfg, so the loops are found again after it
    for (;;) {
        bool changed = false;
        const LoopInfo &info = am.getLoops(fn);
        for (Loop *loop : info.innermostFirst()) {
            if (!isUsable(*loop, fn)) continue;

            //preheader: one outside predecessor that only jumps to the header
            if (loop->preheader() == nullptr) {
                redirectPredecessors(fn, loop->header, loop->outsidePredecessors(),
                                     loop->header->getName() + ".preheader", loop->header);
                changed = true;
                break;
            }

            //one latch: every back edge goes through one block
            if (loop->latches.size() > 1) {
                redirectPredecessors(fn, loop->header, loop->latches, loop->header->getName() + ".latch", nullptr);
                changed = true;
                break;
            }

            //dedicated exits: an exit also entered from outside gets its own block for the loop's edges
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

bool LoopPass::run(Function &fn, Module &module, AnalysisManager &am) {
    bool changed = canonicalize(fn, am);
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
        if (result == LoopResult::CFGChanged) canonicalize(fn, am);
    }
}

} // namespace passes
