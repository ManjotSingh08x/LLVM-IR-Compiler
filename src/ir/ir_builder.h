#ifndef IR_IR_BUILDER_H
#define IR_IR_BUILDER_H

#include <string>
#include <vector>

#include "ir/cfg.h"

namespace ir {

struct InsertPoint {
    Function   *function = nullptr;
    BasicBlock *block = nullptr;
};

class IRBuilder {
public:
    explicit IRBuilder(Module &module) : module(module) {}

    void        setInsertPoint(Function *fn, BasicBlock *block);
    void        setInsertBlock(BasicBlock *block);
    InsertPoint saveInsertPoint() const;
    void        restoreInsertPoint(const InsertPoint &point);
    Function   *getFunction() const { return function; }
    BasicBlock *getInsertBlock() const { return block; }

    bool isReachable() const { return block != nullptr && !block->isTerminated(); }

    BasicBlock *createBlock(const std::string &label);

    //creates a stack slot in the entry block
    AllocaInst *createEntryAlloca(const sema::Type *type, const std::string &baseName);
    Value      *createLoad(const sema::Type *type, Value *address);
    void        createStore(const sema::Type *type, Value *value, Value *address);
    Value      *createGEP(const sema::Type *sourceType, Value *base, const std::vector<Value *> &indices,
                          bool inBounds = true);

    Value      *createStructGEP(const sema::Type *structType, Value *base, unsigned index);
    Value      *createElementGEP(const sema::Type *elementType, Value *base, Value *index);

    Value *createBinary(BinaryOpKind op, const sema::Type *operandType, Value *lhs, Value *rhs);
    Value *createExactSDiv(const sema::Type *operandType, Value *lhs, Value *rhs);
    Value *createFNeg(Value *operand);
    Value *createCast(CastOpKind op, Value *operand, const sema::Type *from, const sema::Type *to);
    Value *createSelect(Value *condition, Value *ifTrue, Value *ifFalse, const sema::Type *type);

    Value *createCall(Value *callee, const sema::FunctionType *sourceType, const std::vector<Value *> &args);

    void        createRet(const sema::Type *returnType, Value *value);
    void        createRetVoid();
    void        createBr(BasicBlock *target);
    void        createCondBr(Value *condition, BasicBlock *trueTarget, BasicBlock *falseTarget);

    SwitchInst *createSwitch(Value *condition, BasicBlock *defaultTarget);
    void        addSwitchCase(SwitchInst *inst, long long value, BasicBlock *target);
    void        setSwitchDefault(SwitchInst *inst, BasicBlock *target);

private:

    Instruction *insert(Instruction *inst);
    std::string  tempName();

    Module     &module;
    Function   *function = nullptr;
    BasicBlock *block = nullptr;
};

}

#endif
