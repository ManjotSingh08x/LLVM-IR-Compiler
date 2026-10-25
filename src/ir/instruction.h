#ifndef IR_INSTRUCTION_H
#define IR_INSTRUCTION_H

#include <ostream>
#include <utility>
#include <vector>

#include "ir/value.h"

namespace ir {

class BasicBlock;

//the families an instruction belongs to. each family shares a base class
enum class InstCategory {
    Return,      //ret
    Branch,      //br, switch, unreachable
    Memory,      //alloca, load, store, getelementptr
    Call,        //call
    Expression   //arithmetic, compares, casts, select: pure register values
};

//one instruction in a basic block, a value-producing instruction is its own %name
class Instruction : public Value {
public:
    Instruction(InstCategory category, const sema::Type *type, const std::string &name)
        : Value(ValueKind::Instruction, type, name), category(category) {}

    InstCategory getCategory() const { return category; }
    BasicBlock  *getParent() const { return parent; }
    void         setParent(BasicBlock *block) { parent = block; }

    //a terminator ends its block: ret, br, switch, unreachable
    virtual bool isTerminator() const { return false; }

    //writes the instruction as one line of llvm text
    virtual void dump(std::ostream &out) const = 0;

    std::string getOperand() const override { return "%" + name; }

protected:
    InstCategory category;
    BasicBlock  *parent = nullptr;
};

// ---- return ----

//ret void, or ret <type> <value>
class ReturnInst : public Instruction {
public:
    ReturnInst(const sema::Type *returnType, Value *returnValue)
        : Instruction(InstCategory::Return, sema::voidType(), ""), returnType(returnType),
          returnValue(returnValue) {}

    bool isTerminator() const override { return true; }
    void dump(std::ostream &out) const override;

    const sema::Type *returnType;
    Value            *returnValue;  //null for ret void
};

// ---- branches ----

//br label %target, or br i1 %cond, label %target, label %falseTarget
class BranchInst : public Instruction {
public:
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
};

//switch i32 %v, label %default [ i32 1, label %a  i32 2, label %b ]
class SwitchInst : public Instruction {
public:
    SwitchInst(Value *condition, BasicBlock *defaultTarget)
        : Instruction(InstCategory::Branch, sema::voidType(), ""), condition(condition),
          defaultTarget(defaultTarget) {}

    bool isTerminator() const override { return true; }
    void dump(std::ostream &out) const override;

    Value                                          *condition;
    BasicBlock                                     *defaultTarget;
    std::vector<std::pair<long long, BasicBlock *>> cases;
};

//unreachable: the end of a block that can never run to its end
class UnreachableInst : public Instruction {
public:
    UnreachableInst() : Instruction(InstCategory::Branch, sema::voidType(), "") {}

    bool isTerminator() const override { return true; }
    void dump(std::ostream &out) const override { out << "  unreachable\n"; }
};

// ---- memory ----

class MemoryInst : public Instruction {
public:
    MemoryInst(const sema::Type *resultType, const sema::Type *accessType, const std::string &name)
        : Instruction(InstCategory::Memory, resultType, name), accessType(accessType) {}

    const sema::Type *getAccessType() const { return accessType; }

protected:
    const sema::Type *accessType;
};

//%x.addr = alloca i32
class AllocaInst : public MemoryInst {
public:
    AllocaInst(const sema::Type *allocatedType, const sema::Type *pointerType, const std::string &name)
        : MemoryInst(pointerType, allocatedType, name) {}

    void dump(std::ostream &out) const override;
};

//%t0 = load i32, ptr %x.addr
class LoadInst : public MemoryInst {
public:
    LoadInst(const sema::Type *loadedType, Value *address, const std::string &name)
        : MemoryInst(loadedType, loadedType, name), address(address) {}

    void dump(std::ostream &out) const override;

    Value *address;
};

//store i32 %t0, ptr %x.addr
class StoreInst : public MemoryInst {
public:
    StoreInst(const sema::Type *storedType, Value *value, Value *address)
        : MemoryInst(sema::voidType(), storedType, ""), value(value), address(address) {}

    void dump(std::ostream &out) const override;

    Value *value;
    Value *address;
};

//getelementptr, accessType is the type the first index steps over
class GEPInst : public MemoryInst {
public:
    GEPInst(const sema::Type *sourceType, const sema::Type *pointerType, Value *base,
            const std::vector<Value *> &indices, bool inBounds, const std::string &name)
        : MemoryInst(pointerType, sourceType, name), base(base), indices(indices), inBounds(inBounds) {}

    void dump(std::ostream &out) const override;

    Value               *base;
    std::vector<Value *> indices;
    bool                 inBounds;
};

// ---- calls ----

//%t2 = call i32 @f(i32 1), variadic callees are written with their full type
class CallInst : public Instruction {
public:
    CallInst(Value *callee, const sema::FunctionType *calleeType, const std::vector<Value *> &args,
             const std::string &name)
        : Instruction(InstCategory::Call, calleeType->returnType, name), callee(callee),
          calleeType(calleeType), args(args) {}

    void dump(std::ostream &out) const override;

    Value                    *callee;
    const sema::FunctionType *calleeType;
    std::vector<Value *>      args;
};

// ---- expressions ----

class ExprInst : public Instruction {
public:
    ExprInst(const sema::Type *type, const std::string &name)
        : Instruction(InstCategory::Expression, type, name) {}
};

enum class BinaryOpKind {
    Add, FAdd, Sub, FSub, Mul, FMul, SDiv, FDiv, SRem,
    Shl, AShr, And, Or, Xor,
    ICmpEQ, ICmpNE, ICmpSLT, ICmpSGT, ICmpSLE, ICmpSGE,
    ICmpULT, ICmpUGT, ICmpULE, ICmpUGE,   //pointer ordering, as clang does
    FCmpOEQ, FCmpUNE, FCmpOLT, FCmpOGT, FCmpOLE, FCmpOGE
};

bool isCompare(BinaryOpKind op);

//binary op or compare, operandType is the type of both operands
class BinaryInst : public ExprInst {
public:
    BinaryInst(BinaryOpKind op, const sema::Type *operandType, Value *lhs, Value *rhs,
               const sema::Type *resultType, const std::string &name)
        : ExprInst(resultType, name), op(op), operandType(operandType), lhs(lhs), rhs(rhs) {}

    void dump(std::ostream &out) const override;

    BinaryOpKind      op;
    const sema::Type *operandType;
    Value            *lhs;
    Value            *rhs;
    bool              isExact = false;  //sdiv exact, for the distance between two pointers
};

enum class UnaryOpKind { FNeg };

//%t1 = fneg double %t0
class UnaryInst : public ExprInst {
public:
    UnaryInst(UnaryOpKind op, Value *operand, const std::string &name)
        : ExprInst(operand->getType(), name), op(op), operand(operand) {}

    void dump(std::ostream &out) const override;

    UnaryOpKind op;
    Value      *operand;
};

enum class CastOpKind {
    SExt, ZExt, Trunc,
    SIToFP, UIToFP, FPToSI,
    FPExt, FPTrunc,
    PtrToInt, IntToPtr
};

//%t2 = sext i8 %t1 to i32
class CastInst : public ExprInst {
public:
    CastInst(CastOpKind op, Value *operand, const sema::Type *fromType, const sema::Type *toType,
             const std::string &name)
        : ExprInst(toType, name), op(op), operand(operand), fromType(fromType), toType(toType) {}

    void dump(std::ostream &out) const override;

    CastOpKind        op;
    Value            *operand;
    const sema::Type *fromType;
    const sema::Type *toType;
};

//select, only to keep a null pointer null across a base class conversion
class SelectInst : public ExprInst {
public:
    SelectInst(Value *condition, Value *ifTrue, Value *ifFalse, const sema::Type *type, const std::string &name)
        : ExprInst(type, name), condition(condition), ifTrue(ifTrue), ifFalse(ifFalse) {}

    void dump(std::ostream &out) const override;

    Value *condition;
    Value *ifTrue;
    Value *ifFalse;
};

} // namespace ir

#endif
