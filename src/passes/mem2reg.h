#ifndef PASSES_MEM2REG_H
#define PASSES_MEM2REG_H

#include "pipeline/pass.h"

namespace passes {

//promotes stack slots to ssa registers
class Mem2Reg : public FunctionPass {
public:
    const char *name() const override { return "mem2reg"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

}

#endif
