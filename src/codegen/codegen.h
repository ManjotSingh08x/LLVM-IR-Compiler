#ifndef CODEGEN_CODEGEN_H
#define CODEGEN_CODEGEN_H

#include <ostream>
#include <string>

#include "ir/cfg.h"
#include "pipeline/analysis_manager.h"

namespace codegen {

bool emitAssembly(ir::Module &module, passes::AnalysisManager &am, int optLevel, std::ostream &out,
                  std::string &error);

}

#endif
