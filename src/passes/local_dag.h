//local optimization of each basic block through its dag
#ifndef PASSES_LOCAL_DAG_H
#define PASSES_LOCAL_DAG_H

#include "pipeline/pass.h"

namespace passes {

class LocalDAG : public FunctionPass {
public:
    const char *name() const override { return "local_dag"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

bool removeUnusedValues(ir::Function &fn);

} // namespace passes

#endif
