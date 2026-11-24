//available expressions: what is already computed on every path to a point
#ifndef DATAFLOW_DFA_AVAILABLE_EXPRS_H
#define DATAFLOW_DFA_AVAILABLE_EXPRS_H

#include <map>
#include <set>
#include <string>

#include "ir/cfg.h"

namespace passes {

class AvailableExpressions {
public:
    explicit AvailableExpressions(const ir::Function &fn);

    const std::set<std::string> &in(const ir::BasicBlock *block) const;
    const std::set<std::string> &out(const ir::BasicBlock *block) const;

private:
    std::map<const ir::BasicBlock *, std::set<std::string>> ins;
    std::map<const ir::BasicBlock *, std::set<std::string>> outs;
};

//pure instructions the analysis tracks: arithmetic, compare, cast, select, getelementptr
bool isExpression(const ir::Instruction &inst);

} // namespace passes

#endif
