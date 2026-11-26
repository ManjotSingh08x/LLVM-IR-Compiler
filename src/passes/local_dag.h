#ifndef PASSES_LOCAL_DAG_H
#define PASSES_LOCAL_DAG_H

#include "pipeline/pass.h"

namespace passes {

//reuses repeated computations within a basic block
class LocalDAG : public FunctionPass {
public:
    const char *name() const override { return "local_dag"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

//deletes pure instructions with no users
bool removeUnusedValues(ir::Function &fn);

}

#endif
