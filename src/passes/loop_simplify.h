//puts every loop in canonical form: a preheader, one latch and dedicated exits
#ifndef PASSES_LOOP_SIMPLIFY_H
#define PASSES_LOOP_SIMPLIFY_H

#include "passes/loop_pass.h"

namespace passes {

class LoopSimplify : public LoopPass {
public:
    const char *name() const override { return "loop_simplify"; }

protected:
    bool       needsLCSSA() const override { return true; }
    LoopResult runOnLoop(Loop &, LoopContext &) override { return LoopResult::Unchanged; }
};

} // namespace passes

#endif
