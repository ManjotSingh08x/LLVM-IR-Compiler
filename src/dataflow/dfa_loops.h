//natural loops of a function and how they nest
#ifndef DATAFLOW_DFA_LOOPS_H
#define DATAFLOW_DFA_LOOPS_H

#include <memory>
#include <ostream>
#include <set>
#include <vector>

#include "ir/cfg.h"
#include "dataflow/dfa_dominators.h"

namespace passes {

struct Loop {
    ir::BasicBlock               *header = nullptr;
    std::set<ir::BasicBlock *>    blocks;    //header included
    std::vector<ir::BasicBlock *> latches;
    std::vector<ir::BasicBlock *> exits;     //blocks outside with a predecessor inside
    Loop                         *parent = nullptr;
    std::vector<Loop *>           children;
    unsigned                      depth = 1;  //1 for an outermost loop

    bool contains(const ir::BasicBlock *block) const {
        return blocks.count(const_cast<ir::BasicBlock *>(block)) != 0;
    }

    std::vector<ir::BasicBlock *> outsidePredecessors() const;

    //the one outside predecessor that only jumps to the header, or null
    ir::BasicBlock *preheader() const;
};

class LoopInfo {
public:
    LoopInfo(const ir::Function &fn, const Dominators &doms);

    const std::vector<Loop *> &innermostFirst() const { return postorder; }

    //the innermost loop containing block, or null
    Loop *loopFor(const ir::BasicBlock *block) const;

    unsigned depth(const ir::BasicBlock *block) const;

    void dump(std::ostream &out) const;

private:
    const ir::Function                &fn;
    std::vector<std::unique_ptr<Loop>> loops;
    std::vector<Loop *>                postorder;
};

} // namespace passes

#endif
