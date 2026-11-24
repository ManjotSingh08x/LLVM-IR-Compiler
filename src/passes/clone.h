#ifndef PASSES_CLONE_H
#define PASSES_CLONE_H

#include <map>
#include <memory>
#include <string>

#include "ir/cfg.h"

namespace passes {

using ValueMap = std::map<const ir::Value *, ir::Value *>;

//copies an instruction under a new name
std::unique_ptr<ir::Instruction> cloneInstruction(const ir::Instruction &inst, const std::string &name);

//rewrites operands using the value map
void remapInstruction(ir::Instruction &inst, const ValueMap &map);

}

#endif
