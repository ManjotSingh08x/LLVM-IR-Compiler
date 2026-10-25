#ifndef IR_IR_TYPES_H
#define IR_IR_TYPES_H

#include <string>

#include "semantic/symbols.h"
#include "semantic/types.h"

//turns sema types and constants into llvm text
namespace ir {

//the i64 type the ir needs for sizes, recognised by address
const sema::Type *int64Type();
bool isInt64(const sema::Type *type);

//the llvm spelling of a type: i32, ptr, [3 x i32], %struct.Point, ...
std::string formatType(const sema::Type *type);

//%struct.Point, %class.Outer.Inner
std::string formatRecordName(const sema::RecordSymbol *record);

//the layout after "= type": bases first, then the fields: { %class.Foo, i32 }
std::string formatRecordBody(const sema::RecordSymbol *record);

//the struct a lambda's captures are stored in: %closure.<line>.<col>
std::string closureTypeName(const ast::LambdaExpr *lambda);
std::string formatClosureBody(const ast::LambdaExpr *lambda);

//the global function the lambda body becomes: __lambda_<line>_<col>
std::string lambdaFunctionName(const ast::LambdaExpr *lambda);

//a floating constant as the 16 hex digit double llvm accepts
std::string formatFloatLiteral(double value, bool isFloat32);

//the bytes of a char array as c\"...\", the caller includes the closing \0
std::string escapeLLVMString(const std::string &bytes);

//integer width in bits: 1, 8, 32 or 64, 0 for non-integers
unsigned integerBits(const sema::Type *type);

//wraps a value to the width of an integer type: 300 as a char is 44
long long normalizeInt(long long value, const sema::Type *type);

} // namespace ir

#endif
