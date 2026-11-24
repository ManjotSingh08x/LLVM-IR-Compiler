#ifndef PASSES_CSE_H
#define PASSES_CSE_H

#include "passes/pass.h"

namespace passes {

//global common subexpression elimination (dragon book 9.1.4, with available expressions, 9.2.6).
//when an expression is available where it is computed again, the earlier result can be reused.
//on ssa the reuse needs one earlier instruction that dominates the later one: the dominator tree is
//walked with a table of the expressions computed in the blocks above, and an expression found there
//(and available on entry, or computed earlier in the block) replaces the later copy. an expression
//available only because each path computes it separately would need a phi, which is partial
//redundancy elimination (9.5), not done here
class CSE : public FunctionPass {
public:
    const char *name() const override { return "cse"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
