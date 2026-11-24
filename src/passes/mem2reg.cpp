#include "passes/mem2reg.h"

#include <map>
#include <set>

#include "passes/escape.h"
#include "passes/simplify_cfg.h"

namespace passes {

using namespace ir;

namespace {

//x.addr -> x, so phis read like the source variable
std::string variableName(const std::string &slot) {
    const std::string suffix = ".addr";
    if (slot.size() > suffix.size() && slot.compare(slot.size() - suffix.size(), suffix.size(), suffix) == 0) {
        return slot.substr(0, slot.size() - suffix.size());
    }
    return slot;
}

class Promoter {
public:
    Promoter(Function &fn, Module &module, const Dominators &doms) : fn(fn), module(module), doms(doms) {}

    bool run() {
        EscapeInfo escape(fn);
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                if (inst->getOpcode() == Opcode::Alloca && escape.isPromotable(inst.get())) {
                    slots.insert(inst.get());
                    order.push_back(inst.get());
                }
            }
        }
        if (slots.empty()) return false;
        for (const Instruction *slot : order) placePhis(slot);

        std::map<const Instruction *, Value *> current;
        rename(fn.getEntryBlock(), current);

        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                for (Value *operand : inst->getOperands()) {
                    Value *now = resolve(operand);
                    if (now != operand) inst->replaceOperand(operand, now);
                }
            }
        }
        for (Instruction *inst : doomed) inst->getParent()->erase(inst);
        return true;
    }

private:
    const Instruction *slotOf(const Value *address) const {
        if (address->getKind() != ValueKind::Instruction) return nullptr;
        const Instruction *inst = static_cast<const Instruction *>(address);
        return slots.count(inst) ? inst : nullptr;
    }

    Value *resolve(Value *value) {
        std::map<Value *, Value *>::iterator it = replaced.find(value);
        while (it != replaced.end()) {
            value = it->second;
            it = replaced.find(value);
        }
        return value;
    }

    std::vector<BasicBlock *> inLayoutOrder(const std::set<BasicBlock *> &blocks) const {
        std::vector<BasicBlock *> ordered;
        for (BasicBlock *block : fn.getBlocks()) {
            if (blocks.count(block)) ordered.push_back(block);
        }
        return ordered;
    }

    void placePhis(const Instruction *slot) {
        std::set<BasicBlock *> defs;
        std::set<BasicBlock *> upwardLoad;  //a load comes before any store in the block
        std::set<BasicBlock *> killed;      //a store in the block
        for (BasicBlock *block : fn.getBlocks()) {
            if (!doms.isReachable(block)) continue;
            bool stored = false;
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                if (inst->getOpcode() == Opcode::Store && slotOf(static_cast<StoreInst *>(inst.get())->address) == slot) {
                    stored = true;
                } else if (inst->getOpcode() == Opcode::Load && slotOf(static_cast<LoadInst *>(inst.get())->address) == slot &&
                           !stored) {
                    upwardLoad.insert(block);
                }
            }
            if (stored) {
                defs.insert(block);
                killed.insert(block);
            }
        }
        std::set<BasicBlock *> liveIn(upwardLoad.begin(), upwardLoad.end());
        std::vector<BasicBlock *> work(upwardLoad.begin(), upwardLoad.end());
        while (!work.empty()) {
            BasicBlock *block = work.back();
            work.pop_back();
            for (BasicBlock *pred : block->predecessors) {
                if (killed.count(pred) || !doms.isReachable(pred)) continue;
                if (liveIn.insert(pred).second) work.push_back(pred);
            }
        }

        //iterated dominance frontier, walked in layout order so phis are named the same every run
        std::set<BasicBlock *> placed;
        std::vector<BasicBlock *> frontierWork = inLayoutOrder(defs);
        while (!frontierWork.empty()) {
            BasicBlock *block = frontierWork.front();
            frontierWork.erase(frontierWork.begin());
            for (BasicBlock *join : inLayoutOrder(doms.frontier(block))) {
                if (!placed.insert(join).second) continue;
                if (liveIn.count(join)) {
                    const sema::Type *type = static_cast<const AllocaInst *>(slot)->getAccessType();
                    PhiInst *phi = new PhiInst(type, fn.uniqueName(variableName(slot->getName())));
                    join->insertAtFront(std::unique_ptr<Instruction>(phi));
                    phiSlot[phi] = slot;
                }
                if (!defs.count(join)) frontierWork.push_back(join);
            }
        }
    }

    Value *valueOf(const Instruction *slot, std::map<const Instruction *, Value *> &current) {
        std::map<const Instruction *, Value *>::iterator it = current.find(slot);
        if (it != current.end()) return it->second;
        //read before any store: zero stands in for the garbage
        return module.getZero(static_cast<const AllocaInst *>(slot)->getAccessType());
    }

    //current is copied per call, so the call stack holds the value stack
    void rename(BasicBlock *block, std::map<const Instruction *, Value *> current) {
        for (std::unique_ptr<Instruction> &owned : block->instructions) {
            Instruction *inst = owned.get();
            if (inst->getOpcode() == Opcode::Phi) {
                std::map<PhiInst *, const Instruction *>::iterator it = phiSlot.find(static_cast<PhiInst *>(inst));
                if (it != phiSlot.end()) current[it->second] = inst;
            } else if (inst->getOpcode() == Opcode::Load) {
                const Instruction *slot = slotOf(static_cast<LoadInst *>(inst)->address);
                if (slot == nullptr) continue;
                replaced[inst] = valueOf(slot, current);
                doomed.push_back(inst);
            } else if (inst->getOpcode() == Opcode::Store) {
                StoreInst *store = static_cast<StoreInst *>(inst);
                const Instruction *slot = slotOf(store->address);
                if (slot == nullptr) continue;
                current[slot] = resolve(store->value);
                doomed.push_back(inst);
            }
        }
        //the successors' phis get this block's values, one entry per edge
        if (Instruction *term = block->getTerminator()) {
            for (BasicBlock *succ : term->getSuccessors()) {
                for (std::unique_ptr<Instruction> &inst : succ->instructions) {
                    if (inst->getOpcode() != Opcode::Phi) break;
                    PhiInst *phi = static_cast<PhiInst *>(inst.get());
                    std::map<PhiInst *, const Instruction *>::iterator it = phiSlot.find(phi);
                    if (it != phiSlot.end()) phi->addIncoming(resolve(valueOf(it->second, current)), block);
                }
            }
        }
        for (BasicBlock *child : doms.children(block)) rename(child, current);
    }

    Function                                 &fn;
    Module                                   &module;
    const Dominators                         &doms;
    std::set<const Instruction *>             slots;
    std::vector<Instruction *>                order;
    std::map<PhiInst *, const Instruction *>  phiSlot;
    std::map<Value *, Value *>                replaced;
    std::vector<Instruction *>                doomed;
};

} // namespace

bool Mem2Reg::run(Function &fn, Module &module, AnalysisManager &am) {
    bool changed = removeUnreachable(fn);
    if (changed) am.invalidate(fn);
    Promoter promoter(fn, module, am.getDominators(fn));
    if (promoter.run()) changed = true;
    if (changed) {
        std::vector<Instruction *> unusedSlots;
        std::map<const Value *, size_t> uses;
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                for (Value *operand : inst->getOperands()) uses[operand]++;
            }
        }
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                if (inst->getOpcode() == Opcode::Alloca && uses[inst.get()] == 0) unusedSlots.push_back(inst.get());
            }
        }
        for (Instruction *slot : unusedSlots) slot->getParent()->erase(slot);
    }
    return changed;
}

} // namespace passes
