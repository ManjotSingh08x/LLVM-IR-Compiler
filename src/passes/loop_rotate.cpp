#include "passes/loop_rotate.h"

#include "passes/clone.h"

namespace passes {

using namespace ir;

namespace {

//the header is copied into the preheader, so it must stay small
const size_t kMaxHeaderInstructions = 16;

bool hasPhi(const BasicBlock *block) {
    return !block->instructions.empty() && block->instructions.front()->getOpcode() == Opcode::Phi;
}

//instructions that can be run once more before the loop without changing anything
bool isCopyable(const Instruction &inst) {
    switch (inst.getOpcode()) {
    case Opcode::Load:
    case Opcode::Binary:
    case Opcode::Unary:
    case Opcode::Cast:
    case Opcode::GEP:
    case Opcode::Select:
        return true;
    default:
        return false;
    }
}

} // namespace

LoopResult LoopRotate::runOnLoop(Loop &loop, LoopContext &ctx) {
    BasicBlock *header = loop.header;
    BasicBlock *pre = loop.preheader();
    if (pre == nullptr || loop.latches.size() != 1) return LoopResult::Unchanged;
    BasicBlock *latch = loop.latches.front();
    if (latch == header) return LoopResult::Unchanged;  //the test is already at the bottom

    Instruction *term = header->getTerminator();
    if (term == nullptr || term->getOpcode() != Opcode::Br) return LoopResult::Unchanged;
    BranchInst *branch = static_cast<BranchInst *>(term);
    if (!branch->isConditional()) return LoopResult::Unchanged;
    const bool trueStays = loop.contains(branch->target);
    const bool falseStays = loop.contains(branch->falseTarget);
    if (trueStays == falseStays) return LoopResult::Unchanged;
    BasicBlock *body = trueStays ? branch->target : branch->falseTarget;
    BasicBlock *exit = trueStays ? branch->falseTarget : branch->target;
    if (body == header || hasPhi(header) || hasPhi(body) || hasPhi(exit)) return LoopResult::Unchanged;

    if (header->instructions.size() - 1 > kMaxHeaderInstructions) return LoopResult::Unchanged;
    for (size_t i = 0; i + 1 < header->instructions.size(); i++) {
        const Instruction &inst = *header->instructions[i];
        if (!isCopyable(inst)) return LoopResult::Unchanged;
        for (const Instruction *user : ctx.defUse.users(&inst)) {
            if (user->getParent() != header) return LoopResult::Unchanged;
        }
    }

    //the guard: a copy of the header's work at the end of the preheader. what the header reads from
    //outside the loop dominates the header, so it is already available there
    ValueMap copies;
    Instruction *preTerm = pre->getTerminator();
    for (size_t i = 0; i + 1 < header->instructions.size(); i++) {
        const Instruction &inst = *header->instructions[i];
        std::unique_ptr<Instruction> copy = cloneInstruction(inst, ctx.fn.uniqueName(inst.getName()));
        remapInstruction(*copy, copies);
        copies[&inst] = copy.get();
        pre->insertBefore(preTerm, std::move(copy));
    }
    Value *condition = branch->condition;
    ValueMap::const_iterator mapped = copies.find(condition);
    if (mapped != copies.end()) condition = mapped->second;
    pre->erase(preTerm);
    BranchInst *guard = trueStays ? new BranchInst(condition, body, exit) : new BranchInst(condition, exit, body);
    pre->append(std::unique_ptr<Instruction>(guard));

    //the old header now runs after the latch: move it there in the layout so the latch falls into it
    ctx.fn.removeBlock(header);
    ctx.fn.placeBlockAfter(header, latch);

    //the loop now starts at body, do not rotate it again
    ctx.markDone(body);
    return LoopResult::CFGChanged;
}

} // namespace passes
