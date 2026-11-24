//turns sema types and constants into llvm text
#ifndef IR_IR_TYPES_H
#define IR_IR_TYPES_H

#include <string>

#include "semantic/symbols.h"
#include "semantic/types.h"

namespace ir {

std::string formatType(const sema::Type *type);

std::string formatRecordName(const sema::RecordSymbol *record);

//the struct body: bases first, then the fields
std::string formatRecordBody(const sema::RecordSymbol *record);

std::string closureTypeName(const ast::LambdaExpr *lambda);
std::string formatClosureBody(const ast::LambdaExpr *lambda);

std::string lambdaFunctionName(const ast::LambdaExpr *lambda);

//a floating constant as the 16 hex digit double llvm accepts
std::string formatFloatLiteral(double value, bool isFloat32);

//escapes the bytes of a char array, the caller includes the closing \0
std::string escapeLLVMString(const std::string &bytes);

//integer width in bits, 0 for non-integers
unsigned integerBits(const sema::Type *type);

bool isPassedByAddress(const sema::Type *type);

//wraps a value to the width of an integer type
long long normalizeInt(long long value, const sema::Type *type);

} // namespace ir

#endif
