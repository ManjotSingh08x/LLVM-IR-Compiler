#include "passes/pipeline.h"

namespace passes {

PassManager buildPipeline(int optLevel) {
    if (optLevel < 0) optLevel = 0;
    if (optLevel > 2) optLevel = 2;

    PassManager pm;
    //-O0: the ir as irgen made it, codegen only asks for liveness
    if (optLevel == 0) return pm;

    //-O1: mem2reg, const_prop, then phi_elim. not written yet
    if (optLevel == 1) return pm;

    //-O2: mem2reg, const_prop, simplify_cfg, cse, dce, then phi_elim. not written yet
    return pm;
}

} // namespace passes
