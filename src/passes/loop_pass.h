#ifndef PASSES_LOOP_PASS_H
#define PASSES_LOOP_PASS_H

#include <set>
#include <string>
#include <vector>

#include "passes/pass.h"

namespace passes {

enum class LoopResult {
    Unchanged,
    Changed,     //instructions moved or rewritten, the cfg is the same
    CFGChanged   //blocks or edges changed, the loops are found again before the next one
};

//what a loop pass gets for the loop it is working on. the analyses are fresh for this call
struct LoopContext {
    ir::Function      &fn;
    ir::Module        &module;
    AnalysisManager   &am;
    const Dominators  &doms;
    const DefUse      &defUse;

    //true when value is an instruction placed in a block of loop
    bool isDefinedInLoop(const ir::Value *value, const Loop &loop) const;

    //true when block dominates every exit of loop, so it runs on every iteration that leaves
    bool dominatesAllExits(const ir::BasicBlock *block, const Loop &loop) const;

    //a loop whose header is block is not given to the pass again in this run, for loops the pass
    //created or rearranged itself (a rotated loop has a new header)
    void markDone(ir::BasicBlock *header) { done.insert(header); }

    std::set<ir::BasicBlock *> &done;
};

//the base of every loop optimization. run() is the same for all of them:
//  1. find the natural loops (dfa_loops) and put each in canonical form: a preheader, a single
//     latch and dedicated exits (every exit block entered only from inside the loop)
//  2. give the loops to runOnLoop innermost first, each once; after CFGChanged the loops are
//     found and canonicalized again and the walk goes on with the ones not done yet
//a subclass writes name() and runOnLoop()
class LoopPass : public FunctionPass {
public:
    bool run(ir::Function &fn, ir::Module &module, AnalysisManager &am) final;

protected:
    virtual LoopResult runOnLoop(Loop &loop, LoopContext &ctx) = 0;

    //a loop whose header is the entry block cannot get a preheader without moving the allocas
    //into the loop, such loops are never canonicalized or handed to runOnLoop
    static bool isUsable(const Loop &loop, const ir::Function &fn);

private:
    //puts every usable loop in canonical form, true when the cfg changed
    static bool canonicalize(ir::Function &fn, AnalysisManager &am);
};

//makes a new block named label that preds jump to instead of target, and that jumps to target.
//phis in target merge the entries of preds into a phi in the new block (or into the one value
//they all share). the new block is placed before placeBefore, or after the last of preds when null.
//returns the new block; the caller recomputes the cfg
ir::BasicBlock *redirectPredecessors(ir::Function &fn, ir::BasicBlock *target,
                                     const std::vector<ir::BasicBlock *> &preds, const std::string &label,
                                     ir::BasicBlock *placeBefore);

} // namespace passes

#endif
