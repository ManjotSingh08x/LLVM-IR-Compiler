//global common subexpression elimination over the dominator tree
#ifndef PASSES_CSE_H
#define PASSES_CSE_H

#include "pipeline/pass.h"

namespace passes {

class CSE : public FunctionPass {
public:
    const char *name() const override { return "cse"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
