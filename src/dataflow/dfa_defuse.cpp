#include "dataflow/dfa_defuse.h"

#include <algorithm>
#include <string>

#include "dataflow/dfa_liveness.h"

namespace passes {

DefUse::DefUse(const ir::Function &fn) : fn(fn) {
    for (ir::BasicBlock *block : fn.getBlocks()) {
        for (const std::unique_ptr<ir::Instruction> &inst : block->instructions) {
            for (ir::Value *operand : inst->getOperands()) {
                std::vector<ir::Instruction *> &list = userMap[operand];
                //add %x, %x reads %x twice but is one user
                if (list.empty() || list.back() != inst.get()) list.push_back(inst.get());
            }
        }
    }
}

const std::vector<ir::Instruction *> &DefUse::users(const ir::Value *value) const {
    static const std::vector<ir::Instruction *> none;
    std::map<const ir::Value *, std::vector<ir::Instruction *>>::const_iterator it = userMap.find(value);
    return it == userMap.end() ? none : it->second;
}

namespace {

std::string describe(const ir::Instruction *inst) {
    if (Liveness::isTracked(inst)) return inst->getOperand();
    const ir::BasicBlock *block = inst->getParent();
    std::string where = block ? " in " + block->getName() : "";
    if (inst->isTerminator()) return "<terminator" + where + ">";
    if (inst->getCategory() == ir::InstCategory::Call) return "<call" + where + ">";
    return "<store" + where + ">";
}

} // namespace

void DefUse::dump(std::ostream &os) const {
    os << "def-use @" << fn.getName() << "\n";
    std::vector<std::pair<std::string, const ir::Value *>> defs;
    for (const std::unique_ptr<ir::Argument> &arg : fn.args) defs.push_back(std::make_pair(arg->getOperand(), arg.get()));
    for (const ir::BasicBlock *block : fn.getBlocks()) {
        for (const std::unique_ptr<ir::Instruction> &inst : block->instructions) {
            if (Liveness::isTracked(inst.get())) defs.push_back(std::make_pair(inst->getOperand(), inst.get()));
        }
    }
    std::sort(defs.begin(), defs.end());
    for (const std::pair<std::string, const ir::Value *> &entry : defs) {
        os << "  " << entry.first << " ->";
        const std::vector<ir::Instruction *> &list = users(entry.second);
        if (list.empty()) os << " (unused)";
        for (const ir::Instruction *user : list) os << " " << describe(user);
        os << "\n";
    }
}

} // namespace passes
