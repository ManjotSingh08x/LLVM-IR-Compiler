#ifndef SEMANTIC_CONSTANTS_H
#define SEMANTIC_CONSTANTS_H

#include <string>

#include "ast/ast.h"

namespace sema {

//literals keep their source spelling in the tree, quotes and escapes
//included, these work out their values

//decimal, 0x1F, 0755 and 0b1010. false when the value does not fit in an int
bool intLiteralValue(const std::string &spelling, long long &value);

//'a', '\n', '\x41', '\0'
long long charLiteralValue(const std::string &spelling);

//the number of characters a string literal holds, without the closing \0
size_t stringLiteralLength(const std::string &spelling);

//works out an integer constant expression: literals, + - * / % and the other
//arithmetic, bitwise, logical and comparison operators, and ?:. false when the
//expression is not a constant, as for anything naming a variable
bool evaluateConstant(const ast::Expr *expr, long long &value);

} // namespace sema

#endif
