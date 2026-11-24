#ifndef PIPELINE_PASS_H
#define PIPELINE_PASS_H

#include "ir/cfg.h"
#include "pipeline/analysis_manager.h"

namespace passes {

class FunctionPass {
public:
    virtual ~FunctionPass() = default;

    virtual const char *name() const = 0;

    virtual bool run(ir::Function &fn, ir::Module &module, AnalysisManager &am) = 0;
};

class GlobalPass {
public:
    virtual ~GlobalPass() = default;

    virtual const char *name() const = 0;

    virtual bool run(ir::Module &module, AnalysisManager &am) = 0;
};

}

#endif
