#include "semantic/constants.h"

#include <climits>

namespace sema {

namespace {

int digitValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 99;
}

//reads one possibly escaped character at i and advances i
long long readChar(const std::string &text, size_t &i) {
    if (text[i] != '\\') return (unsigned char)text[i++];

    i++;  //the backslash
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
    default:   return (unsigned char)c;  //\\ \' \" \?
    }
}

} // namespace

bool intLiteralValue(const std::string &spelling, long long &value) {
    int base = 10;
    size_t i = 0;
    if (spelling.size() > 1 && spelling[0] == '0') {
        if (spelling[1] == 'x' || spelling[1] == 'X') {
            base = 16;
            i = 2;
        } else if (spelling[1] == 'b' || spelling[1] == 'B') {
            base = 2;
            i = 2;
        } else {
            base = 8;
            i = 1;
        }
    }

    value = 0;
    for (; i < spelling.size(); i++) {
        value = value * base + digitValue(spelling[i]);
        if (value > INT_MAX) return false;
    }
    return true;
}

long long charLiteralValue(const std::string &spelling) {
    size_t i = 1;  //past the opening quote
    return readChar(spelling, i);
}

size_t stringLiteralLength(const std::string &spelling) {
    size_t length = 0;
    size_t i = 1;  //past the opening quote
    while (i + 1 < spelling.size()) {
        readChar(spelling, i);
        length++;
    }
    return length;
}

std::string decodeStringLiteral(const std::string &spelling) {
    std::string bytes;
    size_t i = 1;  //past the opening quote
    while (i + 1 < spelling.size()) {
        bytes += (char)readChar(spelling, i);
    }
    return bytes;
}

bool evaluateConstant(const ast::Expr *expr, long long &value) {
    if (expr == nullptr) return false;

    switch (expr->getKind()) {
    case ast::ASTKind::IntLiteral:
        return intLiteralValue(static_cast<const ast::IntLiteral *>(expr)->spelling, value);
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
        switch (binary->op) {
        case ast::BinaryOp::Add:        value = a + b; return true;
        case ast::BinaryOp::Sub:        value = a - b; return true;
        case ast::BinaryOp::Mul:        value = a * b; return true;
        case ast::BinaryOp::Div:        if (b == 0) return false; value = a / b; return true;
        case ast::BinaryOp::Rem:        if (b == 0) return false; value = a % b; return true;
        case ast::BinaryOp::Shl:        value = a << b; return true;
        case ast::BinaryOp::Shr:        value = a >> b; return true;
        case ast::BinaryOp::BitAnd:     value = a & b; return true;
        case ast::BinaryOp::BitOr:      value = a | b; return true;
        case ast::BinaryOp::BitXor:     value = a ^ b; return true;
        case ast::BinaryOp::LogicalAnd: value = a && b; return true;
        case ast::BinaryOp::LogicalOr:  value = a || b; return true;
        case ast::BinaryOp::Eq:         value = a == b; return true;
        case ast::BinaryOp::Ne:         value = a != b; return true;
        case ast::BinaryOp::Lt:         value = a < b; return true;
        case ast::BinaryOp::Gt:         value = a > b; return true;
        case ast::BinaryOp::Le:         value = a <= b; return true;
        case ast::BinaryOp::Ge:         value = a >= b; return true;
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

} // namespace sema
