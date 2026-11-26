#ifndef IR_VERIFIER_H
#define IR_VERIFIER_H

#include <string>

#include "ir/cfg.h"

namespace ir {

//checks that the function's IR is well formed
bool verifyFunction(const Function &fn, std::string &error);

}

#endif
