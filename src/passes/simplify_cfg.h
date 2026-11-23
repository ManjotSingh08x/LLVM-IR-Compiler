#ifndef PASSES_SIMPLIFY_CFG_H
#define PASSES_SIMPLIFY_CFG_H

#include "passes/pass.h"

namespace passes {

//flow-of-control cleanup (dragon book 8.7.2, 8.7.3), repeated until nothing changes:
//  br i1 <constant> and switch on a constant become an unconditional br, br c, L, L becomes br L
//  a block holding only "br L" is bypassed: its predecessors jump to L directly (jumps to jumps)
//  blocks entry cannot reach are deleted (unreachable code)
//  a block whose only successor has it as only predecessor absorbs that successor
//on ssa every edge change keeps the phis right: one entry per incoming edge
class SimplifyCFG : public FunctionPass {
public:
    const char *name() const override { return "simplify_cfg"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

//replaces block's terminator with "br target"; every other outgoing edge (repeats counted) loses
//its entry in the phis of the block it led to
void branchTo(ir::BasicBlock *block, ir::BasicBlock *target);

//deletes the blocks entry cannot reach (and the phi entries they fed), recomputing the cfg.
//true when any was deleted
bool removeUnreachable(ir::Function &fn);

} // namespace passes

#endif
