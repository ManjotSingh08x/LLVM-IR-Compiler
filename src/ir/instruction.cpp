#include "ir/instruction.h"

#include "ir/cfg.h"

namespace ir {

namespace {

const char *binaryOpName(BinaryOpKind op) {
    switch (op) {
    case BinaryOpKind::Add:     return "add";
    case BinaryOpKind::FAdd:    return "fadd";
    case BinaryOpKind::Sub:     return "sub";
    case BinaryOpKind::FSub:    return "fsub";
    case BinaryOpKind::Mul:     return "mul";
    case BinaryOpKind::FMul:    return "fmul";
    case BinaryOpKind::SDiv:    return "sdiv";
    case BinaryOpKind::UDiv:    return "udiv";
    case BinaryOpKind::FDiv:    return "fdiv";
    case BinaryOpKind::SRem:    return "srem";
    case BinaryOpKind::URem:    return "urem";
    case BinaryOpKind::Shl:     return "shl";
    case BinaryOpKind::AShr:    return "ashr";
    case BinaryOpKind::LShr:    return "lshr";
    case BinaryOpKind::And:     return "and";
    case BinaryOpKind::Or:      return "or";
    case BinaryOpKind::Xor:     return "xor";
    case BinaryOpKind::ICmpEQ:  return "icmp eq";
    case BinaryOpKind::ICmpNE:  return "icmp ne";
    case BinaryOpKind::ICmpSLT: return "icmp slt";
    case BinaryOpKind::ICmpSGT: return "icmp sgt";
    case BinaryOpKind::ICmpSLE: return "icmp sle";
    case BinaryOpKind::ICmpSGE: return "icmp sge";
    case BinaryOpKind::ICmpULT: return "icmp ult";
    case BinaryOpKind::ICmpUGT: return "icmp ugt";
    case BinaryOpKind::ICmpULE: return "icmp ule";
    case BinaryOpKind::ICmpUGE: return "icmp uge";
    case BinaryOpKind::FCmpOEQ: return "fcmp oeq";
    case BinaryOpKind::FCmpUNE: return "fcmp une";
    case BinaryOpKind::FCmpOLT: return "fcmp olt";
    case BinaryOpKind::FCmpOGT: return "fcmp ogt";
    case BinaryOpKind::FCmpOLE: return "fcmp ole";
    case BinaryOpKind::FCmpOGE: return "fcmp oge";
    }
    return "";
}

const char *castOpName(CastOpKind op) {
    switch (op) {
    case CastOpKind::SExt:     return "sext";
    case CastOpKind::ZExt:     return "zext";
    case CastOpKind::Trunc:    return "trunc";
    case CastOpKind::SIToFP:   return "sitofp";
    case CastOpKind::UIToFP:   return "uitofp";
    case CastOpKind::FPToSI:   return "fptosi";
    case CastOpKind::FPToUI:   return "fptoui";
    case CastOpKind::FPExt:    return "fpext";
    case CastOpKind::FPTrunc:  return "fptrunc";
    case CastOpKind::PtrToInt: return "ptrtoint";
    case CastOpKind::IntToPtr: return "inttoptr";
    }
    return "";
}

} // namespace

bool isCompare(BinaryOpKind op) {
    switch (op) {
    case BinaryOpKind::ICmpEQ:
    case BinaryOpKind::ICmpNE:
    case BinaryOpKind::ICmpSLT:
    case BinaryOpKind::ICmpSGT:
    case BinaryOpKind::ICmpSLE:
    case BinaryOpKind::ICmpSGE:
    case BinaryOpKind::ICmpULT:
    case BinaryOpKind::ICmpUGT:
    case BinaryOpKind::ICmpULE:
    case BinaryOpKind::ICmpUGE:
    case BinaryOpKind::FCmpOEQ:
    case BinaryOpKind::FCmpUNE:
    case BinaryOpKind::FCmpOLT:
    case BinaryOpKind::FCmpOGT:
    case BinaryOpKind::FCmpOLE:
    case BinaryOpKind::FCmpOGE:
        return true;
    default:
        return false;
    }
}

//  operands and successors 

std::vector<Value *> Instruction::getOperands() const {
    std::vector<Value *> result;
    for (Value **slot : const_cast<Instruction *>(this)->operandSlots()) result.push_back(*slot);
    return result;
}

void Instruction::replaceOperand(Value *from, Value *to) {
    for (Value **slot : operandSlots()) {
        if (*slot == from) *slot = to;
    }
}

std::vector<BasicBlock *> Instruction::getSuccessors() const {
    std::vector<BasicBlock *> result;
    for (BasicBlock **slot : const_cast<Instruction *>(this)->successorSlots()) result.push_back(*slot);
    return result;
}

void Instruction::replaceSuccessor(BasicBlock *from, BasicBlock *to) {
    for (BasicBlock **slot : successorSlots()) {
        if (*slot == from) *slot = to;
    }
}

//ret void has no operand
std::vector<Value **> ReturnInst::operandSlots() {
    if (returnValue == nullptr) return {};
    return { &returnValue };
}

std::vector<Value **> BranchInst::operandSlots() {
    if (condition == nullptr) return {};
    return { &condition };
}

std::vector<BasicBlock **> BranchInst::successorSlots() {
    if (falseTarget == nullptr) return { &target };
    return { &target, &falseTarget };
}

std::vector<BasicBlock **> SwitchInst::successorSlots() {
    std::vector<BasicBlock **> slots;
    slots.push_back(&defaultTarget);
    for (std::pair<long long, BasicBlock *> &c : cases) slots.push_back(&c.second);
    return slots;
}

std::vector<Value **> GEPInst::operandSlots() {
    std::vector<Value **> slots;
    slots.push_back(&base);
    for (Value *&index : indices) slots.push_back(&index);
    return slots;
}

//the callee comes first, it is a value too for calls through a pointer
std::vector<Value **> CallInst::operandSlots() {
    std::vector<Value **> slots;
    slots.push_back(&callee);
    for (Value *&arg : args) slots.push_back(&arg);
    return slots;
}

std::vector<Value **> PhiInst::operandSlots() {
    std::vector<Value **> slots;
    for (std::pair<Value *, BasicBlock *> &entry : incoming) slots.push_back(&entry.first);
    return slots;
}

Value *PhiInst::getIncomingValueFor(const BasicBlock *block) const {
    for (const std::pair<Value *, BasicBlock *> &entry : incoming) {
        if (entry.second == block) return entry.first;
    }
    return nullptr;
}

void PhiInst::removeIncomingBlock(const BasicBlock *block) {
    for (size_t i = 0; i < incoming.size();) {
        if (incoming[i].second == block) {
            incoming.erase(incoming.begin() + i);
        } else {
            i++;
        }
    }
}

void PhiInst::replaceIncomingBlock(BasicBlock *from, BasicBlock *to) {
    for (std::pair<Value *, BasicBlock *> &entry : incoming) {
        if (entry.second == from) entry.second = to;
    }
}

//  dump 

void ReturnInst::dump(std::ostream &out) const {
    if (returnValue == nullptr) {
        out << "  ret void\n";
    } else {
        out << "  ret " << formatType(returnType) << " " << returnValue->getOperand() << "\n";
    }
}

void BranchInst::dump(std::ostream &out) const {
    if (isConditional()) {
        out << "  br i1 " << condition->getOperand() << ", label " << target->getOperand()
            << ", label " << falseTarget->getOperand() << "\n";
    } else {
        out << "  br label " << target->getOperand() << "\n";
    }
}

void SwitchInst::dump(std::ostream &out) const {
    out << "  switch " << condition->getTypedOperand() << ", label " << defaultTarget->getOperand() << " [\n";
    for (size_t i = 0; i < cases.size(); i++) {
        out << "    " << formatType(condition->getType()) << " " << cases[i].first << ", label "
            << cases[i].second->getOperand() << "\n";
    }
    out << "  ]\n";
}

void AllocaInst::dump(std::ostream &out) const {
    out << "  %" << name << " = alloca " << formatType(accessType) << "\n";
}

void LoadInst::dump(std::ostream &out) const {
    out << "  %" << name << " = load " << formatType(accessType) << ", ptr " << address->getOperand() << "\n";
}

void StoreInst::dump(std::ostream &out) const {
    out << "  store " << formatType(accessType) << " " << value->getOperand() << ", ptr "
        << address->getOperand() << "\n";
}

void GEPInst::dump(std::ostream &out) const {
    out << "  %" << name << " = getelementptr " << (inBounds ? "inbounds " : "") << formatType(accessType)
        << ", ptr " << base->getOperand();
    for (size_t i = 0; i < indices.size(); i++) out << ", " << indices[i]->getTypedOperand();
    out << "\n";
}

void CallInst::dump(std::ostream &out) const {
    out << "  ";
    if (!type->is(sema::TypeKind::Void)) out << "%" << name << " = ";
    out << "call ";
    //a variadic callee is written with its full type
    if (calleeType->isVariadic) {
        out << formatType(calleeType);
    } else {
        out << formatType(calleeType->returnType);
    }
    out << " " << callee->getOperand() << "(";
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) out << ", ";
        //fixed parameters use the declared type, variadic extras their own
        if (i < calleeType->params.size()) {
            out << formatType(calleeType->params[i]) << " " << args[i]->getOperand();
        } else {
            out << args[i]->getTypedOperand();
        }
    }
    out << ")\n";
}

void BinaryInst::dump(std::ostream &out) const {
    out << "  %" << name << " = " << binaryOpName(op) << (isExact ? " exact " : " ")
        << formatType(operandType) << " " << lhs->getOperand() << ", " << rhs->getOperand() << "\n";
}

void UnaryInst::dump(std::ostream &out) const {
    out << "  %" << name << " = fneg " << operand->getTypedOperand() << "\n";
}

void CastInst::dump(std::ostream &out) const {
    out << "  %" << name << " = " << castOpName(op) << " " << formatType(fromType) << " "
        << operand->getOperand() << " to " << formatType(toType) << "\n";
}

void SelectInst::dump(std::ostream &out) const {
    out << "  %" << name << " = select i1 " << condition->getOperand() << ", " << formatType(type) << " "
        << ifTrue->getOperand() << ", " << formatType(type) << " " << ifFalse->getOperand() << "\n";
}

void PhiInst::dump(std::ostream &out) const {
    out << "  %" << name << " = phi " << formatType(type);
    for (size_t i = 0; i < incoming.size(); i++) {
        out << (i > 0 ? ", [ " : " [ ") << incoming[i].first->getOperand() << ", "
            << incoming[i].second->getOperand() << " ]";
    }
    out << "\n";
}

} // namespace ir
