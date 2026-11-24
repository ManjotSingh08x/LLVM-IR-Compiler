//moves a branch on a loop-invariant condition out of the loop
#ifndef PASSES_LOOP_UNSWITCH_H
#define PASSES_LOOP_UNSWITCH_H

#include "passes/loop_pass.h"

namespace passes {

class LoopUnswitch : public LoopPass {
public:
    const char *name() const override { return "loop_unswitch"; }

    static const size_t kMaxSize = 60;

protected:
    bool       needsLCSSA() const override { return true; }
    LoopResult runOnLoop(Loop &loop, LoopContext &ctx) override;
};

} // namespace passes

#endif
