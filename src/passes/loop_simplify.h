#ifndef PASSES_LOOP_SIMPLIFY_H
#define PASSES_LOOP_SIMPLIFY_H

#include "passes/loop_pass.h"

namespace passes {

//puts every loop in the shape the loop passes after it rely on, on ssa: a preheader, one latch and
//dedicated exits (the LoopPass canonical form, with the phis kept right), and loop-closed ssa
//(formLCSSA). it changes nothing else
class LoopSimplify : public LoopPass {
public:
    const char *name() const override { return "loop_simplify"; }

protected:
    bool       needsLCSSA() const override { return true; }
    LoopResult runOnLoop(Loop &, LoopContext &) override { return LoopResult::Unchanged; }
};

} // namespace passes

#endif
