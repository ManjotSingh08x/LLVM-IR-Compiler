#ifndef PASSES_PIPELINE_H
#define PASSES_PIPELINE_H

#include "passes/pass_manager.h"

namespace passes {

//the passes for -O0, -O1 or -O2, the only place the levels are defined
//levels below 0 are treated as 0 and above 2 as 2
PassManager buildPipeline(int optLevel);

} // namespace passes

#endif
