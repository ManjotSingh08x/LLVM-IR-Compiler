#ifndef PASSES_DFA_LOOPS_H
#define PASSES_DFA_LOOPS_H

#include <memory>
#include <ostream>
#include <set>
#include <vector>

#include "ir/cfg.h"
#include "passes/dfa_dominators.h"

namespace passes {

//a natural loop (dragon book 9.6.6): a header plus every block that reaches one of its back
//edges t -> header (header dominates t) without passing through the header
struct Loop {
    ir::BasicBlock               *header = nullptr;
    std::set<ir::BasicBlock *>    blocks;    //header included
    std::vector<ir::BasicBlock *> latches;   //sources of the back edges, layout order
    std::vector<ir::BasicBlock *> exits;     //blocks outside with a predecessor inside, layout order
    Loop                         *parent = nullptr;
    std::vector<Loop *>           children;
    unsigned                      depth = 1;  //1 for an outermost loop

    bool contains(const ir::BasicBlock *block) const {
        return blocks.count(const_cast<ir::BasicBlock *>(block)) != 0;
    }

    //the predecessors of the header outside the loop, layout order
    std::vector<ir::BasicBlock *> outsidePredecessors() const;

    //the one outside predecessor whose only successor is the header, null when there is none
    ir::BasicBlock *preheader() const;
};

//every natural loop of a function and how they nest. loops with one header are merged into one.
//a cycle without a back edge (an irreducible goto loop, 9.6.4) is not a natural loop and is left out
class LoopInfo {
public:
    LoopInfo(const ir::Function &fn, const Dominators &doms);

    //innermost loops first, outer loops after everything they contain (postorder of the loop tree)
    const std::vector<Loop *> &innermostFirst() const { return postorder; }

    //the innermost loop containing block, null when block is in no loop
    Loop *loopFor(const ir::BasicBlock *block) const;

    //0 outside every loop
    unsigned depth(const ir::BasicBlock *block) const;

    //header, depth, blocks, latches and exits of every loop, for tests
    void dump(std::ostream &out) const;

private:
    const ir::Function                &fn;
    std::vector<std::unique_ptr<Loop>> loops;
    std::vector<Loop *>                postorder;
};

} // namespace passes

#endif
