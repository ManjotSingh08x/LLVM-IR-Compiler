#ifndef PASSES_SINK_H
#define PASSES_SINK_H

#include "passes/pass.h"

namespace passes {

//downward code motion (dragon book 10.4.3). a pure instruction (arithmetic, compare, cast, select,
//getelementptr; not a load, memory may change on the way) moves down to the nearest block that
//dominates all its uses (a phi uses its operand at the end of the incoming block), when that block
//is below it and not in a deeper loop. it then runs only on the paths that need it, and for this
//backend's block-local registers fewer values live across block boundaries. instructions are
//visited bottom-up so a value can follow its users down. runs last, after licm, so it never undoes
//a hoist (a hoisted value used in the loop has its uses deeper in loops and stays)
class Sink : public FunctionPass {
public:
    const char *name() const override { return "sink"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
