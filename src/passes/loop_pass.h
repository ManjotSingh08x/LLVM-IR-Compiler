#ifndef PASSES_LOOP_PASS_H
#define PASSES_LOOP_PASS_H

#include <set>
#include <string>
#include <vector>

#include "pipeline/pass.h"

namespace passes {

enum class LoopResult {
    Unchanged,
    Changed,
    CFGChanged
};

struct LoopContext {
    ir::Function      &fn;
    ir::Module        &module;
    AnalysisManager   &am;
    const Dominators  &doms;
    const DefUse      &defUse;

    bool isDefinedInLoop(const ir::Value *value, const Loop &loop) const;

    bool dominatesAllExits(const ir::BasicBlock *block, const Loop &loop) const;

    void markDone(ir::BasicBlock *header) { done.insert(header); }

    std::set<ir::BasicBlock *> &done;
};

//base for passes that transform one loop
class LoopPass : public FunctionPass {
public:
    bool run(ir::Function &fn, ir::Module &module, AnalysisManager &am) final;

protected:
    virtual LoopResult runOnLoop(Loop &loop, LoopContext &ctx) = 0;

    virtual bool needsLCSSA() const { return false; }

    static bool isUsable(const Loop &loop, const ir::Function &fn);

private:
    //gives loops a preheader and single latch
    static bool canonicalize(ir::Function &fn, AnalysisManager &am);

    //puts loops into loop-closed form
    static bool closeLoops(ir::Function &fn, AnalysisManager &am);
};

//routes outside uses of loop values through phis
bool formLCSSA(ir::Function &fn, const Loop &loop, const Dominators &doms, bool &changed);

bool isLCSSA(const Loop &loop, const DefUse &defUse);

//sends the given predecessors to a new block
ir::BasicBlock *redirectPredecessors(ir::Function &fn, ir::BasicBlock *target,
                                     const std::vector<ir::BasicBlock *> &preds, const std::string &label,
                                     ir::BasicBlock *placeBefore);

}

#endif
