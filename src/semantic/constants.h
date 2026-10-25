#ifndef SEMANTIC_CONSTANTS_H
#define SEMANTIC_CONSTANTS_H

#include <string>

#include "ast/ast.h"

namespace sema {

//work out literal values from their source spelling

//decimal, 0x1F, 0755 and 0b1010. false when the value does not fit in an int
bool intLiteralValue(const std::string &spelling, long long &value);

//'a', '\n', '\x41', '\0'
long long charLiteralValue(const std::string &spelling);

//the number of characters a string literal holds, without the closing \0
size_t stringLiteralLength(const std::string &spelling);

//a string literal's characters with escapes resolved, without quotes or the closing \0
std::string decodeStringLiteral(const std::string &spelling);

//evaluates an integer constant expression, false when it is not constant
bool evaluateConstant(const ast::Expr *expr, long long &value);

} // namespace sema

#endif
