#ifndef SEMANTIC_CONSTANTS_H
#define SEMANTIC_CONSTANTS_H

#include <string>

#include "ast/ast.h"
#include "semantic/types.h"

namespace sema {

//parses an integer literal and picks its C type from its value and suffix, false when none fits
bool intLiteral(const std::string &spelling, long long &value, const Type *&type);

//converts a value to an integer type: wrapped to its width, then extended back to 64 bits
long long wrapToType(long long value, const Type *type);

//parses a floating literal as double, or as float with an f suffix
double floatLiteral(const std::string &spelling, const Type *&type);

long long charLiteralValue(const std::string &spelling);

//string literal length, without the closing \0
size_t stringLiteralLength(const std::string &spelling);

//a string literal's characters with escapes resolved, without quotes or the closing \0
std::string decodeStringLiteral(const std::string &spelling);

//evaluates an integer constant expression, false when it is not constant
bool evaluateConstant(const ast::Expr *expr, long long &value);

} // namespace sema

#endif
