#ifndef DATAFLOW_DFA_DOMINATORS_H
#define DATAFLOW_DFA_DOMINATORS_H

#include <map>
#include <ostream>
#include <set>
#include <vector>

#include "ir/cfg.h"

namespace passes {

//computes dominators, dominator tree and dominance frontiers
class Dominators {
public:
    explicit Dominators(const ir::Function &fn);

    bool isReachable(const ir::BasicBlock *block) const { return dom.count(block) != 0; }

    bool dominates(const ir::BasicBlock *a, const ir::BasicBlock *b) const;
    bool strictlyDominates(const ir::BasicBlock *a, const ir::BasicBlock *b) const {
        return a != b && dominates(a, b);
    }

    //checks a definition dominates one of its uses
    bool dominatesUse(const ir::Instruction *def, const ir::Instruction *user, const ir::BasicBlock *phiBlock) const;

    ir::BasicBlock *idom(const ir::BasicBlock *block) const;

    const std::vector<ir::BasicBlock *> &children(const ir::BasicBlock *block) const;

    const std::vector<ir::BasicBlock *> &preorder() const { return order; }

    //blocks where this block's dominance ends
    const std::set<ir::BasicBlock *> &frontier(const ir::BasicBlock *block) const;

    void dump(std::ostream &out) const;

private:
    const ir::Function                                                   &fn;
    std::map<const ir::BasicBlock *, std::set<const ir::BasicBlock *>>   dom;
    std::map<const ir::BasicBlock *, ir::BasicBlock *>                   immediate;
    std::map<const ir::BasicBlock *, std::vector<ir::BasicBlock *>>      kids;
    std::map<const ir::BasicBlock *, std::set<ir::BasicBlock *>>         df;
    std::vector<ir::BasicBlock *>                                        order;
};

}

#endif
