#include "passes/pass_manager.h"

namespace passes {

bool PassManager::run(ir::Module &module, AnalysisManager &am) {
    bool anyChange = false;
    for (const std::unique_ptr<ir::Function> &fn : module.getFunctions()) {
        if (!fn->isDefinition()) continue;
        for (const std::unique_ptr<FunctionPass> &pass : passes) {
            if (!pass->run(*fn, module, am)) continue;
            anyChange = true;
            //a pass may have moved branches: rebuild the edges, then forget stale analyses
            fn->recomputeCFG();
            am.invalidate(*fn);
            if (trace) {
                *trace << "; after " << pass->name() << "\n";
                fn->dump(*trace);
                *trace << "\n";
            }
        }
    }
    return anyChange;
}

} // namespace passes
