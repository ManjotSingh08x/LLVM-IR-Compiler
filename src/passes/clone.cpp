#include "passes/clone.h"

namespace passes {

using namespace ir;

std::unique_ptr<Instruction> cloneInstruction(const Instruction &inst, const std::string &name) {
    Instruction *copy = nullptr;
    switch (inst.getOpcode()) {
    case Opcode::Ret: {
        const ReturnInst &ret = static_cast<const ReturnInst &>(inst);
        copy = new ReturnInst(ret.returnType, ret.returnValue);
        break;
    }
    case Opcode::Br: {
        const BranchInst &br = static_cast<const BranchInst &>(inst);
        copy = br.isConditional() ? new BranchInst(br.condition, br.target, br.falseTarget)
                                  : new BranchInst(br.target);
        break;
    }
    case Opcode::Switch: {
        const SwitchInst &sw = static_cast<const SwitchInst &>(inst);
        SwitchInst *s = new SwitchInst(sw.condition, sw.defaultTarget);
        s->cases = sw.cases;
        copy = s;
        break;
    }
    case Opcode::Unreachable:
        copy = new UnreachableInst();
        break;
    case Opcode::Alloca: {
        const AllocaInst &a = static_cast<const AllocaInst &>(inst);
        copy = new AllocaInst(a.getAccessType(), a.getType(), name);
        break;
    }
    case Opcode::Load: {
        const LoadInst &l = static_cast<const LoadInst &>(inst);
        copy = new LoadInst(l.getType(), l.address, name);
        break;
    }
    case Opcode::Store: {
        const StoreInst &s = static_cast<const StoreInst &>(inst);
        copy = new StoreInst(s.getAccessType(), s.value, s.address);
        break;
    }
    case Opcode::GEP: {
        const GEPInst &g = static_cast<const GEPInst &>(inst);
        copy = new GEPInst(g.getAccessType(), g.getType(), g.base, g.indices, g.inBounds, name);
        break;
    }
    case Opcode::Call: {
        const CallInst &c = static_cast<const CallInst &>(inst);
        copy = new CallInst(c.callee, c.calleeType, c.args, name);
        break;
    }
    case Opcode::Binary: {
        const BinaryInst &b = static_cast<const BinaryInst &>(inst);
        BinaryInst *n = new BinaryInst(b.op, b.operandType, b.lhs, b.rhs, b.getType(), name);
        n->isExact = b.isExact;
        copy = n;
        break;
    }
    case Opcode::Unary: {
        const UnaryInst &u = static_cast<const UnaryInst &>(inst);
        copy = new UnaryInst(u.op, u.operand, name);
        break;
    }
    case Opcode::Cast: {
        const CastInst &c = static_cast<const CastInst &>(inst);
        copy = new CastInst(c.op, c.operand, c.fromType, c.toType, name);
        break;
    }
    case Opcode::Select: {
        const SelectInst &s = static_cast<const SelectInst &>(inst);
        copy = new SelectInst(s.condition, s.ifTrue, s.ifFalse, s.getType(), name);
        break;
    }
    case Opcode::Phi: {
        const PhiInst &p = static_cast<const PhiInst &>(inst);
        PhiInst *n = new PhiInst(p.getType(), name);
        n->incoming = p.incoming;
        copy = n;
        break;
    }
    }
    return std::unique_ptr<Instruction>(copy);
}

void remapInstruction(Instruction &inst, const ValueMap &map) {
    for (Value *operand : inst.getOperands()) {
        ValueMap::const_iterator it = map.find(operand);
        if (it != map.end()) inst.replaceOperand(operand, it->second);
    }
    for (BasicBlock *target : inst.getSuccessors()) {
        ValueMap::const_iterator it = map.find(target);
        if (it != map.end()) inst.replaceSuccessor(target, static_cast<BasicBlock *>(it->second));
    }
    if (inst.getOpcode() == Opcode::Phi) {
        for (std::pair<Value *, BasicBlock *> &entry : static_cast<PhiInst &>(inst).incoming) {
            ValueMap::const_iterator it = map.find(entry.second);
            if (it != map.end()) entry.second = static_cast<BasicBlock *>(it->second);
        }
    }
}

}
