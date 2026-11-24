#ifndef PASSES_IV_STRENGTH_REDUCE_H
#define PASSES_IV_STRENGTH_REDUCE_H

#include "passes/loop_pass.h"

namespace passes {

//induction variables and reduction in strength (dragon book 9.1.8, the affine view of 9.8).
//a basic induction variable is a header phi i = phi [init, preheader], [i + c, latch] with a
//constant step c. a value computed from it by a multiply gets its own induction variable updated
//by an addition:
//  j = i * k (or i << s)  ->  j' = phi [init * k, preheader], [j' + c*k, latch]
//and an address indexed by it becomes a pointer that steps through the array:
//  p = getelementptr T, base, i  ->  p' = phi [getelementptr T, base, init, preheader],
//                                             [getelementptr T, p', c, latch]
//(base loop-invariant; also the [N x T] form with a 0 first index). the uses of j and p take the
//new phis, the old computations die. the loop's exit test keeps using i: rewriting it in terms of
//j (induction-variable elimination) is only right when j cannot overflow
class IVStrengthReduce : public LoopPass {
public:
    const char *name() const override { return "iv_strength_reduce"; }

protected:
    LoopResult runOnLoop(Loop &loop, LoopContext &ctx) override;
};

} // namespace passes

#endif
