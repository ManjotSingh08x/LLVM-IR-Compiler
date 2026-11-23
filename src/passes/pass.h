#ifndef PASSES_PASS_H
#define PASSES_PASS_H

#include "ir/cfg.h"
#include "passes/analysis_manager.h"

namespace passes {

//one transformation of a single function, run over every function by a ModulePassManager
class FunctionPass {
public:
    virtual ~FunctionPass() = default;

    virtual const char *name() const = 0;

    //changes fn, returns true when anything changed so cached analyses are dropped
    virtual bool run(ir::Function &fn, ir::Module &module, AnalysisManager &am) = 0;
};

//one transformation that sees every function at once, such as inlining. run by the
//GlobalPassManager between module stages, with the call graph from the AnalysisManager
class GlobalPass {
public:
    virtual ~GlobalPass() = default;

    virtual const char *name() const = 0;

    //changes the module, returns true when any function changed
    virtual bool run(ir::Module &module, AnalysisManager &am) = 0;
};

} // namespace passes

#endif
