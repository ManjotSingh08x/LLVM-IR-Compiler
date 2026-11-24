//promotes stack slots to ssa values, with phis at the dominance frontiers
#ifndef PASSES_MEM2REG_H
#define PASSES_MEM2REG_H

#include "pipeline/pass.h"

namespace passes {

class Mem2Reg : public FunctionPass {
public:
    const char *name() const override { return "mem2reg"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
