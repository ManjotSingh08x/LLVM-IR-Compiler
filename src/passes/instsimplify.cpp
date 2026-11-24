#include "passes/instsimplify.h"

#include <map>
#include <set>

#include "dataflow/dfa_framework.h"
#include "passes/fold.h"
#include "passes/local_dag.h"

namespace passes {

using namespace ir;

namespace {

bool isAssociative(BinaryOpKind op) {
    return op == BinaryOpKind::Add || op == BinaryOpKind::Mul || op == BinaryOpKind::And ||
           op == BinaryOpKind::Or || op == BinaryOpKind::Xor;
}

bool reassociate(Module &module, Instruction &inst) {
    if (inst.getOpcode() != Opcode::Binary) return false;
    BinaryInst &outer = static_cast<BinaryInst &>(inst);
    if (integerBits(outer.operandType) < 8) return false;
    if (outer.op == BinaryOpKind::Shl && outer.lhs->getKind() == ValueKind::Instruction) {
        const Constant *b = asConstant(outer.rhs);
        Instruction *lhs = static_cast<Instruction *>(outer.lhs);
        if (b == nullptr || b->constKind != ConstantKind::Int || lhs->getOpcode() != Opcode::Binary) return false;
        BinaryInst &inner = *static_cast<BinaryInst *>(lhs);
        const Constant *a = asConstant(inner.rhs);
        if (inner.op != BinaryOpKind::Shl || a == nullptr || a->constKind != ConstantKind::Int) return false;
        long long total = a->intValue + b->intValue;
        if (a->intValue < 0 || b->intValue < 0 || total >= (long long)integerBits(outer.operandType)) return false;
        outer.lhs = inner.lhs;
        outer.rhs = module.getInt(outer.getType(), total);
        return true;
    }
    if (!isAssociative(outer.op)) return false;
    const Constant *c2 = asConstant(outer.rhs);
    if (c2 == nullptr || c2->constKind != ConstantKind::Int) return false;
    if (outer.lhs->getKind() != ValueKind::Instruction) return false;
    Instruction *lhs = static_cast<Instruction *>(outer.lhs);
    if (lhs->getOpcode() != Opcode::Binary) return false;
    BinaryInst &inner = *static_cast<BinaryInst *>(lhs);
    if (inner.op != outer.op) return false;
    const Constant *c1 = asConstant(inner.rhs);
    if (c1 == nullptr || c1->constKind != ConstantKind::Int) return false;
    Constant *combined = foldBinary(module, outer.op, outer.operandType, c1, c2, outer.getType());
    if (combined == nullptr) return false;
    outer.lhs = inner.lhs;
    outer.rhs = combined;
    return true;
}

Value *sameIncoming(PhiInst *phi) {
    Value *value = nullptr;
    for (const std::pair<Value *, BasicBlock *> &in : phi->incoming) {
        if (in.first == phi) continue;
        if (value != nullptr && in.first != value) {
            const Constant *a = asConstant(value);
            const Constant *b = asConstant(in.first);
            if (!(a && b && isNumber(a) && isNumber(b) && sameNumber(a, b))) return nullptr;
            continue;
        }
        value = in.first;
    }
    return value;
}

}

bool InstSimplify::run(Function &fn, Module &module, AnalysisManager &am) {
    (void)am;
    bool changedAny = false;
    for (int round = 0; round < 8; round++) {
        std::map<Value *, Value *> replaced;
        auto resolve = [&](Value *value) {
            std::map<Value *, Value *>::iterator it = replaced.find(value);
            while (it != replaced.end()) {
                value = it->second;
                it = replaced.find(value);
            }
            return value;
        };
        bool changed = false;
        for (BasicBlock *block : reversePostorder(fn)) {
            for (std::unique_ptr<Instruction> &owned : block->instructions) {
                Instruction *inst = owned.get();
                for (Value *operand : inst->getOperands()) {
                    Value *now = resolve(operand);
                    if (now != operand) {
                        inst->replaceOperand(operand, now);
                        changed = true;
                    }
                }
                if (replaced.count(inst)) continue;
                if (inst->getOpcode() == Opcode::Phi) {
                    Value *same = sameIncoming(static_cast<PhiInst *>(inst));
                    if (same != nullptr && same != inst) {
                        replaced[inst] = same;
                        changed = true;
                    }
                    continue;
                }
                if (inst->hasSideEffects() || inst->getName().empty()) continue;
                std::vector<const Constant *> constants;
                for (Value *operand : inst->getOperands()) constants.push_back(asConstant(operand));
                if (inst->getOpcode() != Opcode::GEP && inst->getOpcode() != Opcode::Load) {
                    if (Value *folded = foldInstruction(module, *inst, constants)) {
                        replaced[inst] = folded;
                        changed = true;
                        continue;
                    }
                }
                bool rewritten = false;
                if (Value *same = simplifyInstruction(module, *inst, rewritten)) {
                    replaced[inst] = same;
                    changed = true;
                    continue;
                }
                if (rewritten || reassociate(module, *inst)) changed = true;
            }
        }
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                for (Value *operand : inst->getOperands()) {
                    Value *now = resolve(operand);
                    if (now != operand) inst->replaceOperand(operand, now);
                }
            }
        }
        if (removeUnusedValues(fn)) changed = true;
        if (!changed) break;
        changedAny = true;
    }
    return changedAny;
}

}
