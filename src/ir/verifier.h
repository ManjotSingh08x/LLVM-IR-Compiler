#ifndef IR_VERIFIER_H
#define IR_VERIFIER_H

#include <string>

#include "ir/cfg.h"

namespace ir {

//checks that a function is well formed after a pass changed it:
//  every placed block ends in exactly one terminator and branches only to placed blocks
//  phis come first and have one entry per incoming edge, repeats counted
//  every operand is a constant, global, function, own argument or a placed instruction of fn
//  an operand defined earlier in the same block comes before its use
//  predecessor and successor lists match the terminators
//  local names are unique
//false with a message naming the block and instruction on the first problem found
bool verifyFunction(const Function &fn, std::string &error);

} // namespace ir

#endif
