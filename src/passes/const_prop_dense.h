#ifndef PASSES_CONST_PROP_DENSE_H
#define PASSES_CONST_PROP_DENSE_H

#include "passes/pass.h"

namespace passes {

//constant propagation (dragon book 9.4) on the alloca form, before ssa. the variables are the
//integer and floating stack slots whose address never escapes (EscapeInfo::isPromotable).
//each block's IN and OUT map every variable to UNDEF, a constant or NAC (not a constant):
//  meet (9.4.2): UNDEF ^ v = v, c ^ c = c, c ^ d = NAC, NAC ^ v = NAC
//  transfer (9.4.3): a store sets its variable to the value stored, worked out from the block's
//    loads, constants and foldable arithmetic; anything else stored is NAC
//  boundary: every variable is UNDEF on entry; parameters become NAC when their incoming value is stored
//solved by round-robin iteration in reverse postorder. then every load of a variable that is a
//constant at that point is replaced by the constant; local_dag and simplify_cfg fold what follows
class ConstPropDense : public FunctionPass {
public:
    const char *name() const override { return "const_prop_dense"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
