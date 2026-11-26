#ifndef PIPELINE_VERIFY_H
#define PIPELINE_VERIFY_H

#include <string>

#include "ir/cfg.h"
#include "pipeline/analysis_manager.h"

namespace passes {

//verifies the function including dominance of definitions
bool verifyWithDominance(const ir::Function &fn, AnalysisManager &am, std::string &error);

}

#endif
