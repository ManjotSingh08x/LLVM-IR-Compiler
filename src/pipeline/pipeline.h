//builds the pass pipeline for -O0 to -O3
#ifndef PIPELINE_PIPELINE_H
#define PIPELINE_PIPELINE_H

#include "pipeline/global_pass_manager.h"

namespace passes {

GlobalPassManager buildPipeline(int optLevel);

} // namespace passes

#endif
