#ifndef PIPELINE_PIPELINE_H
#define PIPELINE_PIPELINE_H

#include "pipeline/global_pass_manager.h"

namespace passes {

//builds the pass sequence for an optimization level
GlobalPassManager buildPipeline(int optLevel);

}

#endif
