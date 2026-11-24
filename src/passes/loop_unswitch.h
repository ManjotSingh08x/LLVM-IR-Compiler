#ifndef PASSES_LOOP_UNSWITCH_H
#define PASSES_LOOP_UNSWITCH_H

#include "passes/loop_pass.h"

namespace passes {

//loop unswitching (not in the dragon book). a branch inside a loop on a loop-invariant condition
//is decided once, before the loop:
//  trivial: the branch is in the header, nothing before it has side effects, and one side leaves the
//    loop (if (flag) break;). the test moves to the preheader, the header branch becomes a plain
//    jump; no copy of the loop is needed
//  full: both sides stay in the loop. the loop (at most kMaxSize instructions) is copied, the
//    preheader branches on the condition to the original (specialized for true) or the copy
//    (specialized for false), and the loop-closed ssa phis of the exits get the copy's entries.
//    one condition per loop; the copy is not unswitched again
//the condition must be defined outside the loop (licm has moved invariant compares out) and not
//be a constant (simplify_cfg takes those)
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
