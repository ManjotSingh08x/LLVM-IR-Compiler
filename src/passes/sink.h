#ifndef PASSES_SINK_H
#define PASSES_SINK_H

#include "pipeline/pass.h"

namespace passes {

//moves pure instructions closer to their uses
class Sink : public FunctionPass {
public:
    const char *name() const override { return "sink"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

}

#endif
