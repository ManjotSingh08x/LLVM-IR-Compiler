#ifndef SEMANTIC_CONSTANTS_H
#define SEMANTIC_CONSTANTS_H

#include <string>

#include "ast/ast.h"
#include "semantic/types.h"

namespace sema {

bool intLiteral(const std::string &spelling, long long &value, const Type *&type);

long long wrapToType(long long value, const Type *type);

double floatLiteral(const std::string &spelling, const Type *&type);

long long charLiteralValue(const std::string &spelling);

size_t stringLiteralLength(const std::string &spelling);

std::string decodeStringLiteral(const std::string &spelling);

bool evaluateConstant(const ast::Expr *expr, long long &value);

}

#endif
