#ifndef PIPELINE_ANALYSIS_MANAGER_H
#define PIPELINE_ANALYSIS_MANAGER_H

#include <map>
#include <memory>

#include "ir/cfg.h"
#include "dataflow/dfa_callgraph.h"
#include "dataflow/dfa_defuse.h"
#include "dataflow/dfa_dominators.h"
#include "dataflow/dfa_liveness.h"
#include "dataflow/dfa_loops.h"

namespace passes {

//caches analyses until the function changes
class AnalysisManager {
public:
    const Liveness   &getLiveness(const ir::Function &fn);
    const DefUse     &getDefUse(const ir::Function &fn);
    const Dominators &getDominators(const ir::Function &fn);
    const LoopInfo   &getLoops(const ir::Function &fn);

    const CallGraph &getCallGraph(const ir::Module &module);

    void invalidate(const ir::Function &fn);

    void invalidateModule();

private:
    std::map<const ir::Function *, std::unique_ptr<Liveness>> liveness;
    std::map<const ir::Function *, std::unique_ptr<DefUse>>   defUse;
    std::map<const ir::Function *, std::unique_ptr<Dominators>> dominators;
    std::map<const ir::Function *, std::unique_ptr<LoopInfo>> loops;
    std::unique_ptr<CallGraph>                                callGraph;
    const ir::Module                                         *callGraphModule = nullptr;
};

}

#endif
