#ifndef PASSES_FOLD_H
#define PASSES_FOLD_H

#include <string>

#include "ir/cfg.h"

namespace passes {

//constant folding (dragon book 8.5.4): works out at compile time what the machine would compute
//at run time, bit for bit. integers wrap to their width, floats are computed in their own
//precision, and anything that would trap or is undefined (division by zero, INT_MIN / -1,
//a shift by the width or more, a float too big for its integer) is left for run time.
//every pass that folds (local_dag, const_prop_dense, sccp, instsimplify) uses these.
//each returns null when the operation cannot be folded

ir::Constant *foldBinary(ir::Module &module, ir::BinaryOpKind op, const sema::Type *operandType,
                         const ir::Constant *lhs, const ir::Constant *rhs, const sema::Type *resultType);
ir::Constant *foldUnary(ir::Module &module, ir::UnaryOpKind op, const ir::Constant *operand);
ir::Constant *foldCast(ir::Module &module, ir::CastOpKind op, const ir::Constant *operand,
                       const sema::Type *fromType, const sema::Type *toType);

//the value inst computes when the operands are the given constants (operand order of
//getOperands), or null. select with a constant condition can give a non-constant operand
ir::Value *foldInstruction(ir::Module &module, const ir::Instruction &inst,
                           const std::vector<const ir::Constant *> &operands);

//algebraic identities (dragon 8.5.4, 8.7.4): the value inst equals (x+0 -> x, x-x -> 0, ...), or
//null. instead it may rewrite inst in place into something cheaper (x*2^k -> x<<k, sext(sext x)
//-> sext x) and set rewritten. floating identities only where they hold for NaN and -0.0 too
ir::Value *simplifyInstruction(ir::Module &module, ir::Instruction &inst, bool &rewritten);

//a string that two pure instructions (arithmetic, compare, cast, select, getelementptr) share
//exactly when they compute the same value: opcode, kind, types and operands, the operands of
//+ * & | ^ == != sorted. the value numbers of local_dag and the expressions of cse
std::string expressionKey(const ir::Instruction &inst);

//value as a constant, or null
const ir::Constant *asConstant(const ir::Value *value);

//an int or float constant, the only kinds folding produces or reads
bool isNumber(const ir::Value *value);

//the same number of the same type, floats compared bit for bit
bool sameNumber(const ir::Constant *a, const ir::Constant *b);

} // namespace passes

#endif
