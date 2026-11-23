#ifndef PASSES_INSTSIMPLIFY_H
#define PASSES_INSTSIMPLIFY_H

#include "passes/pass.h"

namespace passes {

//algebraic simplification on ssa (dragon book 8.5.4, 8.7.4), one instruction at a time over the
//whole function in reverse postorder, repeated until nothing changes:
//  folding of instructions whose operands are all constants
//  the identities of simplifyInstruction (x+0, x*1, x-x, x*2^k -> x<<k, -(-x), ...)
//  a phi whose entries are all one value (or the phi itself) is that value
//  reassociation of constants (8.5.4, "associative laws"): (x op c1) op c2 -> x op (c1 op c2) for
//    integer + * & | ^, which wrap and so are associative, and (x << a) << b -> x << (a + b);
//    never for floating point
class InstSimplify : public FunctionPass {
public:
    const char *name() const override { return "instsimplify"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
