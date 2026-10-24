#ifndef IR_IR_TYPES_H
#define IR_IR_TYPES_H

#include <string>

#include "semantic/symbols.h"
#include "semantic/types.h"

//the ir reuses the semantic types (sema::Type) for every value. this file holds
//everything that turns those types, and constant values, into llvm text. it does
//not use sema::toLLVMType, which only exists for the -symbols.txt dump
namespace ir {

//sema has no 64-bit integer, but sizes for malloc, the array cookie and the
//distance between two pointers need one. this is a separate Type object of kind
//Int that only the ir knows about: formatType checks for it by address first
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

//a floating point constant as the 16 hex digit ieee double llvm accepts for
//both float and double. a float is rounded to float precision first
std::string formatFloatLiteral(double value, bool isFloat32);

//the bytes of a character array as an llvm c"..." string. every byte is
//written, so the caller includes the closing \0 and any padding
std::string escapeLLVMString(const std::string &bytes);

//the width of an integer type in bits: 1 for bool, 8 for char, 32 for int,
//64 for int64Type(). 0 for anything else
unsigned integerBits(const sema::Type *type);

//wraps a value to the width of an integer type, as storing it there would:
//300 as a char is 44, 2147483648 as an int is -2147483648, 3 as a bool is 1
long long normalizeInt(long long value, const sema::Type *type);

} // namespace ir

#endif
