#include "passes/local_dag.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

#include "passes/escape.h"
#include "passes/fold.h"

namespace passes {

using namespace ir;

namespace {

const Constant *asConstant(const Value *value) {
    return value->getKind() == ValueKind::Constant ? static_cast<const Constant *>(value) : nullptr;
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

bool isScalarType(const sema::Type *type) {
    return sema::isIntegral(type) || sema::isFloating(type) || type->is(sema::TypeKind::Pointer) || isInt64(type);
}

bool isPure(const Instruction &inst) {
    switch (inst.getOpcode()) {
    case Opcode::Binary:
    case Opcode::Unary:
    case Opcode::Cast:
    case Opcode::Select:
    case Opcode::GEP:
        return true;
    default:
        return false;
    }
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

//what makes two pure instructions compute the same value: opcode, kind, types and operands
std::string keyOf(const Instruction &inst) {
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

//algebraic identities (8.5.4, 8.7.4). returns the value inst equals, or null. may instead rewrite
//inst in place into something cheaper (x * 2^k -> x << k) and set rewritten
Value *simplify(Module &module, Instruction &inst, bool &rewritten) {
    rewritten = false;
    switch (inst.getOpcode()) {
    case Opcode::Binary: {
        BinaryInst &b = static_cast<BinaryInst &>(inst);
        Value *x = b.lhs, *y = b.rhs;
        const sema::Type *t = b.getType();
        bool ints = integerBits(b.operandType) != 0;
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
            //local reduction in strength: a multiply by 2^k is a shift
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
            if (isIntConst(y, 1)) return x;
            break;
        case BinaryOpKind::SRem:
            if (isIntConst(y, 1)) return module.getInt(t, 0);
            break;
        case BinaryOpKind::Shl:
        case BinaryOpKind::AShr:
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
        //floating: only the identities that hold for every input, -0.0 and NaN included
        case BinaryOpKind::FAdd:
            if (isFloatConst(y, 0.0, true)) return x;   //x + -0.0
            if (isFloatConst(x, 0.0, true)) return y;
            break;
        case BinaryOpKind::FSub:
            if (isFloatConst(y, 0.0, false)) return x;  //x - +0.0
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
        (void)ints;
        return nullptr;
    }
    case Opcode::Unary: {
        UnaryInst &u = static_cast<UnaryInst &>(inst);
        if (u.operand->getKind() == ValueKind::Instruction &&
            static_cast<Instruction *>(u.operand)->getOpcode() == Opcode::Unary) {
            return static_cast<UnaryInst *>(u.operand)->operand;  //-(-x)
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
        //sext(sext x) and zext(zext x) are one extension
        if ((c.op == CastOpKind::SExt || c.op == CastOpKind::ZExt) && inner.op == c.op) {
            c.operand = inner.operand;
            c.fromType = inner.fromType;
            rewritten = true;
            return nullptr;
        }
        //trunc(ext x) back to x's own type is x
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

//a value read from or written to memory, available until something may change that memory
struct Known {
    Value            *address;
    const sema::Type *type;
    Value            *value;
};

} // namespace

bool removeUnusedValues(Function &fn) {
    bool removedAny = false;
    for (;;) {
        std::map<const Value *, size_t> uses;
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                for (Value *operand : inst->getOperands()) uses[operand]++;
            }
        }
        std::vector<Instruction *> unused;
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                bool removable = isPure(*inst) || inst->getOpcode() == Opcode::Load || inst->getOpcode() == Opcode::Phi;
                if (removable && uses[inst.get()] == 0) unused.push_back(inst.get());
            }
        }
        if (unused.empty()) return removedAny;
        for (Instruction *inst : unused) inst->getParent()->erase(inst);
        removedAny = true;
    }
}

bool LocalDAG::run(Function &fn, Module &module, AnalysisManager &am) {
    (void)am;
    EscapeInfo escape(fn);
    std::map<Value *, Value *> replaced;  //an instruction -> the value that stands for it now
    std::set<Instruction *> dead;
    bool changed = false;

    auto resolve = [&](Value *value) {
        std::map<Value *, Value *>::iterator it = replaced.find(value);
        while (it != replaced.end()) {
            value = it->second;
            it = replaced.find(value);
        }
        return value;
    };
    auto replaceWith = [&](Instruction *inst, Value *value) {
        replaced[inst] = value;
        dead.insert(inst);
        changed = true;
    };

    for (BasicBlock *block : fn.getBlocks()) {
        std::map<std::string, Instruction *> available;  //value numbers of the block's expressions
        std::vector<Known> known;                        //memory contents the block knows
        std::vector<StoreInst *> unread;                 //stores nothing has read yet

        std::vector<Instruction *> order;
        for (std::unique_ptr<Instruction> &inst : block->instructions) order.push_back(inst.get());

        for (Instruction *inst : order) {
            for (Value *operand : inst->getOperands()) {
                Value *now = resolve(operand);
                if (now != operand) {
                    inst->replaceOperand(operand, now);
                    changed = true;
                }
            }

            switch (inst->getOpcode()) {
            case Opcode::Load: {
                LoadInst *load = static_cast<LoadInst *>(inst);
                //whatever this load reads, earlier stores to it are no longer unread
                unread.erase(std::remove_if(unread.begin(), unread.end(),
                                            [&](StoreInst *s) { return escape.mayAlias(s->address, load->address); }),
                             unread.end());
                if (!isScalarType(load->getType())) break;
                Value *same = nullptr;
                for (size_t i = known.size(); i-- > 0;) {
                    if (known[i].address == load->address && sema::isSameType(known[i].type, load->getType()) &&
                        isInt64(known[i].type) == isInt64(load->getType())) {
                        same = known[i].value;
                        break;
                    }
                }
                if (same) {
                    replaceWith(load, same);
                } else {
                    known.push_back(Known{ load->address, load->getType(), load });
                }
                break;
            }
            case Opcode::Store: {
                StoreInst *store = static_cast<StoreInst *>(inst);
                //an unread store to the same place is overwritten: it was dead
                for (size_t i = 0; i < unread.size();) {
                    if (unread[i]->address == store->address &&
                        formatType(unread[i]->getAccessType()) == formatType(store->getAccessType())) {
                        dead.insert(unread[i]);
                        unread.erase(unread.begin() + i);
                        changed = true;
                    } else {
                        i++;
                    }
                }
                known.erase(std::remove_if(known.begin(), known.end(),
                                           [&](const Known &k) { return escape.mayAlias(k.address, store->address); }),
                            known.end());
                if (isScalarType(store->getAccessType())) {
                    known.push_back(Known{ store->address, store->getAccessType(), store->value });
                }
                unread.push_back(store);
                break;
            }
            case Opcode::Call:
                known.erase(std::remove_if(known.begin(), known.end(),
                                           [&](const Known &k) { return escape.callMayTouch(k.address); }),
                            known.end());
                unread.erase(std::remove_if(unread.begin(), unread.end(),
                                            [&](StoreInst *s) { return escape.callMayTouch(s->address); }),
                             unread.end());
                break;
            case Opcode::Binary:
            case Opcode::Unary:
            case Opcode::Cast:
            case Opcode::Select:
            case Opcode::GEP: {
                //folding: every operand a constant (for select, the condition is enough)
                std::vector<const Constant *> constants;
                for (Value *operand : inst->getOperands()) constants.push_back(asConstant(operand));
                if (inst->getOpcode() != Opcode::GEP) {
                    if (Value *folded = foldInstruction(module, *inst, constants)) {
                        replaceWith(inst, folded);
                        break;
                    }
                }
                bool rewritten = false;
                if (Value *same = simplify(module, *inst, rewritten)) {
                    replaceWith(inst, same);
                    break;
                }
                if (rewritten) changed = true;
                //common subexpressions
                std::string key = keyOf(*inst);
                std::map<std::string, Instruction *>::iterator it = available.find(key);
                if (it != available.end()) {
                    replaceWith(inst, it->second);
                } else {
                    available[key] = inst;
                }
                break;
            }
            default:
                break;
            }
        }
    }

    //uses in other blocks follow the replacements too, then the replaced instructions go
    for (BasicBlock *block : fn.getBlocks()) {
        for (std::unique_ptr<Instruction> &inst : block->instructions) {
            if (dead.count(inst.get())) continue;
            for (Value *operand : inst->getOperands()) {
                Value *now = resolve(operand);
                if (now != operand) inst->replaceOperand(operand, now);
            }
        }
    }
    for (Instruction *inst : dead) inst->getParent()->erase(inst);
    if (removeUnusedValues(fn)) changed = true;
    return changed;
}

} // namespace passes
