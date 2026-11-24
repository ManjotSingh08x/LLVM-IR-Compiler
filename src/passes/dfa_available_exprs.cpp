#include "passes/dfa_available_exprs.h"

#include "passes/dfa_framework.h"
#include "passes/fold.h"

namespace passes {

using namespace ir;

bool isExpression(const Instruction &inst) {
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

AvailableExpressions::AvailableExpressions(const Function &fn) {
    DataflowProblem<std::string> problem;
    problem.direction = Direction::Forward;
    problem.meet = Meet::Intersection;
    for (const BasicBlock *block : fn.getBlocks()) {
        for (const std::unique_ptr<Instruction> &inst : block->instructions) {
            if (!isExpression(*inst)) continue;
            std::string key = expressionKey(*inst);
            problem.gen[block].insert(key);
            problem.top.insert(key);
        }
    }
    DataflowResult<std::string> result = solve(fn, problem);
    for (const auto &entry : result.in) ins[entry.first] = entry.second;
    for (const auto &entry : result.out) outs[entry.first] = entry.second;
}

const std::set<std::string> &AvailableExpressions::in(const BasicBlock *block) const {
    static const std::set<std::string> none;
    std::map<const BasicBlock *, std::set<std::string>>::const_iterator it = ins.find(block);
    return it == ins.end() ? none : it->second;
}

const std::set<std::string> &AvailableExpressions::out(const BasicBlock *block) const {
    static const std::set<std::string> none;
    std::map<const BasicBlock *, std::set<std::string>>::const_iterator it = outs.find(block);
    return it == outs.end() ? none : it->second;
}

} // namespace passes
