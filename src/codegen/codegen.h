#ifndef CODEGEN_CODEGEN_H
#define CODEGEN_CODEGEN_H

#include <ostream>
#include <string>

#include "ir/cfg.h"
#include "passes/analysis_manager.h"

namespace codegen {

//writes the module as x86-64 assembly. the ir must have no phis (phi_elim runs first)
//false when the module uses something the backend cannot lower, with the reason in error
bool emitAssembly(ir::Module &module, passes::AnalysisManager &am, int optLevel, std::ostream &out,
                  std::string &error);

} // namespace codegen

#endif
