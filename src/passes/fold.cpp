#include "passes/fold.h"

#include <cmath>
#include <cstdint>
#include <cstring>

namespace passes {

using namespace ir;

namespace {

bool isFloatType(const sema::Type *type) { return type->is(sema::TypeKind::Float); }

//an integer constant read as signed at its width; an i1 true is -1 when signed
long long signedValue(const Constant *c) {
    if (integerBits(c->getType()) == 1) return c->intValue ? -1 : 0;
    return c->intValue;
}

//the same bits read as unsigned at the width
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
        if (y == 0 || (x == minimumOf(bits) && y == -1)) return nullptr;  //traps on the machine
        return module.getInt(type, op == BinaryOpKind::SDiv ? x / y : x % y);
    case BinaryOpKind::Shl:
        if (y < 0 || y >= (long long)bits) return nullptr;  //poison in llvm
        return module.getInt(type, (long long)(ux << y));
    case BinaryOpKind::AShr:
        if (y < 0 || y >= (long long)bits) return nullptr;
        return module.getInt(type, x >> y);
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

} // namespace

bool isNumber(const Value *value) {
    if (value == nullptr || value->getKind() != ValueKind::Constant) return false;
    ConstantKind kind = static_cast<const Constant *>(value)->constKind;
    return kind == ConstantKind::Int || kind == ConstantKind::Float;
}

bool sameNumber(const Constant *a, const Constant *b) {
    if (a->constKind != b->constKind || !sema::isSameType(a->getType(), b->getType()) ||
        isInt64(a->getType()) != isInt64(b->getType())) {
        return false;
    }
    if (a->constKind == ConstantKind::Int) return a->intValue == b->intValue;
    return std::memcmp(&a->floatValue, &b->floatValue, sizeof(double)) == 0;
}

Constant *foldBinary(Module &module, BinaryOpKind op, const sema::Type *operandType, const Constant *lhs,
                     const Constant *rhs, const sema::Type *resultType) {
    if (!isNumber(lhs) || !isNumber(rhs)) {
        //null pointers compare equal to each other
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

    //floating: computed in the precision of the type, as sse does
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
        //an i64 through double then float could round twice, the machine rounds once
        if (integerBits(operand->getType()) == 64 && isFloatType(toType)) return nullptr;
        return module.getFloat(toType, op == CastOpKind::SIToFP ? (double)signedValue(operand)
                                                                : (double)unsignedValue(operand));
    case CastOpKind::FPToSI: {
        double v = operand->floatValue;
        unsigned bits = integerBits(toType);
        if (std::isnan(v) || bits == 0) return nullptr;
        double t = std::trunc(v);
        //out of range is poison in llvm and a different answer on the machine: leave it
        double lo = bits == 1 ? -1.0 : -std::ldexp(1.0, bits - 1);
        double hi = bits == 1 ? 0.0 : std::ldexp(1.0, bits - 1) - 1.0;
        if (t < lo || t > hi) return nullptr;
        return module.getInt(toType, (long long)t);
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

} // namespace passes
