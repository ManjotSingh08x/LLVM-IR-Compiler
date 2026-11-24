#ifndef PASSES_DCE_H
#define PASSES_DCE_H

#include "passes/pass.h"

namespace passes {

//dead-code elimination (dragon book 9.1.6) by mark and sweep over def-use chains: everything that
//changes the program's behaviour (stores, calls, terminators) is live, then every value a live
//instruction uses is live, repeated until nothing new is marked. what is left unmarked is dead and
//removed, cycles of phis feeding each other included (an old induction variable that only its own
//update still reads), which deleting values without users one by one never finds
class DCE : public FunctionPass {
public:
    const char *name() const override { return "dce"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
