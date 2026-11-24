#include "passes/iv_strength_reduce.h"

#include "passes/fold.h"

namespace passes {

using namespace ir;

namespace {

struct BasicIV {
    PhiInst *phi;
    Value   *init;
    long long step;
};

const Constant *intConstant(const Value *value) {
    const Constant *c = asConstant(value);
    return c && c->constKind == ConstantKind::Int ? c : nullptr;
}

//matches i = phi [init, pre], [i +/- c, latch]
bool matchBasicIV(PhiInst *phi, BasicBlock *pre, BasicBlock *latch, BasicIV &iv) {
    unsigned bits = integerBits(phi->getType());
    if ((bits != 32 && bits != 64) || phi->incoming.size() != 2) return false;
    Value *init = phi->getIncomingValueFor(pre);
    Value *next = phi->getIncomingValueFor(latch);
    if (init == nullptr || next == nullptr || next->getKind() != ValueKind::Instruction) return false;
    Instruction *inst = static_cast<Instruction *>(next);
    if (inst->getOpcode() != Opcode::Binary) return false;
    BinaryInst *b = static_cast<BinaryInst *>(inst);
    const Constant *c = nullptr;
    if (b->op == BinaryOpKind::Add && b->lhs == phi) c = intConstant(b->rhs);
    if (b->op == BinaryOpKind::Add && b->rhs == phi && !c) c = intConstant(b->lhs);
    if (b->op == BinaryOpKind::Sub && b->lhs == phi) {
        c = intConstant(b->rhs);
        if (c) {
            iv = BasicIV{ phi, init, -c->intValue };
            return true;
        }
    }
    if (c == nullptr) return false;
    iv = BasicIV{ phi, init, c->intValue };
    return true;
}

//before the latch terminator and the compare feeding it, so compare and branch can still fuse
Instruction *updatePoint(BasicBlock *latch) {
    Instruction *term = latch->getTerminator();
    if (term->getOpcode() == Opcode::Br && latch->instructions.size() >= 2) {
        BranchInst *br = static_cast<BranchInst *>(term);
        Instruction *before = latch->instructions[latch->instructions.size() - 2].get();
        if (br->isConditional() && br->condition == before) return before;
    }
    return term;
}

void replaceUses(Function &fn, Value *from, Value *to) {
    for (BasicBlock *block : fn.getBlocks()) {
        for (std::unique_ptr<Instruction> &inst : block->instructions) {
            if (inst.get() != to) inst->replaceOperand(from, to);
        }
    }
}

} // namespace

LoopResult IVStrengthReduce::runOnLoop(Loop &loop, LoopContext &ctx) {
    BasicBlock *pre = loop.preheader();
    if (pre == nullptr || loop.latches.size() != 1) return LoopResult::Unchanged;
    BasicBlock *latch = loop.latches.front();
    BasicBlock *header = loop.header;

    std::vector<BasicIV> ivs;
    for (std::unique_ptr<Instruction> &inst : header->instructions) {
        if (inst->getOpcode() != Opcode::Phi) break;
        BasicIV iv;
        if (matchBasicIV(static_cast<PhiInst *>(inst.get()), pre, latch, iv)) ivs.push_back(iv);
    }

    bool changed = false;
    Instruction *preTerm = pre->getTerminator();
    for (const BasicIV &iv : ivs) {
        const sema::Type *type = iv.phi->getType();
        std::vector<Instruction *> users = ctx.defUse.users(iv.phi);
        for (Instruction *user : users) {
            if (!loop.contains(user->getParent())) continue;

            //j = i * k or i << s
            if (user->getOpcode() == Opcode::Binary) {
                BinaryInst *b = static_cast<BinaryInst *>(user);
                const Constant *k = nullptr;
                if (b->op == BinaryOpKind::Mul) k = intConstant(b->lhs == iv.phi ? b->rhs : b->lhs);
                long long factor = 0;
                if (k) factor = k->intValue;
                if (b->op == BinaryOpKind::Shl && b->lhs == iv.phi) {
                    const Constant *s = intConstant(b->rhs);
                    if (s && s->intValue >= 0 && s->intValue < (long long)integerBits(type) - 1) factor = 1LL << s->intValue;
                }
                if (factor == 0 || integerBits(b->getType()) != integerBits(type)) continue;

                Value *start = nullptr;
                if (const Constant *c = intConstant(iv.init)) {
                    start = ctx.module.getInt(type, c->intValue * factor);
                } else {
                    Instruction *mul = new BinaryInst(BinaryOpKind::Mul, type, iv.init, ctx.module.getInt(type, factor), type,
                                                      ctx.fn.uniqueName(b->getName() + ".start"));
                    pre->insertBefore(preTerm, std::unique_ptr<Instruction>(mul));
                    start = mul;
                }
                PhiInst *phi = new PhiInst(type, ctx.fn.uniqueName(b->getName() + ".iv"));
                Instruction *next = new BinaryInst(BinaryOpKind::Add, type, phi, ctx.module.getInt(type, iv.step * factor),
                                                   type, ctx.fn.uniqueName(b->getName() + ".next"));
                phi->addIncoming(start, pre);
                phi->addIncoming(next, latch);
                header->insertAtFront(std::unique_ptr<Instruction>(phi));
                latch->insertBefore(updatePoint(latch), std::unique_ptr<Instruction>(next));
                replaceUses(ctx.fn, b, phi);
                changed = true;
                continue;
            }

            //p = getelementptr T, base, i
            if (user->getOpcode() == Opcode::GEP) {
                GEPInst *g = static_cast<GEPInst *>(user);
                if (ctx.isDefinedInLoop(g->base, loop) || g->indices.empty() || g->indices.back() != iv.phi) continue;
                const sema::Type *element = nullptr;
                if (g->indices.size() == 1) {
                    element = g->getAccessType();
                } else if (g->indices.size() == 2 && intConstant(g->indices[0]) && intConstant(g->indices[0])->intValue == 0) {
                    const sema::ArrayType *array = sema::asArray(g->getAccessType());
                    if (array) element = array->element;
                }
                if (element == nullptr) continue;

                std::vector<Value *> startIndices = g->indices;
                startIndices.back() = iv.init;
                GEPInst *start = new GEPInst(g->getAccessType(), g->getType(), g->base, startIndices, g->inBounds,
                                             ctx.fn.uniqueName(g->getName() + ".start"));
                pre->insertBefore(preTerm, std::unique_ptr<Instruction>(start));
                PhiInst *phi = new PhiInst(g->getType(), ctx.fn.uniqueName(g->getName() + ".iv"));
                GEPInst *next = new GEPInst(element, g->getType(), phi, std::vector<Value *>(1, ctx.module.getInt(type, iv.step)),
                                            false, ctx.fn.uniqueName(g->getName() + ".next"));
                phi->addIncoming(start, pre);
                phi->addIncoming(next, latch);
                header->insertAtFront(std::unique_ptr<Instruction>(phi));
                latch->insertBefore(updatePoint(latch), std::unique_ptr<Instruction>(next));
                replaceUses(ctx.fn, g, phi);
                changed = true;
            }
        }
    }
    return changed ? LoopResult::Changed : LoopResult::Unchanged;
}

} // namespace passes
