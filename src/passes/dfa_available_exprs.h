#ifndef PASSES_DFA_AVAILABLE_EXPRS_H
#define PASSES_DFA_AVAILABLE_EXPRS_H

#include <map>
#include <set>
#include <string>

#include "ir/cfg.h"

namespace passes {

//available expressions (dragon book 9.2.6): an expression is available at a point when every path
//from entry computes it and nothing changes its operands afterwards. a forward problem with
//intersection on the 9.3 framework, over the pure expressions of the function (expressionKey):
//  gen[B]  = the expressions B computes
//  kill[B] = nothing: on ssa an operand is defined once and never changes
//  OUT[entry] = {}, every other OUT starts as all expressions
class AvailableExpressions {
public:
    explicit AvailableExpressions(const ir::Function &fn);

    const std::set<std::string> &in(const ir::BasicBlock *block) const;
    const std::set<std::string> &out(const ir::BasicBlock *block) const;

private:
    std::map<const ir::BasicBlock *, std::set<std::string>> ins;
    std::map<const ir::BasicBlock *, std::set<std::string>> outs;
};

//the instructions available expressions describe: arithmetic, compares, casts, selects, addresses
bool isExpression(const ir::Instruction &inst);

} // namespace passes

#endif
