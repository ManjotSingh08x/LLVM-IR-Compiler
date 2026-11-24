//base classes for function passes and global passes
#ifndef PIPELINE_PASS_H
#define PIPELINE_PASS_H

#include "ir/cfg.h"
#include "pipeline/analysis_manager.h"

namespace passes {

class FunctionPass {
public:
    virtual ~FunctionPass() = default;

    virtual const char *name() const = 0;

    //returns true when fn changed, so cached analyses are dropped
    virtual bool run(ir::Function &fn, ir::Module &module, AnalysisManager &am) = 0;
};

//a pass that sees every function at once, such as inlining
class GlobalPass {
public:
    virtual ~GlobalPass() = default;

    virtual const char *name() const = 0;

    virtual bool run(ir::Module &module, AnalysisManager &am) = 0;
};

} // namespace passes

#endif
