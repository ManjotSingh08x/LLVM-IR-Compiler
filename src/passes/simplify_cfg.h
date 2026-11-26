#ifndef PASSES_SIMPLIFY_CFG_H
#define PASSES_SIMPLIFY_CFG_H

#include "pipeline/pass.h"

namespace passes {

//merges and removes redundant blocks and branches
class SimplifyCFG : public FunctionPass {
public:
    const char *name() const override { return "simplify_cfg"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

void branchTo(ir::BasicBlock *block, ir::BasicBlock *target);

//deletes blocks that cannot be reached
bool removeUnreachable(ir::Function &fn);

}

#endif
