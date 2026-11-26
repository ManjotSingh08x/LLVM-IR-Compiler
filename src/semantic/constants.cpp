#include "semantic/constants.h"

#include <climits>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "semantic/symbols.h"

namespace sema {

namespace {

int digitValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 99;
}

long long readChar(const std::string &text, size_t &i) {
    if (text[i] != '\\') return (unsigned char)text[i++];

    i++;
    char c = text[i++];
    switch (c) {
    case 'n':  return '\n';
    case 't':  return '\t';
    case 'r':  return '\r';
    case 'a':  return '\a';
    case 'b':  return '\b';
    case 'f':  return '\f';
    case 'v':  return '\v';
    case '0':  return 0;
    case 'x': {
        long long value = 0;
        for (int n = 0; n < 2 && i < text.size() && digitValue(text[i]) < 16; n++) {
            value = value * 16 + digitValue(text[i++]);
        }
        return value;
    }
    default:   return (unsigned char)c;
    }
}

}

bool intLiteral(const std::string &spelling, long long &value, const Type *&type) {
    size_t end = spelling.size();
    bool isUnsignedSuffix = false;
    int longs = 0;
    while (end > 0 && strchr("uUlL", spelling[end - 1])) {
        end--;
        if (spelling[end] == 'u' || spelling[end] == 'U') isUnsignedSuffix = true;
        else longs++;
    }
    std::string digits = spelling.substr(0, end);

    int base = 10;
    size_t i = 0;
    if (digits.size() > 1 && digits[0] == '0') {
        if (digits[1] == 'x' || digits[1] == 'X') {
            base = 16;
            i = 2;
        } else if (digits[1] == 'b' || digits[1] == 'B') {
            base = 2;
            i = 2;
        } else {
            base = 8;
            i = 1;
        }
    }

    unsigned long long raw = 0;
    for (; i < digits.size(); i++) {
        unsigned long long digit = (unsigned long long)digitValue(digits[i]);
        if (raw > (ULLONG_MAX - digit) / (unsigned long long)base) return false;
        raw = raw * base + digit;
    }
    value = (long long)raw;

    std::vector<const Type *> candidates;
    if (isUnsignedSuffix && longs) candidates = { ulongType() };
    else if (isUnsignedSuffix) candidates = { uintType(), ulongType() };
    else if (longs) candidates = base == 10 ? std::vector<const Type *>{ longType() }
                                            : std::vector<const Type *>{ longType(), ulongType() };
    else candidates = base == 10 ? std::vector<const Type *>{ intType(), longType() }
                                 : std::vector<const Type *>{ intType(), uintType(), longType(), ulongType() };
    for (const Type *candidate : candidates) {
        unsigned long long max = candidate == intType()    ? (unsigned long long)INT_MAX
                               : candidate == uintType()   ? (unsigned long long)UINT_MAX
                               : candidate == longType()   ? (unsigned long long)LLONG_MAX
                                                           : ULLONG_MAX;
        if (raw <= max) {
            type = candidate;
            return true;
        }
    }
    type = candidates.back();
    return false;
}

long long wrapToType(long long value, const Type *type) {
    if (type == nullptr || !isIntegral(type)) return value;
    if (type->is(TypeKind::Bool)) return value != 0;
    unsigned bits = integerSize(type) * 8;
    if (bits == 64) return value;
    unsigned long long mask = (1ULL << bits) - 1;
    unsigned long long low = (unsigned long long)value & mask;
    if (isUnsigned(type)) return (long long)low;
    unsigned long long sign = 1ULL << (bits - 1);
    return (long long)((low ^ sign) - sign);
}

double floatLiteral(const std::string &spelling, const Type *&type) {
    char last = spelling.empty() ? 0 : spelling.back();
    if (last == 'f' || last == 'F') {
        type = floatType();
        return std::strtof(spelling.c_str(), nullptr);
    }
    type = doubleType();
    return std::strtod(spelling.c_str(), nullptr);
}

long long charLiteralValue(const std::string &spelling) {
    size_t i = 1;
    return readChar(spelling, i);
}

size_t stringLiteralLength(const std::string &spelling) {
    size_t length = 0;
    size_t i = 1;
    while (i + 1 < spelling.size()) {
        readChar(spelling, i);
        length++;
    }
    return length;
}

std::string decodeStringLiteral(const std::string &spelling) {
    std::string bytes;
    size_t i = 1;
    while (i + 1 < spelling.size()) {
        bytes += (char)readChar(spelling, i);
    }
    return bytes;
}

namespace {

bool evaluate(const ast::Expr *expr, long long &value);

bool constVariableValue(const Symbol *symbol, long long &value) {
    static int depth = 0;
    if (symbol == nullptr || symbol->kind != SymbolKind::Variable || !symbol->type->isConst()
        || !isIntegral(symbol->type) || symbol->decl == nullptr
        || symbol->decl->getKind() != ast::ASTKind::VarDecl || depth > 32) {
        return false;
    }
    const ast::VarDecl *var = static_cast<const ast::VarDecl *>(symbol->decl);
    const ast::Expr *init = var->init.get();
    if (init && init->getKind() == ast::ASTKind::InitListExpr) {
        const ast::InitListExpr *list = static_cast<const ast::InitListExpr *>(init);
        init = list->inits.size() == 1 ? list->inits[0].get() : nullptr;
    }
    if (var->initStyle == ast::InitStyle::CtorArgs && var->ctorArgs.size() == 1) init = var->ctorArgs[0].get();
    depth++;
    bool known = init && evaluateConstant(init, value);
    depth--;
    if (known) value = wrapToType(value, symbol->type);
    return known;
}

bool unsignedOperands(const ast::Expr *operand) { return operand->type && isUnsigned(operand->type); }

}

bool evaluateConstant(const ast::Expr *expr, long long &value) {
    if (!evaluate(expr, value)) return false;
    value = wrapToType(value, expr->type);
    return true;
}

namespace {

bool evaluate(const ast::Expr *expr, long long &value) {
    if (expr == nullptr) return false;

    switch (expr->getKind()) {
    case ast::ASTKind::IntLiteral: {
        const Type *type = nullptr;
        return intLiteral(static_cast<const ast::IntLiteral *>(expr)->spelling, value, type);
    }
    case ast::ASTKind::CharLiteral:
        value = charLiteralValue(static_cast<const ast::CharLiteral *>(expr)->spelling);
        return true;
    case ast::ASTKind::BoolLiteral:
        value = static_cast<const ast::BoolLiteral *>(expr)->value ? 1 : 0;
        return true;
    case ast::ASTKind::ParenExpr:
        return evaluateConstant(static_cast<const ast::ParenExpr *>(expr)->sub.get(), value);
    case ast::ASTKind::ImplicitCastExpr:
        return evaluateConstant(static_cast<const ast::ImplicitCastExpr *>(expr)->sub.get(), value);
    case ast::ASTKind::CastExpr:
        return expr->type && isIntegral(expr->type) &&
               evaluateConstant(static_cast<const ast::CastExpr *>(expr)->sub.get(), value);
    case ast::ASTKind::SizeofExpr:
        value = (long long)static_cast<const ast::SizeofExpr *>(expr)->size;
        return true;
    case ast::ASTKind::DeclRefExpr:
        return constVariableValue(static_cast<const ast::DeclRefExpr *>(expr)->symbol, value);

    case ast::ASTKind::UnaryExpr: {
        const ast::UnaryExpr *unary = static_cast<const ast::UnaryExpr *>(expr);
        long long operand;
        if (!evaluateConstant(unary->operand.get(), operand)) return false;
        switch (unary->op) {
        case ast::UnaryOp::Plus:       value = operand; return true;
        case ast::UnaryOp::Minus:      value = -operand; return true;
        case ast::UnaryOp::BitNot:     value = ~operand; return true;
        case ast::UnaryOp::LogicalNot: value = !operand; return true;
        default:                       return false;
        }
    }

    case ast::ASTKind::BinaryExpr: {
        const ast::BinaryExpr *binary = static_cast<const ast::BinaryExpr *>(expr);
        long long a, b;
        if (!evaluateConstant(binary->lhs.get(), a) || !evaluateConstant(binary->rhs.get(), b)) return false;
        unsigned long long ua = (unsigned long long)a, ub = (unsigned long long)b;
        bool isUnsignedOp = unsignedOperands(binary->lhs.get());
        switch (binary->op) {
        case ast::BinaryOp::Add:        value = (long long)(ua + ub); return true;
        case ast::BinaryOp::Sub:        value = (long long)(ua - ub); return true;
        case ast::BinaryOp::Mul:        value = (long long)(ua * ub); return true;
        case ast::BinaryOp::Div:
            if (b == 0 || (!isUnsignedOp && a == LLONG_MIN && b == -1)) return false;
            value = isUnsignedOp ? (long long)(ua / ub) : a / b;
            return true;
        case ast::BinaryOp::Rem:
            if (b == 0 || (!isUnsignedOp && a == LLONG_MIN && b == -1)) return false;
            value = isUnsignedOp ? (long long)(ua % ub) : a % b;
            return true;
        case ast::BinaryOp::Shl:        value = (long long)(ua << (ub & 63)); return true;
        case ast::BinaryOp::Shr:        value = isUnsignedOp ? (long long)(ua >> (ub & 63)) : a >> (ub & 63); return true;
        case ast::BinaryOp::BitAnd:     value = a & b; return true;
        case ast::BinaryOp::BitOr:      value = a | b; return true;
        case ast::BinaryOp::BitXor:     value = a ^ b; return true;
        case ast::BinaryOp::LogicalAnd: value = a && b; return true;
        case ast::BinaryOp::LogicalOr:  value = a || b; return true;
        case ast::BinaryOp::Eq:         value = a == b; return true;
        case ast::BinaryOp::Ne:         value = a != b; return true;
        case ast::BinaryOp::Lt:         value = isUnsignedOp ? ua < ub : a < b; return true;
        case ast::BinaryOp::Gt:         value = isUnsignedOp ? ua > ub : a > b; return true;
        case ast::BinaryOp::Le:         value = isUnsignedOp ? ua <= ub : a <= b; return true;
        case ast::BinaryOp::Ge:         value = isUnsignedOp ? ua >= ub : a >= b; return true;
        }
        return false;
    }

    case ast::ASTKind::ConditionalExpr: {
        const ast::ConditionalExpr *cond = static_cast<const ast::ConditionalExpr *>(expr);
        long long test;
        if (!evaluateConstant(cond->cond.get(), test)) return false;
        return evaluateConstant(test ? cond->trueExpr.get() : cond->falseExpr.get(), value);
    }

    default:
        return false;
    }
}

}

}
