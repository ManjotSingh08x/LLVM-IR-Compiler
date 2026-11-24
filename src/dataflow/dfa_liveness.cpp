#include "dataflow/dfa_liveness.h"

#include <algorithm>
#include <string>
#include <vector>

#include "dataflow/dfa_framework.h"

namespace passes {

bool Liveness::isTracked(const ir::Value *value) {
    if (value == nullptr) return false;
    if (value->getKind() == ir::ValueKind::Argument) return true;
    if (value->getKind() != ir::ValueKind::Instruction) return false;
    return !value->getType()->is(sema::TypeKind::Void);
}

Liveness::Liveness(const ir::Function &fn) : fn(fn) {
    DataflowProblem<const ir::Value *> problem;
    problem.direction = Direction::Backward;
    problem.meet = Meet::Union;

    for (ir::BasicBlock *block : fn.getBlocks()) {
        ValueSet &blockUse = use[block];
        ValueSet &blockDef = def[block];
        for (const std::unique_ptr<ir::Instruction> &inst : block->instructions) {
            if (inst->getCategory() != ir::InstCategory::Phi) {
                for (ir::Value *operand : inst->getOperands()) {
                    if (isTracked(operand) && !blockDef.count(operand)) blockUse.insert(operand);
                }
            }
            if (isTracked(inst.get())) blockDef.insert(inst.get());
        }
        problem.gen[block] = blockUse;
        problem.kill[block] = blockDef;
    }

    problem.edge = [](const ir::BasicBlock *from, const ir::BasicBlock *to, ValueSet &value) {
        for (const std::unique_ptr<ir::Instruction> &inst : to->instructions) {
            if (inst->getCategory() != ir::InstCategory::Phi) break;
            const ir::PhiInst *phi = static_cast<const ir::PhiInst *>(inst.get());
            for (const std::pair<ir::Value *, ir::BasicBlock *> &entry : phi->incoming) {
                if (entry.second == from && isTracked(entry.first)) value.insert(entry.first);
            }
        }
    };

    DataflowResult<const ir::Value *> result = solve(fn, problem);
    in = result.in;
    out = result.out;
    rounds = result.iterations;
}

const ValueSet &Liveness::find(const std::map<const ir::BasicBlock *, ValueSet> &sets, const ir::BasicBlock *block) {
    static const ValueSet empty;
    std::map<const ir::BasicBlock *, ValueSet>::const_iterator it = sets.find(block);
    return it == sets.end() ? empty : it->second;
}

void dumpValueSet(std::ostream &out, const ValueSet &values) {
    std::vector<std::string> names;
    for (const ir::Value *value : values) names.push_back(value->getOperand());
    std::sort(names.begin(), names.end());
    out << "{";
    for (size_t i = 0; i < names.size(); i++) out << (i > 0 ? ", " : "") << names[i];
    out << "}";
}

void Liveness::dump(std::ostream &os) const {
    os << "liveness @" << fn.getName() << " (" << rounds << " rounds)\n";
    for (const ir::BasicBlock *block : fn.getBlocks()) {
        os << "  " << block->getName() << ":\n";
        os << "    use = ";
        dumpValueSet(os, uses(block));
        os << "\n    def = ";
        dumpValueSet(os, defs(block));
        os << "\n    in  = ";
        dumpValueSet(os, liveIn(block));
        os << "\n    out = ";
        dumpValueSet(os, liveOut(block));
        os << "\n";
    }
}

}
