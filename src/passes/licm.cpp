#include "passes/licm.h"

namespace passes {

using namespace ir;

namespace {

bool isCandidate(const Instruction &inst) {
    switch (inst.getOpcode()) {
    case Opcode::Binary:
    case Opcode::Unary:
    case Opcode::Cast:
    case Opcode::Select:
    case Opcode::GEP:
        return true;
    default:
        return false;
    }
}

bool mayTrap(const Instruction &inst) {
    if (inst.getOpcode() != Opcode::Binary) return false;
    BinaryOpKind op = static_cast<const BinaryInst &>(inst).op;
    return op == BinaryOpKind::SDiv || op == BinaryOpKind::SRem || op == BinaryOpKind::UDiv ||
           op == BinaryOpKind::URem;
}

} // namespace

LoopResult LICM::runOnLoop(Loop &loop, LoopContext &ctx) {
    BasicBlock *pre = loop.preheader();
    if (pre == nullptr) return LoopResult::Unchanged;

    //repeat until no new invariant instruction is found
    std::set<const Instruction *> invariant;
    bool grew = true;
    while (grew) {
        grew = false;
        for (BasicBlock *block : loop.blocks) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                if (invariant.count(inst.get()) || !isCandidate(*inst)) continue;
                bool all = true;
                for (Value *operand : inst->getOperands()) {
                    if (!ctx.isDefinedInLoop(operand, loop)) continue;
                    if (!invariant.count(static_cast<Instruction *>(operand))) all = false;
                }
                if (!all) continue;
                //a trapping instruction must dominate every exit, and a loop with no exit never qualifies
                if (mayTrap(*inst) && (loop.exits.empty() || !ctx.dominatesAllExits(inst->getParent(), loop))) continue;
                invariant.insert(inst.get());
                grew = true;
            }
        }
    }
    if (invariant.empty()) return LoopResult::Unchanged;

    //hoist in dominator-tree order so operands reach the preheader first
    Instruction *preTerm = pre->getTerminator();
    for (BasicBlock *block : ctx.doms.preorder()) {
        if (!loop.contains(block)) continue;
        std::vector<Instruction *> moving;
        for (std::unique_ptr<Instruction> &inst : block->instructions) {
            if (invariant.count(inst.get())) moving.push_back(inst.get());
        }
        for (Instruction *inst : moving) pre->insertBefore(preTerm, block->detach(inst));
    }
    return LoopResult::Changed;
}

} // namespace passes
