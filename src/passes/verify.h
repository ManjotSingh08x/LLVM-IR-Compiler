#ifndef PASSES_VERIFY_H
#define PASSES_VERIFY_H

#include <string>

#include "ir/cfg.h"
#include "passes/analysis_manager.h"

namespace passes {

//everything ir::verifyFunction checks, then that every definition dominates each of its uses
//(a phi operand at the end of its incoming block), using the cached dominators of fn
bool verifyWithDominance(const ir::Function &fn, AnalysisManager &am, std::string &error);

} // namespace passes

#endif
