//checks the ir, then that every definition dominates its uses
#ifndef PIPELINE_VERIFY_H
#define PIPELINE_VERIFY_H

#include <string>

#include "ir/cfg.h"
#include "pipeline/analysis_manager.h"

namespace passes {

bool verifyWithDominance(const ir::Function &fn, AnalysisManager &am, std::string &error);

} // namespace passes

#endif
