#ifndef IR_INSTRUCTION_H
#define IR_INSTRUCTION_H

#include <ostream>
#include <utility>
#include <vector>

#include "ir/value.h"

namespace ir {

class BasicBlock;

//instruction families, each with its own base class
enum class InstCategory {
    Return,
    Branch,      //br, switch, unreachable
    Memory,      //alloca, load, store, getelementptr
    Call,
    Expression,  //arithmetic, compares, casts, select: pure register values
    Phi          //phi, only made by the optimization passes
};

enum class Opcode {
    Ret, Br, Switch, Unreachable,
    Alloca, Load, Store, GEP,
    Call,
    Binary, Unary, Cast, Select,
    Phi
};

//one instruction in a basic block, a value-producing instruction is its own %name
class Instruction : public Value {
public:
    Instruction(InstCategory category, const sema::Type *type, const std::string &name)
        : Value(ValueKind::Instruction, type, name), category(category) {}

    InstCategory   getCategory() const { return category; }
    virtual Opcode getOpcode() const = 0;
    BasicBlock  *getParent() const { return parent; }
    void         setParent(BasicBlock *block) { parent = block; }

    virtual bool isTerminator() const { return false; }

    //true when removing the instruction could change what the program does
    virtual bool hasSideEffects() const { return isTerminator(); }

    //the values the instruction reads, in order, blocks are not operands
    std::vector<Value *> getOperands() const;

    void replaceOperand(Value *from, Value *to);

    //the blocks a terminator can jump to, in order, with repeats
    std::vector<BasicBlock *> getSuccessors() const;

    void replaceSuccessor(BasicBlock *from, BasicBlock *to);

    virtual void dump(std::ostream &out) const = 0;

    std::string getOperand() const override { return "%" + name; }

protected:
    //the operand fields of the subclass, so the base can read and rewrite them
    virtual std::vector<Value **> operandSlots() { return {}; }
    virtual std::vector<BasicBlock **> successorSlots() { return {}; }

    InstCategory category;
    BasicBlock  *parent = nullptr;
};

class ReturnInst : public Instruction {
public:
    Opcode getOpcode() const override { return Opcode::Ret; }

    ReturnInst(const sema::Type *returnType, Value *returnValue)
        : Instruction(InstCategory::Return, sema::voidType(), ""), returnType(returnType),
          returnValue(returnValue) {}

    bool isTerminator() const override { return true; }
    void dump(std::ostream &out) const override;

    const sema::Type *returnType;
    Value            *returnValue;  //null for ret void

protected:
    std::vector<Value **> operandSlots() override;
};

class BranchInst : public Instruction {
public:
    Opcode getOpcode() const override { return Opcode::Br; }

    explicit BranchInst(BasicBlock *target)
        : Instruction(InstCategory::Branch, sema::voidType(), ""), target(target) {}

    BranchInst(Value *condition, BasicBlock *trueTarget, BasicBlock *falseTarget)
        : Instruction(InstCategory::Branch, sema::voidType(), ""), target(trueTarget),
          condition(condition), falseTarget(falseTarget) {}

    bool isTerminator() const override { return true; }
    bool isConditional() const { return condition != nullptr; }
    void dump(std::ostream &out) const override;

    BasicBlock *target;
    Value      *condition = nullptr;
    BasicBlock *falseTarget = nullptr;

protected:
    std::vector<Value **> operandSlots() override;
    std::vector<BasicBlock **> successorSlots() override;
};

class SwitchInst : public Instruction {
public:
    Opcode getOpcode() const override { return Opcode::Switch; }

    SwitchInst(Value *condition, BasicBlock *defaultTarget)
        : Instruction(InstCategory::Branch, sema::voidType(), ""), condition(condition),
          defaultTarget(defaultTarget) {}

    bool isTerminator() const override { return true; }
    void dump(std::ostream &out) const override;

    Value                                          *condition;
    BasicBlock                                     *defaultTarget;
    std::vector<std::pair<long long, BasicBlock *>> cases;

protected:
    std::vector<Value **> operandSlots() override { return { &condition }; }
    std::vector<BasicBlock **> successorSlots() override;
};

class UnreachableInst : public Instruction {
public:
    Opcode getOpcode() const override { return Opcode::Unreachable; }

    UnreachableInst() : Instruction(InstCategory::Branch, sema::voidType(), "") {}

    bool isTerminator() const override { return true; }
    void dump(std::ostream &out) const override { out << "  unreachable\n"; }
};

class MemoryInst : public Instruction {
public:
    MemoryInst(const sema::Type *resultType, const sema::Type *accessType, const std::string &name)
        : Instruction(InstCategory::Memory, resultType, name), accessType(accessType) {}

    const sema::Type *getAccessType() const { return accessType; }

protected:
    const sema::Type *accessType;
};

class AllocaInst : public MemoryInst {
public:
    Opcode getOpcode() const override { return Opcode::Alloca; }

    AllocaInst(const sema::Type *allocatedType, const sema::Type *pointerType, const std::string &name)
        : MemoryInst(pointerType, allocatedType, name) {}

    void dump(std::ostream &out) const override;
};

class LoadInst : public MemoryInst {
public:
    Opcode getOpcode() const override { return Opcode::Load; }

    LoadInst(const sema::Type *loadedType, Value *address, const std::string &name)
        : MemoryInst(loadedType, loadedType, name), address(address) {}

    void dump(std::ostream &out) const override;

    Value *address;

protected:
    std::vector<Value **> operandSlots() override { return { &address }; }
};

class StoreInst : public MemoryInst {
public:
    Opcode getOpcode() const override { return Opcode::Store; }

    StoreInst(const sema::Type *storedType, Value *value, Value *address)
        : MemoryInst(sema::voidType(), storedType, ""), value(value), address(address) {}

    bool hasSideEffects() const override { return true; }
    void dump(std::ostream &out) const override;

    Value *value;
    Value *address;

protected:
    std::vector<Value **> operandSlots() override { return { &value, &address }; }
};

//getelementptr, accessType is the type the first index steps over
class GEPInst : public MemoryInst {
public:
    Opcode getOpcode() const override { return Opcode::GEP; }

    GEPInst(const sema::Type *sourceType, const sema::Type *pointerType, Value *base,
            const std::vector<Value *> &indices, bool inBounds, const std::string &name)
        : MemoryInst(pointerType, sourceType, name), base(base), indices(indices), inBounds(inBounds) {}

    void dump(std::ostream &out) const override;

    Value               *base;
    std::vector<Value *> indices;
    bool                 inBounds;

protected:
    std::vector<Value **> operandSlots() override;
};

//variadic callees are written with their full type
class CallInst : public Instruction {
public:
    Opcode getOpcode() const override { return Opcode::Call; }

    CallInst(Value *callee, const sema::FunctionType *calleeType, const std::vector<Value *> &args,
             const std::string &name)
        : Instruction(InstCategory::Call, calleeType->returnType, name), callee(callee),
          calleeType(calleeType), args(args) {}

    bool hasSideEffects() const override { return true; }
    void dump(std::ostream &out) const override;

    Value                    *callee;
    const sema::FunctionType *calleeType;
    std::vector<Value *>      args;

protected:
    std::vector<Value **> operandSlots() override;
};

class ExprInst : public Instruction {
public:
    ExprInst(const sema::Type *type, const std::string &name)
        : Instruction(InstCategory::Expression, type, name) {}
};

enum class BinaryOpKind {
    Add, FAdd, Sub, FSub, Mul, FMul, SDiv, UDiv, FDiv, SRem, URem,
    Shl, AShr, LShr, And, Or, Xor,
    ICmpEQ, ICmpNE, ICmpSLT, ICmpSGT, ICmpSLE, ICmpSGE,
    ICmpULT, ICmpUGT, ICmpULE, ICmpUGE,   //pointer ordering, as clang does
    FCmpOEQ, FCmpUNE, FCmpOLT, FCmpOGT, FCmpOLE, FCmpOGE
};

bool isCompare(BinaryOpKind op);

//binary op or compare, operandType is the type of both operands
class BinaryInst : public ExprInst {
public:
    Opcode getOpcode() const override { return Opcode::Binary; }

    BinaryInst(BinaryOpKind op, const sema::Type *operandType, Value *lhs, Value *rhs,
               const sema::Type *resultType, const std::string &name)
        : ExprInst(resultType, name), op(op), operandType(operandType), lhs(lhs), rhs(rhs) {}

    void dump(std::ostream &out) const override;

    BinaryOpKind      op;
    const sema::Type *operandType;
    Value            *lhs;
    Value            *rhs;
    bool              isExact = false;  //sdiv exact, for the distance between two pointers

protected:
    std::vector<Value **> operandSlots() override { return { &lhs, &rhs }; }
};

enum class UnaryOpKind { FNeg };

class UnaryInst : public ExprInst {
public:
    Opcode getOpcode() const override { return Opcode::Unary; }

    UnaryInst(UnaryOpKind op, Value *operand, const std::string &name)
        : ExprInst(operand->getType(), name), op(op), operand(operand) {}

    void dump(std::ostream &out) const override;

    UnaryOpKind op;
    Value      *operand;

protected:
    std::vector<Value **> operandSlots() override { return { &operand }; }
};

enum class CastOpKind {
    SExt, ZExt, Trunc,
    SIToFP, UIToFP, FPToSI, FPToUI,
    FPExt, FPTrunc,
    PtrToInt, IntToPtr
};

class CastInst : public ExprInst {
public:
    Opcode getOpcode() const override { return Opcode::Cast; }

    CastInst(CastOpKind op, Value *operand, const sema::Type *fromType, const sema::Type *toType,
             const std::string &name)
        : ExprInst(toType, name), op(op), operand(operand), fromType(fromType), toType(toType) {}

    void dump(std::ostream &out) const override;

    CastOpKind        op;
    Value            *operand;
    const sema::Type *fromType;
    const sema::Type *toType;

protected:
    std::vector<Value **> operandSlots() override { return { &operand }; }
};

//select, only to keep a null pointer null across a base class conversion
class SelectInst : public ExprInst {
public:
    Opcode getOpcode() const override { return Opcode::Select; }

    SelectInst(Value *condition, Value *ifTrue, Value *ifFalse, const sema::Type *type, const std::string &name)
        : ExprInst(type, name), condition(condition), ifTrue(ifTrue), ifFalse(ifFalse) {}

    void dump(std::ostream &out) const override;

    Value *condition;
    Value *ifTrue;
    Value *ifFalse;

protected:
    std::vector<Value **> operandSlots() override { return { &condition, &ifTrue, &ifFalse }; }
};

//one incoming value per predecessor block
class PhiInst : public Instruction {
public:
    Opcode getOpcode() const override { return Opcode::Phi; }

    PhiInst(const sema::Type *type, const std::string &name) : Instruction(InstCategory::Phi, type, name) {}

    void   addIncoming(Value *value, BasicBlock *block) { incoming.push_back(std::make_pair(value, block)); }
    //null when block is not listed
    Value *getIncomingValueFor(const BasicBlock *block) const;
    void   removeIncomingBlock(const BasicBlock *block);
    void   replaceIncomingBlock(BasicBlock *from, BasicBlock *to);

    void dump(std::ostream &out) const override;

    std::vector<std::pair<Value *, BasicBlock *>> incoming;

protected:
    std::vector<Value **> operandSlots() override;
};

} // namespace ir

#endif
