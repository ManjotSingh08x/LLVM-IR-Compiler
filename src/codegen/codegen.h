#ifndef CODEGEN_CODEGEN_H
#define CODEGEN_CODEGEN_H

#include <ostream>
#include <string>

#include "ir/cfg.h"
#include "pipeline/analysis_manager.h"

namespace codegen {

//writes the module as x86-64 assembly (phis must be gone); false with the reason in error on unsupported ir
bool emitAssembly(ir::Module &module, passes::AnalysisManager &am, int optLevel, std::ostream &out,
                  std::string &error);

} // namespace codegen

#endif
