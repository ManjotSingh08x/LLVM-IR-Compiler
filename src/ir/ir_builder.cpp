#include "ir/ir_builder.h"

namespace ir {

void IRBuilder::setInsertPoint(Function *fn, BasicBlock *newBlock) {
    function = fn;
    setInsertBlock(newBlock);
}

void IRBuilder::setInsertBlock(BasicBlock *newBlock) {
    block = newBlock;
    if (block) function->placeBlock(block);
}

InsertPoint IRBuilder::saveInsertPoint() const {
    InsertPoint point;
    point.function = function;
    point.block = block;
    return point;
}

void IRBuilder::restoreInsertPoint(const InsertPoint &point) {
    function = point.function;
    block = point.block;
}

BasicBlock *IRBuilder::createBlock(const std::string &label) {
    return function->createBlock(label);
}

std::string IRBuilder::tempName() { return function->nextTempName(); }

Instruction *IRBuilder::insert(Instruction *inst) {
    std::unique_ptr<Instruction> owned(inst);
    if (isReachable()) {
        block->append(std::move(owned));
    } else {
        function->discard(std::move(owned));
    }
    return inst;
}

// ---- memory ----

//allocas go at the top of entry so they run once per call and dominate every use
AllocaInst *IRBuilder::createEntryAlloca(const sema::Type *type, const std::string &baseName) {
    AllocaInst *inst = new AllocaInst(type, module.pointerType(), function->uniqueName(baseName));
    function->getEntryBlock()->insertAlloca(std::unique_ptr<Instruction>(inst));
    return inst;
}

Value *IRBuilder::createLoad(const sema::Type *type, Value *address) {
    return insert(new LoadInst(type, address, tempName()));
}

void IRBuilder::createStore(const sema::Type *type, Value *value, Value *address) {
    insert(new StoreInst(type, value, address));
}

Value *IRBuilder::createGEP(const sema::Type *sourceType, Value *base, const std::vector<Value *> &indices,
                            bool inBounds) {
    return insert(new GEPInst(sourceType, module.pointerType(), base, indices, inBounds, tempName()));
}

Value *IRBuilder::createStructGEP(const sema::Type *structType, Value *base, unsigned index) {
    std::vector<Value *> indices;
    indices.push_back(module.getInt32(0));
    indices.push_back(module.getInt32(index));
    return createGEP(structType, base, indices);
}

Value *IRBuilder::createElementGEP(const sema::Type *elementType, Value *base, Value *index) {
    std::vector<Value *> indices;
    indices.push_back(index);
    return createGEP(elementType, base, indices);
}

// ---- values ----

Value *IRBuilder::createBinary(BinaryOpKind op, const sema::Type *operandType, Value *lhs, Value *rhs) {
    const sema::Type *resultType = isCompare(op) ? sema::boolType() : operandType;
    return insert(new BinaryInst(op, operandType, lhs, rhs, resultType, tempName()));
}

Value *IRBuilder::createExactSDiv(const sema::Type *operandType, Value *lhs, Value *rhs) {
    BinaryInst *inst = new BinaryInst(BinaryOpKind::SDiv, operandType, lhs, rhs, operandType, tempName());
    inst->isExact = true;
    return insert(inst);
}

Value *IRBuilder::createFNeg(Value *operand) {
    return insert(new UnaryInst(UnaryOpKind::FNeg, operand, tempName()));
}

Value *IRBuilder::createCast(CastOpKind op, Value *operand, const sema::Type *from, const sema::Type *to) {
    return insert(new CastInst(op, operand, from, to, tempName()));
}

Value *IRBuilder::createSelect(Value *condition, Value *ifTrue, Value *ifFalse, const sema::Type *type) {
    return insert(new SelectInst(condition, ifTrue, ifFalse, type, tempName()));
}

Value *IRBuilder::createCall(Value *callee, const sema::FunctionType *calleeType, const std::vector<Value *> &args) {
    if (callee->getKind() == ValueKind::Function && isReachable()) {
        static_cast<Function *>(callee)->isUsed = true;
    }
    std::string name = calleeType->returnType->is(sema::TypeKind::Void) ? "" : tempName();
    return insert(new CallInst(callee, calleeType, args, name));
}

// ---- terminators, edges are recorded only for branches really written ----

void IRBuilder::createRet(const sema::Type *returnType, Value *value) {
    insert(new ReturnInst(returnType, value));
}

void IRBuilder::createRetVoid() {
    insert(new ReturnInst(sema::voidType(), nullptr));
}

void IRBuilder::createBr(BasicBlock *target) {
    if (isReachable()) BasicBlock::addEdge(block, target);
    insert(new BranchInst(target));
}

void IRBuilder::createCondBr(Value *condition, BasicBlock *trueTarget, BasicBlock *falseTarget) {
    if (isReachable()) {
        BasicBlock::addEdge(block, trueTarget);
        BasicBlock::addEdge(block, falseTarget);
    }
    insert(new BranchInst(condition, trueTarget, falseTarget));
}

SwitchInst *IRBuilder::createSwitch(Value *condition, BasicBlock *defaultTarget) {
    if (!isReachable()) return nullptr;
    BasicBlock::addEdge(block, defaultTarget);
    SwitchInst *inst = new SwitchInst(condition, defaultTarget);
    insert(inst);
    return inst;
}

void IRBuilder::addSwitchCase(SwitchInst *inst, long long value, BasicBlock *target) {
    if (inst == nullptr) return;
    inst->cases.push_back(std::make_pair(value, target));
    BasicBlock::addEdge(inst->getParent(), target);
}

//the default starts as the switch end and moves to the default label once seen
void IRBuilder::setSwitchDefault(SwitchInst *inst, BasicBlock *target) {
    if (inst == nullptr) return;
    BasicBlock *from = inst->getParent();
    BasicBlock *old = inst->defaultTarget;
    for (size_t i = 0; i < from->successors.size(); i++) {
        if (from->successors[i] == old) {
            from->successors.erase(from->successors.begin() + i);
            break;
        }
    }
    for (size_t i = 0; i < old->predecessors.size(); i++) {
        if (old->predecessors[i] == from) {
            old->predecessors.erase(old->predecessors.begin() + i);
            break;
        }
    }
    inst->defaultTarget = target;
    BasicBlock::addEdge(from, target);
}

} // namespace ir
