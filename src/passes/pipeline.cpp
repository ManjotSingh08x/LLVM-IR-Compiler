#include "passes/pipeline.h"

namespace passes {

GlobalPassManager buildPipeline(int optLevel) {
    if (optLevel < 0) optLevel = 0;
    if (optLevel > 3) optLevel = 3;

    GlobalPassManager pipeline;

    //stage 1, -O0 and up: simplify_cfg, loop_rotate. not written yet
    pipeline.addModule(std::unique_ptr<ModulePassManager>(new ModulePassManager()));
    if (optLevel == 0) return pipeline;

    //stage 2, -O1 and up: inline (global). not written yet
    //stage 3, -O1 and up: local_dag, const_prop_dense, local_dag, simplify_cfg. not written yet
    pipeline.addModule(std::unique_ptr<ModulePassManager>(new ModulePassManager()));
    if (optLevel == 1) return pipeline;

    //stage 4, -O2 and up: mem2reg, sccp, instsimplify, loop_simplify, licm, iv_strength_reduce,
    //loop_unswitch, simplify_cfg, instsimplify, cse, dce, sink. not written yet
    pipeline.addModule(std::unique_ptr<ModulePassManager>(new ModulePassManager()));

    //-O3 runs the -O2 stages, only codegen differs
    return pipeline;
}

} // namespace passes
