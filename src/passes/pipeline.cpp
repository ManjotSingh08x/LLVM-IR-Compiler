#include "passes/pipeline.h"

#include "passes/const_prop_dense.h"
#include "passes/inline.h"
#include "passes/local_dag.h"
#include "passes/loop_rotate.h"
#include "passes/simplify_cfg.h"

namespace passes {

GlobalPassManager buildPipeline(int optLevel) {
    if (optLevel < 0) optLevel = 0;
    if (optLevel > 3) optLevel = 3;

    GlobalPassManager pipeline;

    //stage 1, -O0 and up: clean the cfg irgen made, then put the test of each loop at the bottom
    std::unique_ptr<ModulePassManager> cleanup(new ModulePassManager());
    cleanup->add(std::unique_ptr<FunctionPass>(new SimplifyCFG()));
    cleanup->add(std::unique_ptr<FunctionPass>(new LoopRotate()));
    pipeline.addModule(std::move(cleanup));
    if (optLevel == 0) return pipeline;

    //stage 2, -O1 and up: inlining sees every function after stage 1, callees first
    pipeline.addGlobal(std::unique_ptr<GlobalPass>(new Inliner()));

    //stage 3, -O1 and up: the local and dense optimizations of the alloca form. the second
    //local_dag folds what constant propagation substituted, simplify_cfg the branches it decided
    std::unique_ptr<ModulePassManager> scalar(new ModulePassManager());
    scalar->add(std::unique_ptr<FunctionPass>(new LocalDAG()));
    scalar->add(std::unique_ptr<FunctionPass>(new ConstPropDense()));
    scalar->add(std::unique_ptr<FunctionPass>(new LocalDAG()));
    scalar->add(std::unique_ptr<FunctionPass>(new SimplifyCFG()));
    pipeline.addModule(std::move(scalar));
    if (optLevel == 1) return pipeline;

    //stage 4, -O2 and up: mem2reg, sccp, instsimplify, loop_simplify, licm, iv_strength_reduce,
    //loop_unswitch, simplify_cfg, instsimplify, cse, dce, sink. not written yet
    pipeline.addModule(std::unique_ptr<ModulePassManager>(new ModulePassManager()));

    //-O3 runs the -O2 stages, only codegen differs
    return pipeline;
}

} // namespace passes
