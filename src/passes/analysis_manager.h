#ifndef PASSES_ANALYSIS_MANAGER_H
#define PASSES_ANALYSIS_MANAGER_H

#include <map>
#include <memory>

#include "ir/cfg.h"
#include "passes/dfa_callgraph.h"
#include "passes/dfa_defuse.h"
#include "passes/dfa_dominators.h"
#include "passes/dfa_liveness.h"
#include "passes/dfa_loops.h"

namespace passes {

//works out each analysis on first request and keeps it until what it describes changes
//per function: liveness, def-use, dominators, loops. per module: the call graph
class AnalysisManager {
public:
    const Liveness   &getLiveness(const ir::Function &fn);
    const DefUse     &getDefUse(const ir::Function &fn);
    const Dominators &getDominators(const ir::Function &fn);
    const LoopInfo   &getLoops(const ir::Function &fn);  //built on getDominators

    //built over the whole module, dropped by invalidateModule
    const CallGraph &getCallGraph(const ir::Module &module);

    //forgets everything known about fn, called after a pass changes it
    void invalidate(const ir::Function &fn);

    //forgets the module-wide analyses, called after any function changes
    void invalidateModule();

private:
    std::map<const ir::Function *, std::unique_ptr<Liveness>> liveness;
    std::map<const ir::Function *, std::unique_ptr<DefUse>>   defUse;
    std::map<const ir::Function *, std::unique_ptr<Dominators>> dominators;
    std::map<const ir::Function *, std::unique_ptr<LoopInfo>> loops;
    std::unique_ptr<CallGraph>                                callGraph;
    const ir::Module                                         *callGraphModule = nullptr;
};

} // namespace passes

#endif
