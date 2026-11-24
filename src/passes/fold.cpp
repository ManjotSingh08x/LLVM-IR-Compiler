#include "passes/fold.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace passes {

using namespace ir;

namespace {

bool isFloatType(const sema::Type *type) { return type->is(sema::TypeKind::Float); }

long long signedValue(const Constant *c) {
    if (integerBits(c->getType()) == 1) return c->intValue ? -1 : 0;
    return c->intValue;
}

unsigned long long unsignedValue(const Constant *c) {
    unsigned bits = integerBits(c->getType());
    unsigned long long v = (unsigned long long)c->intValue;
    if (bits == 0 || bits >= 64) return v;
    return v & ((1ULL << bits) - 1);
}

long long minimumOf(unsigned bits) { return bits >= 64 ? INT64_MIN : -(1LL << (bits - 1)); }

Constant *foldInt(Module &module, BinaryOpKind op, const sema::Type *type, const Constant *a, const Constant *b) {
    unsigned bits = integerBits(type);
    if (bits == 0) return nullptr;
    long long x = signedValue(a), y = signedValue(b);
    unsigned long long ux = (unsigned long long)x, uy = (unsigned long long)y;
    switch (op) {
    case BinaryOpKind::Add: return module.getInt(type, (long long)(ux + uy));
    case BinaryOpKind::Sub: return module.getInt(type, (long long)(ux - uy));
    case BinaryOpKind::Mul: return module.getInt(type, (long long)(ux * uy));
    case BinaryOpKind::SDiv:
    case BinaryOpKind::SRem:
        if (y == 0 || (x == minimumOf(bits) && y == -1)) return nullptr;
        return module.getInt(type, op == BinaryOpKind::SDiv ? x / y : x % y);
    case BinaryOpKind::UDiv:
    case BinaryOpKind::URem: {
        unsigned long long n = unsignedValue(a), d = unsignedValue(b);
        if (d == 0) return nullptr;
        return module.getInt(type, (long long)(op == BinaryOpKind::UDiv ? n / d : n % d));
    }
    case BinaryOpKind::Shl:
        if (y < 0 || y >= (long long)bits) return nullptr;
        return module.getInt(type, (long long)(ux << y));
    case BinaryOpKind::AShr:
        if (y < 0 || y >= (long long)bits) return nullptr;
        return module.getInt(type, x >> y);
    case BinaryOpKind::LShr:
        if (y < 0 || y >= (long long)bits) return nullptr;
        return module.getInt(type, (long long)(unsignedValue(a) >> y));
    case BinaryOpKind::And: return module.getInt(type, x & y);
    case BinaryOpKind::Or:  return module.getInt(type, x | y);
    case BinaryOpKind::Xor: return module.getInt(type, x ^ y);
    default: return nullptr;
    }
}

bool compareInts(BinaryOpKind op, const Constant *a, const Constant *b, bool &result) {
    long long x = signedValue(a), y = signedValue(b);
    unsigned long long ux = unsignedValue(a), uy = unsignedValue(b);
    switch (op) {
    case BinaryOpKind::ICmpEQ:  result = ux == uy; return true;
    case BinaryOpKind::ICmpNE:  result = ux != uy; return true;
    case BinaryOpKind::ICmpSLT: result = x < y; return true;
    case BinaryOpKind::ICmpSGT: result = x > y; return true;
    case BinaryOpKind::ICmpSLE: result = x <= y; return true;
    case BinaryOpKind::ICmpSGE: result = x >= y; return true;
    case BinaryOpKind::ICmpULT: result = ux < uy; return true;
    case BinaryOpKind::ICmpUGT: result = ux > uy; return true;
    case BinaryOpKind::ICmpULE: result = ux <= uy; return true;
    case BinaryOpKind::ICmpUGE: result = ux >= uy; return true;
    default: return false;
    }
}

bool compareFloats(BinaryOpKind op, double x, double y, bool &result) {
    bool unordered = std::isnan(x) || std::isnan(y);
    switch (op) {
    case BinaryOpKind::FCmpOEQ: result = !unordered && x == y; return true;
    case BinaryOpKind::FCmpUNE: result = unordered || x != y; return true;
    case BinaryOpKind::FCmpOLT: result = !unordered && x < y; return true;
    case BinaryOpKind::FCmpOGT: result = !unordered && x > y; return true;
    case BinaryOpKind::FCmpOLE: result = !unordered && x <= y; return true;
    case BinaryOpKind::FCmpOGE: result = !unordered && x >= y; return true;
    default: return false;
    }
}

bool isIntConst(const Value *value, long long v) {
    const Constant *c = asConstant(value);
    return c && c->constKind == ConstantKind::Int && c->intValue == ir::normalizeInt(v, c->getType());
}

bool isFloatConst(const Value *value, double v, bool negativeZero) {
    const Constant *c = asConstant(value);
    if (!c || c->constKind != ConstantKind::Float || c->floatValue != v) return false;
    return std::signbit(c->floatValue) == negativeZero;
}

bool isCommutative(BinaryOpKind op) {
    switch (op) {
    case BinaryOpKind::Add: case BinaryOpKind::Mul: case BinaryOpKind::And: case BinaryOpKind::Or:
    case BinaryOpKind::Xor: case BinaryOpKind::ICmpEQ: case BinaryOpKind::ICmpNE:
    case BinaryOpKind::FAdd: case BinaryOpKind::FMul: case BinaryOpKind::FCmpOEQ: case BinaryOpKind::FCmpUNE:
        return true;
    default:
        return false;
    }
}

}

const Constant *asConstant(const Value *value) {
    return value->getKind() == ValueKind::Constant ? static_cast<const Constant *>(value) : nullptr;
}

bool isNumber(const Value *value) {
    if (value == nullptr || value->getKind() != ValueKind::Constant) return false;
    ConstantKind kind = static_cast<const Constant *>(value)->constKind;
    return kind == ConstantKind::Int || kind == ConstantKind::Float;
}

bool sameNumber(const Constant *a, const Constant *b) {
    if (a->constKind != b->constKind || !sema::isSameUnqualified(a->getType(), b->getType())) {
        return false;
    }
    if (a->constKind == ConstantKind::Int) return a->intValue == b->intValue;
    return std::memcmp(&a->floatValue, &b->floatValue, sizeof(double)) == 0;
}

Constant *foldBinary(Module &module, BinaryOpKind op, const sema::Type *operandType, const Constant *lhs,
                     const Constant *rhs, const sema::Type *resultType) {
    if (!isNumber(lhs) || !isNumber(rhs)) {
        if (lhs->constKind == ConstantKind::Null && rhs->constKind == ConstantKind::Null) {
            if (op == BinaryOpKind::ICmpEQ) return module.getBool(true);
            if (op == BinaryOpKind::ICmpNE) return module.getBool(false);
        }
        return nullptr;
    }
    bool truth = false;
    if (isCompare(op)) {
        if (lhs->constKind == ConstantKind::Int && compareInts(op, lhs, rhs, truth)) return module.getBool(truth);
        if (lhs->constKind == ConstantKind::Float && compareFloats(op, lhs->floatValue, rhs->floatValue, truth)) {
            return module.getBool(truth);
        }
        return nullptr;
    }
    if (lhs->constKind == ConstantKind::Int) return foldInt(module, op, resultType, lhs, rhs);

    double x = lhs->floatValue, y = rhs->floatValue, r;
    bool single = isFloatType(operandType);
    switch (op) {
    case BinaryOpKind::FAdd: r = single ? (double)((float)x + (float)y) : x + y; break;
    case BinaryOpKind::FSub: r = single ? (double)((float)x - (float)y) : x - y; break;
    case BinaryOpKind::FMul: r = single ? (double)((float)x * (float)y) : x * y; break;
    case BinaryOpKind::FDiv: r = single ? (double)((float)x / (float)y) : x / y; break;
    default: return nullptr;
    }
    return module.getFloat(resultType, r);
}

Constant *foldUnary(Module &module, UnaryOpKind op, const Constant *operand) {
    if (op != UnaryOpKind::FNeg || operand->constKind != ConstantKind::Float) return nullptr;
    return module.getFloat(operand->getType(), -operand->floatValue);
}

Constant *foldCast(Module &module, CastOpKind op, const Constant *operand, const sema::Type *fromType,
                   const sema::Type *toType) {
    (void)fromType;
    if (op == CastOpKind::PtrToInt && operand->constKind == ConstantKind::Null) return module.getInt(toType, 0);
    if (op == CastOpKind::IntToPtr && operand->constKind == ConstantKind::Int && operand->intValue == 0) {
        return module.getNull(toType);
    }
    if (!isNumber(operand)) return nullptr;
    switch (op) {
    case CastOpKind::SExt:
    case CastOpKind::Trunc:
        return module.getInt(toType, signedValue(operand));
    case CastOpKind::ZExt:
        return module.getInt(toType, (long long)unsignedValue(operand));
    case CastOpKind::SIToFP:
    case CastOpKind::UIToFP:
        if (integerBits(operand->getType()) == 64 && isFloatType(toType)) return nullptr;
        return module.getFloat(toType, op == CastOpKind::SIToFP ? (double)signedValue(operand)
                                                                : (double)unsignedValue(operand));
    case CastOpKind::FPToSI: {
        double v = operand->floatValue;
        unsigned bits = integerBits(toType);
        if (std::isnan(v) || bits == 0) return nullptr;
        double t = std::trunc(v);
        double lo = bits == 1 ? -1.0 : -std::ldexp(1.0, bits - 1);
        double hi = bits == 1 ? 0.0 : std::ldexp(1.0, bits - 1) - 1.0;
        if (t < lo || t > hi) return nullptr;
        return module.getInt(toType, (long long)t);
    }
    case CastOpKind::FPToUI: {
        double v = operand->floatValue;
        unsigned bits = integerBits(toType);
        if (std::isnan(v) || bits == 0) return nullptr;
        double t = std::trunc(v);
        if (t < 0.0 || t > std::ldexp(1.0, bits) - 1.0) return nullptr;
        return module.getInt(toType, (long long)(unsigned long long)t);
    }
    case CastOpKind::FPExt:
    case CastOpKind::FPTrunc:
        return module.getFloat(toType, operand->floatValue);
    default:
        return nullptr;
    }
}

Value *foldInstruction(Module &module, const Instruction &inst, const std::vector<const Constant *> &operands) {
    switch (inst.getOpcode()) {
    case Opcode::Binary: {
        const BinaryInst &b = static_cast<const BinaryInst &>(inst);
        if (operands.size() != 2 || !operands[0] || !operands[1]) return nullptr;
        return foldBinary(module, b.op, b.operandType, operands[0], operands[1], b.getType());
    }
    case Opcode::Unary:
        if (operands.size() != 1 || !operands[0]) return nullptr;
        return foldUnary(module, static_cast<const UnaryInst &>(inst).op, operands[0]);
    case Opcode::Cast: {
        const CastInst &c = static_cast<const CastInst &>(inst);
        if (operands.size() != 1 || !operands[0]) return nullptr;
        return foldCast(module, c.op, operands[0], c.fromType, c.toType);
    }
    case Opcode::Select: {
        const SelectInst &s = static_cast<const SelectInst &>(inst);
        if (operands.empty() || !operands[0] || operands[0]->constKind != ConstantKind::Int) return nullptr;
        return operands[0]->intValue ? s.ifTrue : s.ifFalse;
    }
    default:
        return nullptr;
    }
}

Value *simplifyInstruction(Module &module, Instruction &inst, bool &rewritten) {
    rewritten = false;
    switch (inst.getOpcode()) {
    case Opcode::Binary: {
        BinaryInst &b = static_cast<BinaryInst &>(inst);
        Value *x = b.lhs, *y = b.rhs;
        const sema::Type *t = b.getType();
        switch (b.op) {
        case BinaryOpKind::Add:
            if (isIntConst(y, 0)) return x;
            if (isIntConst(x, 0)) return y;
            break;
        case BinaryOpKind::Sub:
            if (isIntConst(y, 0)) return x;
            if (x == y) return module.getInt(t, 0);
            break;
        case BinaryOpKind::Mul: {
            if (isIntConst(y, 1)) return x;
            if (isIntConst(x, 1)) return y;
            if (isIntConst(y, 0) || isIntConst(x, 0)) return module.getInt(t, 0);
            Value *other = x;
            const Constant *c = asConstant(y);
            if (!c || c->constKind != ConstantKind::Int) {
                other = y;
                c = asConstant(x);
            }
            if (c && c->constKind == ConstantKind::Int && integerBits(t) >= 8 && c->intValue > 1 &&
                (c->intValue & (c->intValue - 1)) == 0) {
                long long k = 0;
                while ((1LL << k) < c->intValue) k++;
                b.op = BinaryOpKind::Shl;
                b.lhs = other;
                b.rhs = module.getInt(t, k);
                rewritten = true;
            }
            break;
        }
        case BinaryOpKind::SDiv:
        case BinaryOpKind::UDiv:
            if (isIntConst(y, 1)) return x;
            break;
        case BinaryOpKind::SRem:
        case BinaryOpKind::URem:
            if (isIntConst(y, 1)) return module.getInt(t, 0);
            break;
        case BinaryOpKind::Shl:
        case BinaryOpKind::AShr:
        case BinaryOpKind::LShr:
            if (isIntConst(y, 0)) return x;
            break;
        case BinaryOpKind::And:
            if (isIntConst(y, 0) || isIntConst(x, 0)) return module.getInt(t, 0);
            if (x == y || isIntConst(y, -1)) return x;
            if (isIntConst(x, -1)) return y;
            break;
        case BinaryOpKind::Or:
            if (isIntConst(y, 0) || x == y) return x;
            if (isIntConst(x, 0)) return y;
            break;
        case BinaryOpKind::Xor:
            if (isIntConst(y, 0)) return x;
            if (isIntConst(x, 0)) return y;
            if (x == y) return module.getInt(t, 0);
            break;
        case BinaryOpKind::ICmpEQ: case BinaryOpKind::ICmpSLE: case BinaryOpKind::ICmpSGE:
        case BinaryOpKind::ICmpULE: case BinaryOpKind::ICmpUGE:
            if (x == y) return module.getBool(true);
            break;
        case BinaryOpKind::ICmpNE: case BinaryOpKind::ICmpSLT: case BinaryOpKind::ICmpSGT:
        case BinaryOpKind::ICmpULT: case BinaryOpKind::ICmpUGT:
            if (x == y) return module.getBool(false);
            break;
        case BinaryOpKind::FAdd:
            if (isFloatConst(y, 0.0, true)) return x;
            if (isFloatConst(x, 0.0, true)) return y;
            break;
        case BinaryOpKind::FSub:
            if (isFloatConst(y, 0.0, false)) return x;
            break;
        case BinaryOpKind::FMul:
            if (isFloatConst(y, 1.0, false)) return x;
            if (isFloatConst(x, 1.0, false)) return y;
            break;
        case BinaryOpKind::FDiv:
            if (isFloatConst(y, 1.0, false)) return x;
            break;
        default:
            break;
        }
        return nullptr;
    }
    case Opcode::Unary: {
        UnaryInst &u = static_cast<UnaryInst &>(inst);
        if (u.operand->getKind() == ValueKind::Instruction &&
            static_cast<Instruction *>(u.operand)->getOpcode() == Opcode::Unary) {
            return static_cast<UnaryInst *>(u.operand)->operand;
        }
        return nullptr;
    }
    case Opcode::Cast: {
        CastInst &c = static_cast<CastInst &>(inst);
        if (c.operand->getKind() != ValueKind::Instruction ||
            static_cast<Instruction *>(c.operand)->getOpcode() != Opcode::Cast) {
            return nullptr;
        }
        CastInst &inner = *static_cast<CastInst *>(c.operand);
        if ((c.op == CastOpKind::SExt || c.op == CastOpKind::ZExt) && inner.op == c.op) {
            c.operand = inner.operand;
            c.fromType = inner.fromType;
            rewritten = true;
            return nullptr;
        }
        if (c.op == CastOpKind::Trunc && (inner.op == CastOpKind::SExt || inner.op == CastOpKind::ZExt) &&
            formatType(inner.fromType) == formatType(c.toType)) {
            return inner.operand;
        }
        return nullptr;
    }
    case Opcode::Select: {
        SelectInst &s = static_cast<SelectInst &>(inst);
        if (s.ifTrue == s.ifFalse) return s.ifTrue;
        return nullptr;
    }
    default:
        return nullptr;
    }
}

std::string expressionKey(const Instruction &inst) {
    std::vector<std::string> ops;
    for (const Value *operand : inst.getOperands()) ops.push_back(operand->getTypedOperand());
    std::string key = std::to_string((int)inst.getOpcode()) + "|" + formatType(inst.getType());
    switch (inst.getOpcode()) {
    case Opcode::Binary: {
        const BinaryInst &b = static_cast<const BinaryInst &>(inst);
        key += "|" + std::to_string((int)b.op) + "|" + formatType(b.operandType) + (b.isExact ? "|exact" : "");
        if (isCommutative(b.op)) std::sort(ops.begin(), ops.end());
        break;
    }
    case Opcode::Unary:
        key += "|" + std::to_string((int)static_cast<const UnaryInst &>(inst).op);
        break;
    case Opcode::Cast: {
        const CastInst &c = static_cast<const CastInst &>(inst);
        key += "|" + std::to_string((int)c.op) + "|" + formatType(c.fromType);
        break;
    }
    case Opcode::GEP: {
        const GEPInst &g = static_cast<const GEPInst &>(inst);
        key += "|" + formatType(g.getAccessType()) + (g.inBounds ? "|inbounds" : "");
        break;
    }
    default:
        break;
    }
    for (const std::string &op : ops) key += "|" + op;
    return key;
}

}
