#ifndef PASSES_LICM_H
#define PASSES_LICM_H

#include "passes/loop_pass.h"

namespace passes {

//loop-invariant code motion (dragon book 9.1.7; the invariance test of the 1st edition, 10.7,
//simplified by ssa). an instruction of the loop is invariant when it is pure (arithmetic, compare,
//cast, select, getelementptr; never a phi, load, store or call) and each operand is a constant, an
//argument, a global, a value defined outside the loop or another invariant instruction, found by
//repeating until nothing changes. ssa settles the 1st edition's other two conditions: there is one
//definition, and it reaches every use. invariant instructions move to the end of the preheader in
//dominator-tree order, so an operand moves before its users. sdiv and srem can trap: they move only
//when their block dominates every exit, so they would have run anyway (a rotated loop's body does,
//its guard keeps the preheader from running when the loop does not). loads stay: knowing no store
//or call in the loop writes their memory needs alias analysis
class LICM : public LoopPass {
public:
    const char *name() const override { return "licm"; }

protected:
    LoopResult runOnLoop(Loop &loop, LoopContext &ctx) override;
};

} // namespace passes

#endif
