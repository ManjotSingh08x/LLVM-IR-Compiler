#ifndef IR_VERIFIER_H
#define IR_VERIFIER_H

#include <string>

#include "ir/cfg.h"

namespace ir {

//checks a function is well formed after a pass, returning false with a message on the first problem
bool verifyFunction(const Function &fn, std::string &error);

} // namespace ir

#endif
