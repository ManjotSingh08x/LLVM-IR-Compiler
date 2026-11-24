#ifndef PASSES_DCE_H
#define PASSES_DCE_H

#include "pipeline/pass.h"

namespace passes {

//removes instructions whose results are never used
class DCE : public FunctionPass {
public:
    const char *name() const override { return "dce"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

}

#endif
