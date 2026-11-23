#include "passes/simplify_cfg.h"

#include <set>

namespace passes {

using namespace ir;

namespace {

bool hasPhi(const Function &fn) {
    for (const BasicBlock *block : fn.getBlocks()) {
        for (const std::unique_ptr<Instruction> &inst : block->instructions) {
            if (inst->getOpcode() == Opcode::Phi) return true;
        }
    }
    return false;
}

bool isIntConstant(const Value *value) {
    return value->getKind() == ValueKind::Constant &&
           static_cast<const Constant *>(value)->constKind == ConstantKind::Int;
}

//replaces block's terminator with br target
void branchTo(BasicBlock *block, BasicBlock *target) {
    block->erase(block->getTerminator());
    block->append(std::unique_ptr<Instruction>(new BranchInst(target)));
}

//br i1 <constant>, br c, L, L and switch <constant> become br L
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

//a block that is only "br L" is skipped: whoever jumps to it jumps to L
bool bypassEmptyBlocks(Function &fn) {
    bool changed = false;
    for (BasicBlock *block : fn.getBlocks()) {
        if (block == fn.getEntryBlock() || block->instructions.size() != 1) continue;
        Instruction *term = block->getTerminator();
        if (term == nullptr || term->getOpcode() != Opcode::Br) continue;
        BranchInst *br = static_cast<BranchInst *>(term);
        if (br->isConditional() || br->target == block) continue;
        std::vector<BasicBlock *> preds = block->predecessors;
        for (BasicBlock *pred : preds) {
            if (pred == block) continue;
            pred->getTerminator()->replaceSuccessor(block, br->target);
            changed = true;
        }
        if (changed) return true;  //edges moved: recompute before looking at more blocks
    }
    return changed;
}

//blocks entry does not reach are dropped
bool removeUnreachable(Function &fn) {
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
    for (BasicBlock *block : dead) fn.removeBlock(block);
    return !dead.empty();
}

//b -> s with s entered only from b: s's instructions move to the end of b
bool mergeStraightLines(Function &fn) {
    for (BasicBlock *block : fn.getBlocks()) {
        Instruction *term = block->getTerminator();
        if (term == nullptr || term->getOpcode() != Opcode::Br) continue;
        BranchInst *br = static_cast<BranchInst *>(term);
        if (br->isConditional()) continue;
        BasicBlock *next = br->target;
        if (next == block || next == fn.getEntryBlock()) continue;
        if (next->predecessors.size() != 1 || next->predecessors.front() != block) continue;
        block->erase(term);
        for (std::unique_ptr<Instruction> &inst : next->instructions) {
            inst->setParent(block);
            block->instructions.push_back(std::move(inst));
        }
        next->instructions.clear();
        fn.removeBlock(next);
        return true;  //the edge lists are stale now
    }
    return false;
}

} // namespace

bool SimplifyCFG::run(Function &fn, Module &module, AnalysisManager &am) {
    (void)module;
    (void)am;
    if (hasPhi(fn)) return false;
    bool changedAny = false;
    for (;;) {
        fn.recomputeCFG();
        bool changed = foldConstantBranches(fn);
        if (changed) fn.recomputeCFG();
        changed = removeUnreachable(fn) || changed;
        if (!changed && bypassEmptyBlocks(fn)) changed = true;
        if (!changed && mergeStraightLines(fn)) changed = true;
        if (!changed) return changedAny;
        changedAny = true;
    }
}

} // namespace passes
