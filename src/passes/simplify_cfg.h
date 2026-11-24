//control-flow cleanup: constant branches, jumps to jumps, unreachable blocks
#ifndef PASSES_SIMPLIFY_CFG_H
#define PASSES_SIMPLIFY_CFG_H

#include "pipeline/pass.h"

namespace passes {

class SimplifyCFG : public FunctionPass {
public:
    const char *name() const override { return "simplify_cfg"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

//replaces block's terminator with br target, fixing the phis of the dropped successors
void branchTo(ir::BasicBlock *block, ir::BasicBlock *target);

bool removeUnreachable(ir::Function &fn);

} // namespace passes

#endif
