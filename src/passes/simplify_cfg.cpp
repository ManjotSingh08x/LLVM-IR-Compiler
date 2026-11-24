#include "passes/simplify_cfg.h"

#include <algorithm>
#include <set>

namespace passes {

using namespace ir;

namespace {

std::vector<PhiInst *> phisOf(BasicBlock *block) {
    std::vector<PhiInst *> phis;
    for (std::unique_ptr<Instruction> &inst : block->instructions) {
        if (inst->getOpcode() != Opcode::Phi) break;
        phis.push_back(static_cast<PhiInst *>(inst.get()));
    }
    return phis;
}

void dropOneEntry(BasicBlock *to, BasicBlock *from) {
    for (PhiInst *phi : phisOf(to)) {
        for (size_t i = 0; i < phi->incoming.size(); i++) {
            if (phi->incoming[i].second == from) {
                phi->incoming.erase(phi->incoming.begin() + i);
                break;
            }
        }
    }
}

bool isIntConstant(const Value *value) {
    return value->getKind() == ValueKind::Constant &&
           static_cast<const Constant *>(value)->constKind == ConstantKind::Int;
}

bool foldConstantBranches(Function &fn) {
    bool changed = false;
    for (BasicBlock *block : fn.getBlocks()) {
        Instruction *term = block->getTerminator();
        if (term == nullptr) continue;
        if (term->getOpcode() == Opcode::Br) {
            BranchInst *br = static_cast<BranchInst *>(term);
            if (!br->isConditional()) continue;
            if (br->target == br->falseTarget) {
                branchTo(block, br->target);
                changed = true;
            } else if (isIntConstant(br->condition)) {
                BasicBlock *taken = static_cast<Constant *>(br->condition)->intValue != 0 ? br->target : br->falseTarget;
                branchTo(block, taken);
                changed = true;
            }
        } else if (term->getOpcode() == Opcode::Switch) {
            SwitchInst *sw = static_cast<SwitchInst *>(term);
            if (!isIntConstant(sw->condition)) continue;
            long long value = static_cast<Constant *>(sw->condition)->intValue;
            BasicBlock *taken = sw->defaultTarget;
            for (const std::pair<long long, BasicBlock *> &c : sw->cases) {
                if (c.first == value) {
                    taken = c.second;
                    break;
                }
            }
            branchTo(block, taken);
            changed = true;
        }
    }
    return changed;
}

bool bypassEmptyBlocks(Function &fn) {
    for (BasicBlock *block : fn.getBlocks()) {
        if (block == fn.getEntryBlock() || block->instructions.size() != 1) continue;
        Instruction *term = block->getTerminator();
        if (term == nullptr || term->getOpcode() != Opcode::Br) continue;
        BranchInst *br = static_cast<BranchInst *>(term);
        BasicBlock *target = br->target;
        if (br->isConditional() || target == block) continue;

        std::vector<BasicBlock *> preds;
        for (BasicBlock *pred : block->predecessors) {
            if (pred != block) preds.push_back(pred);
        }
        if (preds.empty()) continue;
        std::vector<PhiInst *> phis = phisOf(target);
        if (!phis.empty()) {
            bool clash = false;
            for (BasicBlock *pred : preds) {
                if (std::find(target->predecessors.begin(), target->predecessors.end(), pred) != target->predecessors.end()) {
                    clash = true;
                }
            }
            if (clash) continue;
            for (PhiInst *phi : phis) {
                Value *value = phi->getIncomingValueFor(block);
                phi->removeIncomingBlock(block);
                for (BasicBlock *pred : preds) {
                    for (BasicBlock *succ : pred->getTerminator()->getSuccessors()) {
                        if (succ == block) phi->addIncoming(value, pred);
                    }
                }
            }
        }
        for (BasicBlock *pred : preds) pred->getTerminator()->replaceSuccessor(block, target);
        return true;
    }
    return false;
}

bool removeUnreachableBlocks(Function &fn) {
    std::set<BasicBlock *> reached;
    std::vector<BasicBlock *> work(1, fn.getEntryBlock());
    reached.insert(fn.getEntryBlock());
    while (!work.empty()) {
        BasicBlock *block = work.back();
        work.pop_back();
        for (BasicBlock *next : block->successors) {
            if (reached.insert(next).second) work.push_back(next);
        }
    }
    std::vector<BasicBlock *> dead;
    for (BasicBlock *block : fn.getBlocks()) {
        if (!reached.count(block)) dead.push_back(block);
    }
    for (BasicBlock *block : dead) {
        Instruction *term = block->getTerminator();
        if (term == nullptr) continue;
        for (BasicBlock *succ : term->getSuccessors()) {
            for (PhiInst *phi : phisOf(succ)) phi->removeIncomingBlock(block);
        }
    }
    for (BasicBlock *block : dead) fn.removeBlock(block);
    return !dead.empty();
}

bool mergeStraightLines(Function &fn) {
    for (BasicBlock *block : fn.getBlocks()) {
        Instruction *term = block->getTerminator();
        if (term == nullptr || term->getOpcode() != Opcode::Br) continue;
        BranchInst *br = static_cast<BranchInst *>(term);
        if (br->isConditional()) continue;
        BasicBlock *next = br->target;
        if (next == block || next == fn.getEntryBlock()) continue;
        if (next->predecessors.size() != 1 || next->predecessors.front() != block) continue;

        for (PhiInst *phi : phisOf(next)) {
            Value *value = phi->incoming.empty() ? nullptr : phi->incoming.front().second == block ? phi->incoming.front().first : nullptr;
            if (value == nullptr || value == phi) return false;
            for (BasicBlock *b : fn.getBlocks()) {
                for (std::unique_ptr<Instruction> &inst : b->instructions) inst->replaceOperand(phi, value);
            }
            next->erase(phi);
        }
        if (Instruction *nextTerm = next->getTerminator()) {
            for (BasicBlock *succ : nextTerm->getSuccessors()) {
                for (PhiInst *phi : phisOf(succ)) phi->replaceIncomingBlock(next, block);
            }
        }
        block->erase(term);
        for (std::unique_ptr<Instruction> &inst : next->instructions) {
            inst->setParent(block);
            block->instructions.push_back(std::move(inst));
        }
        next->instructions.clear();
        fn.removeBlock(next);
        return true;
    }
    return false;
}

}

void branchTo(BasicBlock *block, BasicBlock *target) {
    std::vector<BasicBlock *> old = block->getTerminator()->getSuccessors();
    bool kept = false;
    for (BasicBlock *succ : old) {
        if (succ == target && !kept) {
            kept = true;
            continue;
        }
        dropOneEntry(succ, block);
    }
    block->erase(block->getTerminator());
    block->append(std::unique_ptr<Instruction>(new BranchInst(target)));
}

bool removeUnreachable(Function &fn) {
    fn.recomputeCFG();
    bool changed = removeUnreachableBlocks(fn);
    if (changed) fn.recomputeCFG();
    return changed;
}

bool SimplifyCFG::run(Function &fn, Module &module, AnalysisManager &am) {
    (void)module;
    (void)am;
    bool changedAny = false;
    for (;;) {
        fn.recomputeCFG();
        bool changed = foldConstantBranches(fn);
        if (changed) fn.recomputeCFG();
        changed = removeUnreachableBlocks(fn) || changed;
        if (!changed && bypassEmptyBlocks(fn)) changed = true;
        if (!changed && mergeStraightLines(fn)) changed = true;
        if (!changed) return changedAny;
        changedAny = true;
    }
}

}
