//turns a loop that tests at the top into a guarded loop that tests at the bottom
#ifndef PASSES_LOOP_ROTATE_H
#define PASSES_LOOP_ROTATE_H

#include "passes/loop_pass.h"

namespace passes {

class LoopRotate : public LoopPass {
public:
    const char *name() const override { return "loop_rotate"; }

protected:
    LoopResult runOnLoop(Loop &loop, LoopContext &ctx) override;
};

} // namespace passes

#endif
