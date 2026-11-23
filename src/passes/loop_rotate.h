#ifndef PASSES_LOOP_ROTATE_H
#define PASSES_LOOP_ROTATE_H

#include "passes/loop_pass.h"

namespace passes {

//loop rotation (not in the dragon book): a loop that tests at the top becomes a guarded loop that
//tests at the bottom
//  pre: br head                         pre:  <copy of head> br c', body, exit   (the guard)
//  head: c = ... br c, body, exit   ->   body: ...
//  body: ... br head                     latch: ... br head
//                                        head: c = ... br c, body, exit          (now at the bottom)
//one branch per iteration instead of two, and the body now dominates the test, so later loop
//passes can hoist into a preheader that only runs when the loop does. runs on the alloca form:
//the header is copied with fresh names and nothing else needs repair. a loop is rotated only when
//  the header is one block ending in a conditional branch with one target in the loop and one out
//  the header has no calls, stores or phis and is small, and its values are used only inside it
//  it is not already rotated (the header is not its own latch)
//so a condition with && or || (several blocks) or with a call is left alone
class LoopRotate : public LoopPass {
public:
    const char *name() const override { return "loop_rotate"; }

protected:
    LoopResult runOnLoop(Loop &loop, LoopContext &ctx) override;
};

} // namespace passes

#endif
