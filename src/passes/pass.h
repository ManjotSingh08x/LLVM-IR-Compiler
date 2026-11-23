#ifndef PASSES_PASS_H
#define PASSES_PASS_H

#include "ir/cfg.h"
#include "passes/analysis_manager.h"

namespace passes {

//one transformation of a function, chained with others by a PassManager
class FunctionPass {
public:
    virtual ~FunctionPass() = default;

    virtual const char *name() const = 0;

    //changes fn, returns true when anything changed so cached analyses are dropped
    virtual bool run(ir::Function &fn, ir::Module &module, AnalysisManager &am) = 0;
};

} // namespace passes

#endif
