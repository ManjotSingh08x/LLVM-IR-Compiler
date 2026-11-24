//inlines small or single-use callees, visiting the call graph bottom-up
#ifndef PASSES_INLINE_H
#define PASSES_INLINE_H

#include "pipeline/pass.h"

namespace passes {

class Inliner : public GlobalPass {
public:
    const char *name() const override { return "inline"; }
    bool        run(ir::Module &module, AnalysisManager &am) override;

    static const size_t kSmallCallee = 40;
    static const size_t kSingleCallee = 400;
    static const size_t kCallerLimit = 4000;
};

} // namespace passes

#endif
