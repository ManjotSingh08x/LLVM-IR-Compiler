#ifndef PASSES_PASS_OPTIONS_H
#define PASSES_PASS_OPTIONS_H

#include <ostream>

namespace passes {

//debugging switches shared by the pass managers, set from the driver flags
struct PassOptions {
    bool          verifyEach = false;  //--verify-each: check the ir after every pass that changed it
    std::ostream *trace = nullptr;     //--trace-passes: each changed function after each pass
};

} // namespace passes

#endif
