#include "passes/analysis_manager.h"

namespace passes {

const Liveness &AnalysisManager::getLiveness(const ir::Function &fn) {
    std::unique_ptr<Liveness> &slot = liveness[&fn];
    if (!slot) slot.reset(new Liveness(fn));
    return *slot;
}

const DefUse &AnalysisManager::getDefUse(const ir::Function &fn) {
    std::unique_ptr<DefUse> &slot = defUse[&fn];
    if (!slot) slot.reset(new DefUse(fn));
    return *slot;
}

const CallGraph &AnalysisManager::getCallGraph(const ir::Module &module) {
    if (!callGraph || callGraphModule != &module) {
        callGraph.reset(new CallGraph(module));
        callGraphModule = &module;
    }
    return *callGraph;
}

void AnalysisManager::invalidate(const ir::Function &fn) {
    liveness.erase(&fn);
    defUse.erase(&fn);
}

void AnalysisManager::invalidateModule() {
    callGraph.reset();
    callGraphModule = nullptr;
}

} // namespace passes
