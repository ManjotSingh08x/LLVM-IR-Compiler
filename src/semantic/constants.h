#ifndef SEMANTIC_CONSTANTS_H
#define SEMANTIC_CONSTANTS_H

#include <string>

#include "ast/ast.h"
#include "semantic/types.h"

namespace sema {

//work out literal values from their source spelling

//decimal, 0x1F, 0755 and 0b1010 with a u and l suffix. the type is C's: the first of int, long
//(decimal) or int, unsigned int, long, unsigned long (others) that holds the value, narrowed by
//the suffix. false when no type holds it
bool intLiteral(const std::string &spelling, long long &value, const Type *&type);

//a value converted to an integer type: wrapped to its width, sign- or zero-extended back to 64 bits
long long wrapToType(long long value, const Type *type);

//1.5, .5e3, 2e-1 as double, with an f suffix as float, rounded once to that type
double floatLiteral(const std::string &spelling, const Type *&type);

//'a', '\n', '\x41', '\0'
long long charLiteralValue(const std::string &spelling);

//the number of characters a string literal holds, without the closing \0
size_t stringLiteralLength(const std::string &spelling);

//a string literal's characters with escapes resolved, without quotes or the closing \0
std::string decodeStringLiteral(const std::string &spelling);

//evaluates an integer constant expression, false when it is not constant. after type checking each
//step is done in the type of its node, unsigned where the types say so
bool evaluateConstant(const ast::Expr *expr, long long &value);

} // namespace sema

#endif
