#include "pipeline/verify.h"

#include "ir/verifier.h"

namespace passes {

bool verifyWithDominance(const ir::Function &fn, AnalysisManager &am, std::string &error) {
    if (!ir::verifyFunction(fn, error)) return false;
    const Dominators &doms = am.getDominators(fn);
    for (const ir::BasicBlock *block : fn.getBlocks()) {
        if (!doms.isReachable(block)) continue;
        for (const std::unique_ptr<ir::Instruction> &inst : block->instructions) {
            const bool isPhi = inst->getOpcode() == ir::Opcode::Phi;
            std::vector<std::pair<ir::Value *, const ir::BasicBlock *>> uses;
            if (isPhi) {
                for (const std::pair<ir::Value *, ir::BasicBlock *> &in :
                     static_cast<const ir::PhiInst *>(inst.get())->incoming) {
                    uses.push_back(std::make_pair(in.first, in.second));
                }
            } else {
                for (ir::Value *operand : inst->getOperands()) uses.push_back(std::make_pair(operand, nullptr));
            }
            for (const std::pair<ir::Value *, const ir::BasicBlock *> &use : uses) {
                if (use.first->getKind() != ir::ValueKind::Instruction) continue;
                const ir::Instruction *def = static_cast<const ir::Instruction *>(use.first);
                if (isPhi && !doms.isReachable(use.second)) continue;
                if (!doms.dominatesUse(def, inst.get(), use.second)) {
                    error = "in block %" + block->getName() + ": %" + def->getName() +
                            " (defined in %" + def->getParent()->getName() + ") does not dominate a use in '" +
                            (inst->getName().empty() ? std::string("a ") + (inst->isTerminator() ? "terminator" : "store or call")
                                                     : "%" + inst->getName()) + "'";
                    return false;
                }
            }
        }
    }
    return true;
}

}
