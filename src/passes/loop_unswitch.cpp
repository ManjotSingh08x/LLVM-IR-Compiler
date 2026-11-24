#include "passes/loop_unswitch.h"

#include "passes/clone.h"
#include "passes/simplify_cfg.h"

namespace passes {

using namespace ir;

namespace {

std::vector<PhiInst *> phisOf(BasicBlock *block) {
    std::vector<PhiInst *> phis;
    for (std::unique_ptr<Instruction> &inst : block->instructions) {
        if (inst->getOpcode() != Opcode::Phi) break;
        phis.push_back(static_cast<PhiInst *>(inst.get()));
    }
    return phis;
}

//the preheader's "br header" becomes "br cond, onTrue, onFalse"
void branchPreheader(BasicBlock *pre, Value *cond, BasicBlock *onTrue, BasicBlock *onFalse) {
    pre->erase(pre->getTerminator());
    pre->append(std::unique_ptr<Instruction>(new BranchInst(cond, onTrue, onFalse)));
}

//if (cond) leave; at the top of the loop: decide it in the preheader instead
bool unswitchTrivially(Loop &loop, BasicBlock *pre, BranchInst *br, LoopContext &ctx) {
    BasicBlock *header = loop.header;
    for (std::unique_ptr<Instruction> &inst : header->instructions) {
        if (inst.get() != br && inst->hasSideEffects()) return false;  //would be skipped on the way out
    }
    bool trueLeaves = !loop.contains(br->target);
    BasicBlock *exit = trueLeaves ? br->target : br->falseTarget;
    BasicBlock *stay = trueLeaves ? br->falseTarget : br->target;

    //the values the exit's phis get when leaving from the header, as they are in the preheader
    std::vector<std::pair<PhiInst *, Value *>> entries;
    for (PhiInst *phi : phisOf(exit)) {
        Value *value = phi->getIncomingValueFor(header);
        if (value == nullptr) continue;
        if (ctx.isDefinedInLoop(value, loop)) {
            Instruction *def = static_cast<Instruction *>(value);
            if (def->getParent() != header || def->getOpcode() != Opcode::Phi) return false;
            value = static_cast<PhiInst *>(def)->getIncomingValueFor(pre);
            if (value == nullptr) return false;
        }
        entries.push_back(std::make_pair(phi, value));
    }
    for (std::pair<PhiInst *, Value *> &entry : entries) entry.first->addIncoming(entry.second, pre);
    branchPreheader(pre, br->condition, trueLeaves ? exit : header, trueLeaves ? header : exit);
    branchTo(header, stay);
    return true;
}

//copy the loop: the original runs when cond is true, the copy when it is false
bool unswitchByCopy(Loop &loop, BasicBlock *pre, BasicBlock *block, BranchInst *br, LoopContext &ctx) {
    size_t size = 0;
    for (BasicBlock *b : loop.blocks) size += b->instructions.size();
    if (size > LoopUnswitch::kMaxSize) return false;
    if (!isLCSSA(loop, ctx.defUse)) return false;

    std::vector<BasicBlock *> blocks;
    BasicBlock *last = nullptr;
    for (BasicBlock *b : ctx.fn.getBlocks()) {
        if (loop.contains(b)) {
            blocks.push_back(b);
            last = b;
        }
    }
    ValueMap map;
    for (BasicBlock *b : blocks) {
        BasicBlock *copy = ctx.fn.createBlock(b->getName());
        ctx.fn.placeBlockAfter(copy, last);
        last = copy;
        map[b] = copy;
    }
    std::vector<Instruction *> copies;
    for (BasicBlock *b : blocks) {
        BasicBlock *copy = static_cast<BasicBlock *>(map[b]);
        for (std::unique_ptr<Instruction> &inst : b->instructions) {
            std::string name = inst->getName().empty() ? "" : ctx.fn.uniqueName(inst->getName());
            std::unique_ptr<Instruction> clone = cloneInstruction(*inst, name);
            map[inst.get()] = clone.get();
            copies.push_back(clone.get());
            copy->append(std::move(clone));
        }
    }
    for (Instruction *inst : copies) remapInstruction(*inst, map);

    //the exits' loop-closed phis: the copy's exiting blocks bring the copy's values
    for (BasicBlock *exit : loop.exits) {
        for (PhiInst *phi : phisOf(exit)) {
            std::vector<std::pair<Value *, BasicBlock *>> added;
            for (const std::pair<Value *, BasicBlock *> &in : phi->incoming) {
                if (!loop.contains(in.second)) continue;
                ValueMap::const_iterator v = map.find(in.first);
                added.push_back(std::make_pair(v == map.end() ? in.first : v->second,
                                               static_cast<BasicBlock *>(map[in.second])));
            }
            for (const std::pair<Value *, BasicBlock *> &in : added) phi->addIncoming(in.first, in.second);
        }
    }

    BasicBlock *copyHeader = static_cast<BasicBlock *>(map[loop.header]);
    BasicBlock *copyBlock = static_cast<BasicBlock *>(map[block]);
    BranchInst *copyBranch = static_cast<BranchInst *>(copyBlock->getTerminator());
    branchPreheader(pre, br->condition, loop.header, copyHeader);
    branchTo(copyBlock, copyBranch->falseTarget);
    branchTo(block, br->target);
    ctx.markDone(loop.header);
    ctx.markDone(copyHeader);
    return true;
}

} // namespace

LoopResult LoopUnswitch::runOnLoop(Loop &loop, LoopContext &ctx) {
    BasicBlock *pre = loop.preheader();
    if (pre == nullptr) return LoopResult::Unchanged;
    for (BasicBlock *block : ctx.fn.getBlocks()) {
        if (!loop.contains(block)) continue;
        Instruction *term = block->getTerminator();
        if (term == nullptr || term->getOpcode() != Opcode::Br) continue;
        BranchInst *br = static_cast<BranchInst *>(term);
        if (!br->isConditional() || br->target == br->falseTarget) continue;
        Value *cond = br->condition;
        if (cond->getKind() == ValueKind::Constant || ctx.isDefinedInLoop(cond, loop)) continue;
        bool inTrue = loop.contains(br->target);
        bool inFalse = loop.contains(br->falseTarget);
        if (inTrue != inFalse) {
            if (block == loop.header && unswitchTrivially(loop, pre, br, ctx)) return LoopResult::CFGChanged;
            continue;
        }
        if (unswitchByCopy(loop, pre, block, br, ctx)) return LoopResult::CFGChanged;
        return LoopResult::Unchanged;  //one condition per loop
    }
    return LoopResult::Unchanged;
}

} // namespace passes
