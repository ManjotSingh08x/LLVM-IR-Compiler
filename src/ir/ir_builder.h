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

//makes instructions at the insertion point, allocas go to entry and nothing follows a terminator
class IRBuilder {
public:
    explicit IRBuilder(Module &module) : module(module) {}

    void        setInsertPoint(Function *fn, BasicBlock *block);
    void        setInsertBlock(BasicBlock *block);
    InsertPoint saveInsertPoint() const;
    void        restoreInsertPoint(const InsertPoint &point);
    Function   *getFunction() const { return function; }
    BasicBlock *getInsertBlock() const { return block; }

    //true while there is an unterminated block to write into
    bool isReachable() const { return block != nullptr && !block->isTerminated(); }

    BasicBlock *createBlock(const std::string &label);

    AllocaInst *createEntryAlloca(const sema::Type *type, const std::string &baseName);
    Value      *createLoad(const sema::Type *type, Value *address);
    void        createStore(const sema::Type *type, Value *value, Value *address);
    Value      *createGEP(const sema::Type *sourceType, Value *base, const std::vector<Value *> &indices,
                          bool inBounds = true);
    //address of field or base number index of a struct
    Value      *createStructGEP(const sema::Type *structType, Value *base, unsigned index);
    Value      *createElementGEP(const sema::Type *elementType, Value *base, Value *index);

    Value *createBinary(BinaryOpKind op, const sema::Type *operandType, Value *lhs, Value *rhs);
    Value *createExactSDiv(const sema::Type *operandType, Value *lhs, Value *rhs);
    Value *createFNeg(Value *operand);
    Value *createCast(CastOpKind op, Value *operand, const sema::Type *from, const sema::Type *to);
    Value *createSelect(Value *condition, Value *ifTrue, Value *ifFalse, const sema::Type *type);

    //marks a called function as used
    Value *createCall(Value *callee, const sema::FunctionType *sourceType, const std::vector<Value *> &args);

    void        createRet(const sema::Type *returnType, Value *value);
    void        createRetVoid();
    void        createBr(BasicBlock *target);
    void        createCondBr(Value *condition, BasicBlock *trueTarget, BasicBlock *falseTarget);
    //returns null when the switch itself is unreachable
    SwitchInst *createSwitch(Value *condition, BasicBlock *defaultTarget);
    void        addSwitchCase(SwitchInst *inst, long long value, BasicBlock *target);
    void        setSwitchDefault(SwitchInst *inst, BasicBlock *target);

private:
    //appends to the current block, or keeps the instruction unwritten when it is terminated
    Instruction *insert(Instruction *inst);
    std::string  tempName();

    Module     &module;
    Function   *function = nullptr;
    BasicBlock *block = nullptr;
};

} // namespace ir

#endif
