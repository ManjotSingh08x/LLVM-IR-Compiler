#include "passes/pipeline.h"

#include "passes/const_prop_dense.h"
#include "passes/cse.h"
#include "passes/dce.h"
#include "passes/inline.h"
#include "passes/instsimplify.h"
#include "passes/iv_strength_reduce.h"
#include "passes/licm.h"
#include "passes/local_dag.h"
#include "passes/loop_rotate.h"
#include "passes/loop_simplify.h"
#include "passes/loop_unswitch.h"
#include "passes/mem2reg.h"
#include "passes/sccp.h"
#include "passes/simplify_cfg.h"
#include "passes/sink.h"

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

    //stage 4, -O2 and up: ssa, the sparse optimizations, the loop optimizations, then the
    //data-flow cleanups. sink runs last so it cannot undo licm
    std::unique_ptr<ModulePassManager> ssa(new ModulePassManager());
    ssa->add(std::unique_ptr<FunctionPass>(new Mem2Reg()));
    ssa->add(std::unique_ptr<FunctionPass>(new SCCP()));
    ssa->add(std::unique_ptr<FunctionPass>(new InstSimplify()));
    ssa->add(std::unique_ptr<FunctionPass>(new LoopSimplify()));
    ssa->add(std::unique_ptr<FunctionPass>(new LICM()));
    //iv_strength_reduce is not run here: on the linear (8.6) allocator every induction variable it
    //makes is a phi whose value goes through a stack slot each iteration, which costs more than the
    //multiply it saves (measured: +5% instructions, +10% stack accesses, 10% slower on an array
    //loop). it waits for -O3, whose global allocator can keep the new variable in a register
    ssa->add(std::unique_ptr<FunctionPass>(new LoopUnswitch()));
    ssa->add(std::unique_ptr<FunctionPass>(new SimplifyCFG()));
    ssa->add(std::unique_ptr<FunctionPass>(new InstSimplify()));
    ssa->add(std::unique_ptr<FunctionPass>(new CSE()));
    ssa->add(std::unique_ptr<FunctionPass>(new DCE()));
    ssa->add(std::unique_ptr<FunctionPass>(new Sink()));
    pipeline.addModule(std::move(ssa));

    //-O3 runs the -O2 stages, only codegen differs
    return pipeline;
}

} // namespace passes
