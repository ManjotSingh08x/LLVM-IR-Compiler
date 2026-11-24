#ifndef PASSES_LOOP_SIMPLIFY_H
#define PASSES_LOOP_SIMPLIFY_H

#include "passes/loop_pass.h"

namespace passes {

//puts loops into canonical form only
class LoopSimplify : public LoopPass {
public:
    const char *name() const override { return "loop_simplify"; }

protected:
    bool       needsLCSSA() const override { return true; }
    LoopResult runOnLoop(Loop &, LoopContext &) override { return LoopResult::Unchanged; }
};

}

#endif
