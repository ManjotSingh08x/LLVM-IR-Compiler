#ifndef PASSES_CLONE_H
#define PASSES_CLONE_H

#include <map>
#include <memory>
#include <string>

#include "ir/cfg.h"

namespace passes {

//old value -> its copy, for operands and for blocks
using ValueMap = std::map<const ir::Value *, ir::Value *>;

//a copy of inst with the same operands, successors and phi entries, named name
//(void instructions keep no name). the copy is not in any block yet
std::unique_ptr<ir::Instruction> cloneInstruction(const ir::Instruction &inst, const std::string &name);

//rewrites every operand, jump target and phi block of inst found in map to its copy
void remapInstruction(ir::Instruction &inst, const ValueMap &map);

} // namespace passes

#endif
