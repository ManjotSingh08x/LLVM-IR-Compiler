#ifndef PASSES_MEM2REG_H
#define PASSES_MEM2REG_H

#include "passes/pass.h"

namespace passes {

//builds ssa (cytron et al.; the dragon book only names ssa, 6.2.4). every promotable stack slot
//(EscapeInfo::isPromotable: a scalar used only by whole loads and stores) becomes ssa values:
//  1. phi placement: a phi for slot x goes at every block of the iterated dominance frontier of
//     the blocks that store x, pruned to blocks where x is live on entry (a load of x can be
//     reached from the block's top without passing a store), so no dead phis are made
//  2. renaming: a walk of the dominator tree with the current value of each slot. a store sets
//     it, a load is replaced by it, a phi defines it, and each successor's phis get it as the
//     entry for this block, once per edge. a slot read before any store reads zero
//  3. the slots, their loads and their stores are deleted
//unreachable blocks are deleted first, they have no place in the dominator tree
class Mem2Reg : public FunctionPass {
public:
    const char *name() const override { return "mem2reg"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
