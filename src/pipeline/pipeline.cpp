#include "pipeline/pipeline.h"

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
#include "passes/simplify_cfg.h"
#include "passes/sink.h"

namespace passes {

GlobalPassManager buildPipeline(int optLevel) {
    if (optLevel < 0) optLevel = 0;
    if (optLevel > 3) optLevel = 3;

    GlobalPassManager pipeline;

    std::unique_ptr<ModulePassManager> cleanup(new ModulePassManager());
    cleanup->add(std::unique_ptr<FunctionPass>(new SimplifyCFG()));
    cleanup->add(std::unique_ptr<FunctionPass>(new LoopRotate()));
    pipeline.addModule(std::move(cleanup));
    if (optLevel == 0) return pipeline;

    pipeline.addGlobal(std::unique_ptr<GlobalPass>(new Inliner()));

    std::unique_ptr<ModulePassManager> scalar(new ModulePassManager());
    scalar->add(std::unique_ptr<FunctionPass>(new LocalDAG()));
    scalar->add(std::unique_ptr<FunctionPass>(new SimplifyCFG()));
    pipeline.addModule(std::move(scalar));
    if (optLevel == 1) return pipeline;

    std::unique_ptr<ModulePassManager> ssa(new ModulePassManager());
    ssa->add(std::unique_ptr<FunctionPass>(new Mem2Reg()));
    ssa->add(std::unique_ptr<FunctionPass>(new InstSimplify()));
    ssa->add(std::unique_ptr<FunctionPass>(new LoopSimplify()));
    ssa->add(std::unique_ptr<FunctionPass>(new LICM()));
    if (optLevel >= 3) ssa->add(std::unique_ptr<FunctionPass>(new IVStrengthReduce()));
    ssa->add(std::unique_ptr<FunctionPass>(new LoopUnswitch()));
    ssa->add(std::unique_ptr<FunctionPass>(new SimplifyCFG()));
    ssa->add(std::unique_ptr<FunctionPass>(new InstSimplify()));
    ssa->add(std::unique_ptr<FunctionPass>(new CSE()));
    ssa->add(std::unique_ptr<FunctionPass>(new DCE()));
    ssa->add(std::unique_ptr<FunctionPass>(new Sink()));
    pipeline.addModule(std::move(ssa));

    return pipeline;
}

}
