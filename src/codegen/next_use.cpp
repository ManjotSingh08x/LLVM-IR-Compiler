#include "codegen/next_use.h"

namespace codegen {

namespace {

long lookup(const NextUseMap &state, const ir::Value *value) {
    NextUseMap::const_iterator it = state.find(value);
    return it == state.end() ? kDead : it->second;
}

}

NextUse::NextUse(const ir::BasicBlock &block, const passes::ValueSet &liveOut) {
    size_t count = block.instructions.size();
    table.resize(count);

    NextUseMap state;
    for (const ir::Value *value : liveOut) state[value] = kLiveOut;

    for (size_t i = count; i-- > 0;) {
        const ir::Instruction *inst = block.instructions[i].get();
        InstNextUse &info = table[i];

        if (passes::Liveness::isTracked(inst)) {
            info.result = lookup(state, inst);
            state.erase(inst);
        }
        if (inst->getOpcode() == ir::Opcode::Phi) continue;
        std::vector<ir::Value *> operands = inst->getOperands();
        for (const ir::Value *operand : operands) {
            if (passes::Liveness::isTracked(operand)) info.operands[operand] = lookup(state, operand);
        }
        for (const ir::Value *operand : operands) {
            if (passes::Liveness::isTracked(operand)) state[operand] = (long)i;
        }
    }
}

}
