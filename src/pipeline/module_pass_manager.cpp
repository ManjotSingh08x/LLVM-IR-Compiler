#include "pipeline/module_pass_manager.h"

#include "pipeline/verify.h"

namespace passes {

bool ModulePassManager::run(ir::Module &module, AnalysisManager &am, const PassOptions &options,
                            std::string &error) {
    for (const std::unique_ptr<ir::Function> &fn : module.getFunctions()) {
        if (!fn->isDefinition()) continue;
        for (const std::unique_ptr<FunctionPass> &pass : passes) {
            if (!pass->run(*fn, module, am)) continue;
            fn->recomputeCFG();
            am.invalidate(*fn);
            am.invalidateModule();
            if (options.trace) {
                *options.trace << "; after " << pass->name() << "\n";
                fn->dump(*options.trace);
                *options.trace << "\n";
            }
            std::string problem;
            if (options.verifyEach && !verifyWithDominance(*fn, am, problem)) {
                error = "after " + std::string(pass->name()) + ", @" + fn->getName() + " " + problem;
                return false;
            }
        }
    }
    return true;
}

}
