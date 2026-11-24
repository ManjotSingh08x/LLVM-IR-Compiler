#include "passes/sink.h"

#include "dataflow/dfa_available_exprs.h"

namespace passes {

using namespace ir;

namespace {

BasicBlock *commonDominator(const Dominators &doms, BasicBlock *a, BasicBlock *b) {
    while (a != nullptr && !doms.dominates(a, b)) a = doms.idom(a);
    return a;
}

} // namespace

bool Sink::run(Function &fn, Module &module, AnalysisManager &am) {
    (void)module;
    const Dominators &doms = am.getDominators(fn);
    const LoopInfo &loops = am.getLoops(fn);
    const DefUse &defUse = am.getDefUse(fn);
    bool changed = false;

    //bottom-up, so users move before what they use
    std::vector<BasicBlock *> order(doms.preorder().rbegin(), doms.preorder().rend());
    for (BasicBlock *block : order) {
        std::vector<Instruction *> candidates;
        for (std::unique_ptr<Instruction> &inst : block->instructions) {
            if (isExpression(*inst)) candidates.push_back(inst.get());
        }
        for (size_t i = candidates.size(); i-- > 0;) {
            Instruction *inst = candidates[i];
            BasicBlock *target = nullptr;
            bool usedHere = false;
            for (Instruction *user : defUse.users(inst)) {
                std::vector<BasicBlock *> useBlocks;
                if (user->getOpcode() == Opcode::Phi) {
                    for (const std::pair<Value *, BasicBlock *> &in : static_cast<PhiInst *>(user)->incoming) {
                        if (in.first == inst) useBlocks.push_back(in.second);
                    }
                } else {
                    useBlocks.push_back(user->getParent());
                }
                for (BasicBlock *useBlock : useBlocks) {
                    if (useBlock == block) usedHere = true;
                    target = target == nullptr ? useBlock : commonDominator(doms, target, useBlock);
                }
            }
            if (usedHere || target == nullptr || target == block || !doms.strictlyDominates(block, target)) continue;
            if (loops.depth(target) > loops.depth(block)) continue;  //never into a loop
            //after the target's phis; the users there come later in the block
            Instruction *at = nullptr;
            for (std::unique_ptr<Instruction> &other : target->instructions) {
                if (other->getOpcode() != Opcode::Phi) {
                    at = other.get();
                    break;
                }
            }
            target->insertBefore(at, block->detach(inst));
            changed = true;
        }
    }
    return changed;
}

} // namespace passes
