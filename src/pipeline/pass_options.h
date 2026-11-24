//debugging switches for the pass managers, set from driver flags
#ifndef PIPELINE_PASS_OPTIONS_H
#define PIPELINE_PASS_OPTIONS_H

#include <ostream>

namespace passes {

struct PassOptions {
    bool          verifyEach = false;  //--verify-each: check the ir after each pass that changed it
    std::ostream *trace = nullptr;     //--trace-passes: print each changed function
};

} // namespace passes

#endif
