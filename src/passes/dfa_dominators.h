#ifndef PASSES_DFA_DOMINATORS_H
#define PASSES_DFA_DOMINATORS_H

#include <map>
#include <ostream>
#include <set>
#include <vector>

#include "ir/cfg.h"

namespace passes {

//dominators (dragon book 9.6.1): d dominates n when every path from entry to n goes through d
//found as a forward data-flow problem with intersection, OUT[B] = {B} U (meet of OUT[P] over preds),
//on the 9.3 framework. the immediate dominators, the dominator tree and the dominance frontiers
//(by the runner walk of cooper, harvey and kennedy, which the book does not cover) come from it.
//one per function, shared by every pass that needs it. only blocks entry reaches are described
class Dominators {
public:
    explicit Dominators(const ir::Function &fn);

    bool isReachable(const ir::BasicBlock *block) const { return dom.count(block) != 0; }

    //a dominates b; every reachable block dominates itself
    bool dominates(const ir::BasicBlock *a, const ir::BasicBlock *b) const;
    bool strictlyDominates(const ir::BasicBlock *a, const ir::BasicBlock *b) const {
        return a != b && dominates(a, b);
    }

    //the definition of def reaches use: def's block strictly dominates use's block, or both are in
    //one block and def comes first. a phi reads its operand at the end of the incoming block
    bool dominatesUse(const ir::Instruction *def, const ir::Instruction *user, const ir::BasicBlock *phiBlock) const;

    //null for the entry block
    ir::BasicBlock *idom(const ir::BasicBlock *block) const;

    //the blocks block immediately dominates, in layout order
    const std::vector<ir::BasicBlock *> &children(const ir::BasicBlock *block) const;

    //every reachable block, parents before children (a preorder of the dominator tree)
    const std::vector<ir::BasicBlock *> &preorder() const { return order; }

    //DF(b): the blocks where b's dominance stops, where a phi for a value defined in b goes
    const std::set<ir::BasicBlock *> &frontier(const ir::BasicBlock *block) const;

    //every block's idom and frontier, by label, for tests
    void dump(std::ostream &out) const;

private:
    const ir::Function                                                   &fn;
    std::map<const ir::BasicBlock *, std::set<const ir::BasicBlock *>>   dom;
    std::map<const ir::BasicBlock *, ir::BasicBlock *>                   immediate;
    std::map<const ir::BasicBlock *, std::vector<ir::BasicBlock *>>      kids;
    std::map<const ir::BasicBlock *, std::set<ir::BasicBlock *>>         df;
    std::vector<ir::BasicBlock *>                                        order;
};

} // namespace passes

#endif
