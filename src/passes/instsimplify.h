#ifndef PASSES_INSTSIMPLIFY_H
#define PASSES_INSTSIMPLIFY_H

#include "pipeline/pass.h"

namespace passes {

//rewrites instructions into simpler equivalent forms
class InstSimplify : public FunctionPass {
public:
    const char *name() const override { return "instsimplify"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

}

#endif
