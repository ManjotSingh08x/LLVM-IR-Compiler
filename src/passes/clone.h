//copies instructions and remaps their operands, for inlining and loop copies
#ifndef PASSES_CLONE_H
#define PASSES_CLONE_H

#include <map>
#include <memory>
#include <string>

#include "ir/cfg.h"

namespace passes {

using ValueMap = std::map<const ir::Value *, ir::Value *>;

//a copy of inst not yet in any block; void instructions get no name
std::unique_ptr<ir::Instruction> cloneInstruction(const ir::Instruction &inst, const std::string &name);

void remapInstruction(ir::Instruction &inst, const ValueMap &map);

} // namespace passes

#endif
