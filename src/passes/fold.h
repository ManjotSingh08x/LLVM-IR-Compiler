//constant folding and algebraic identities shared by the passes that fold
#ifndef PASSES_FOLD_H
#define PASSES_FOLD_H

#include <string>

#include "ir/cfg.h"

namespace passes {

//each returns null when the operation cannot be folded
ir::Constant *foldBinary(ir::Module &module, ir::BinaryOpKind op, const sema::Type *operandType,
                         const ir::Constant *lhs, const ir::Constant *rhs, const sema::Type *resultType);
ir::Constant *foldUnary(ir::Module &module, ir::UnaryOpKind op, const ir::Constant *operand);
ir::Constant *foldCast(ir::Module &module, ir::CastOpKind op, const ir::Constant *operand,
                       const sema::Type *fromType, const sema::Type *toType);

//the constant inst computes from the given constant operands, or null
ir::Value *foldInstruction(ir::Module &module, const ir::Instruction &inst,
                           const std::vector<const ir::Constant *> &operands);

//the value inst equals by an algebraic identity, or null; may instead rewrite inst in place
ir::Value *simplifyInstruction(ir::Module &module, ir::Instruction &inst, bool &rewritten);

//a key two pure instructions share exactly when they compute the same value
std::string expressionKey(const ir::Instruction &inst);

const ir::Constant *asConstant(const ir::Value *value);

//an int or float constant
bool isNumber(const ir::Value *value);

//floats are compared bit for bit
bool sameNumber(const ir::Constant *a, const ir::Constant *b);

} // namespace passes

#endif
