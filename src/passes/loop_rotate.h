#ifndef PASSES_LOOP_ROTATE_H
#define PASSES_LOOP_ROTATE_H

#include "passes/loop_pass.h"

namespace passes {

//turns top-tested loops into bottom-tested loops
class LoopRotate : public LoopPass {
public:
    const char *name() const override { return "loop_rotate"; }

protected:
    LoopResult runOnLoop(Loop &loop, LoopContext &ctx) override;
};

}

#endif
