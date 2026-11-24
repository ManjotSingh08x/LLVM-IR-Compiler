#ifndef PASSES_LICM_H
#define PASSES_LICM_H

#include "passes/loop_pass.h"

namespace passes {

//moves loop-invariant code into the preheader
class LICM : public LoopPass {
public:
    const char *name() const override { return "licm"; }

protected:
    LoopResult runOnLoop(Loop &loop, LoopContext &ctx) override;
};

}

#endif
