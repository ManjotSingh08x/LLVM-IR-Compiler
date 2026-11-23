#ifndef PASSES_LOCAL_DAG_H
#define PASSES_LOCAL_DAG_H

#include "passes/pass.h"

namespace passes {

//the dag of a basic block (dragon book 8.5), built by value numbering one block at a time:
//  constant folding and algebraic identities (8.5.4): x+0, x*1, x-x, x*2^k -> x<<k, ...
//  local common subexpressions (8.5.2): an expression already computed in the block is reused,
//    + * & | ^ == != with their operands in either order
//  memory (8.5.5, 8.5.6): a load after a store to the same address is the stored value, two loads
//    of one address with nothing in between that may write it are one load, and a store
//    overwritten in the block before anything may read it is dead. a store kills what may alias
//    it; a call kills what it can reach (anything but a stack slot whose address never escapes)
//  dead code (8.5.3): a value nothing uses is removed
//works on the alloca form and on ssa alike
class LocalDAG : public FunctionPass {
public:
    const char *name() const override { return "local_dag"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

//removes instructions with no side effects whose values nothing uses, until none is left.
//true when any was removed
bool removeUnusedValues(ir::Function &fn);

} // namespace passes

#endif
