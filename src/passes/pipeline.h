#ifndef PASSES_PIPELINE_H
#define PASSES_PIPELINE_H

#include "passes/global_pass_manager.h"

namespace passes {

//the stages for -O0 to -O3, the only place the levels are defined
//levels below 0 are treated as 0 and above 3 as 3
GlobalPassManager buildPipeline(int optLevel);

} // namespace passes

#endif
