#ifndef PASSES_SIMPLIFY_CFG_H
#define PASSES_SIMPLIFY_CFG_H

#include "passes/pass.h"

namespace passes {

//flow-of-control cleanup (dragon book 8.7.2, 8.7.3), repeated until nothing changes:
//  br i1 <constant> and switch on a constant become an unconditional br, br c, L, L becomes br L
//  a block holding only "br L" is bypassed: its predecessors jump to L directly (jumps to jumps)
//  blocks entry cannot reach are deleted (unreachable code)
//  a block whose only successor has it as only predecessor absorbs that successor
//phis are not maintained yet: a function that has any is left alone (they arrive with mem2reg)
class SimplifyCFG : public FunctionPass {
public:
    const char *name() const override { return "simplify_cfg"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
