//base class for loop passes and the loop forms they rely on
#ifndef PASSES_LOOP_PASS_H
#define PASSES_LOOP_PASS_H

#include <set>
#include <string>
#include <vector>

#include "pipeline/pass.h"

namespace passes {

enum class LoopResult {
    Unchanged,
    Changed,     //instructions moved or rewritten, same cfg
    CFGChanged   //blocks or edges changed, loops are found again
};

struct LoopContext {
    ir::Function      &fn;
    ir::Module        &module;
    AnalysisManager   &am;
    const Dominators  &doms;
    const DefUse      &defUse;

    bool isDefinedInLoop(const ir::Value *value, const Loop &loop) const;

    bool dominatesAllExits(const ir::BasicBlock *block, const Loop &loop) const;

    //the loop with this header is not given to the pass again in this run
    void markDone(ir::BasicBlock *header) { done.insert(header); }

    std::set<ir::BasicBlock *> &done;
};

//canonicalizes every loop, then runs runOnLoop on each, innermost first
class LoopPass : public FunctionPass {
public:
    bool run(ir::Function &fn, ir::Module &module, AnalysisManager &am) final;

protected:
    virtual LoopResult runOnLoop(Loop &loop, LoopContext &ctx) = 0;

    virtual bool needsLCSSA() const { return false; }

    //a loop headed by the entry block cannot get a preheader, so it is skipped
    static bool isUsable(const Loop &loop, const ir::Function &fn);

private:
    static bool canonicalize(ir::Function &fn, AnalysisManager &am);

    //forms lcssa for every usable loop
    static bool closeLoops(ir::Function &fn, AnalysisManager &am);
};

//routes each outside use of a loop value through an exit-block phi; false when no single exit dominates a use
bool formLCSSA(ir::Function &fn, const Loop &loop, const Dominators &doms, bool &changed);

bool isLCSSA(const Loop &loop, const DefUse &defUse);

//inserts a block named label between preds and target, merging target's phis into it
ir::BasicBlock *redirectPredecessors(ir::Function &fn, ir::BasicBlock *target,
                                     const std::vector<ir::BasicBlock *> &preds, const std::string &label,
                                     ir::BasicBlock *placeBefore);

} // namespace passes

#endif
