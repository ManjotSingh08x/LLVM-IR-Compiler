#include "passes/dce.h"

#include <set>

namespace passes {

using namespace ir;

bool DCE::run(Function &fn, Module &module, AnalysisManager &am) {
    (void)module;
    (void)am;
    std::set<const Instruction *> live;
    std::vector<const Instruction *> work;
    for (BasicBlock *block : fn.getBlocks()) {
        for (std::unique_ptr<Instruction> &inst : block->instructions) {
            if (inst->hasSideEffects() && live.insert(inst.get()).second) work.push_back(inst.get());
        }
    }
    while (!work.empty()) {
        const Instruction *inst = work.back();
        work.pop_back();
        for (Value *operand : inst->getOperands()) {
            if (operand->getKind() != ValueKind::Instruction) continue;
            const Instruction *def = static_cast<const Instruction *>(operand);
            if (live.insert(def).second) work.push_back(def);
        }
    }
    std::vector<Instruction *> dead;
    for (BasicBlock *block : fn.getBlocks()) {
        for (std::unique_ptr<Instruction> &inst : block->instructions) {
            if (!live.count(inst.get())) dead.push_back(inst.get());
        }
    }
    for (Instruction *inst : dead) inst->getParent()->erase(inst);
    return !dead.empty();
}

} // namespace passes
