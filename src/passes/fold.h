#ifndef PASSES_FOLD_H
#define PASSES_FOLD_H

#include <string>

#include "ir/cfg.h"

namespace passes {

ir::Constant *foldBinary(ir::Module &module, ir::BinaryOpKind op, const sema::Type *operandType,
                         const ir::Constant *lhs, const ir::Constant *rhs, const sema::Type *resultType);
ir::Constant *foldUnary(ir::Module &module, ir::UnaryOpKind op, const ir::Constant *operand);
ir::Constant *foldCast(ir::Module &module, ir::CastOpKind op, const ir::Constant *operand,
                       const sema::Type *fromType, const sema::Type *toType);

//evaluates an instruction when operands are constant
ir::Value *foldInstruction(ir::Module &module, const ir::Instruction &inst,
                           const std::vector<const ir::Constant *> &operands);

ir::Value *simplifyInstruction(ir::Module &module, ir::Instruction &inst, bool &rewritten);

//builds a text key identifying an expression
std::string expressionKey(const ir::Instruction &inst);

const ir::Constant *asConstant(const ir::Value *value);

bool isNumber(const ir::Value *value);

bool sameNumber(const ir::Constant *a, const ir::Constant *b);

}

#endif
