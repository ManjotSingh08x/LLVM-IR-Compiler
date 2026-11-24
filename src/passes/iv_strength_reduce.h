//replaces multiplies of induction variables with additions
#ifndef PASSES_IV_STRENGTH_REDUCE_H
#define PASSES_IV_STRENGTH_REDUCE_H

#include "passes/loop_pass.h"

namespace passes {

class IVStrengthReduce : public LoopPass {
public:
    const char *name() const override { return "iv_strength_reduce"; }

protected:
    LoopResult runOnLoop(Loop &loop, LoopContext &ctx) override;
};

} // namespace passes

#endif
