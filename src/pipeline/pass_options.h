#ifndef PIPELINE_PASS_OPTIONS_H
#define PIPELINE_PASS_OPTIONS_H

#include <ostream>

namespace passes {

struct PassOptions {
    bool          verifyEach = false;
    std::ostream *trace = nullptr;
};

}

#endif
