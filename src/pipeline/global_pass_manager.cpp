#include "pipeline/global_pass_manager.h"

#include "pipeline/verify.h"

namespace passes {

void GlobalPassManager::addGlobal(std::unique_ptr<GlobalPass> pass) {
    Stage stage;
    stage.global = std::move(pass);
    stages.push_back(std::move(stage));
}

void GlobalPassManager::addModule(std::unique_ptr<ModulePassManager> module) {
    Stage stage;
    stage.module = std::move(module);
    stages.push_back(std::move(stage));
}

bool GlobalPassManager::verifyAll(ir::Module &module, AnalysisManager &am, const std::string &when) {
    for (const std::unique_ptr<ir::Function> &fn : module.getFunctions()) {
        if (!fn->isDefinition()) continue;
        std::string problem;
        if (!verifyWithDominance(*fn, am, problem)) {
            errorText = when + ", @" + fn->getName() + " " + problem;
            return false;
        }
    }
    return true;
}

bool GlobalPassManager::run(ir::Module &module, AnalysisManager &am) {
    if (options.verifyEach && !verifyAll(module, am, "before any pass")) return false;

    for (Stage &stage : stages) {
        if (stage.module) {
            if (!stage.module->run(module, am, options, errorText)) return false;
            continue;
        }
        if (!stage.global->run(module, am)) continue;
        //a global pass may have changed any function, so every analysis is dropped
        for (const std::unique_ptr<ir::Function> &fn : module.getFunctions()) {
            if (!fn->isDefinition()) continue;
            fn->recomputeCFG();
            am.invalidate(*fn);
        }
        am.invalidateModule();
        if (options.trace) {
            *options.trace << "; after " << stage.global->name() << "\n";
            module.dump(*options.trace);
            *options.trace << "\n";
        }
        if (options.verifyEach && !verifyAll(module, am, "after " + std::string(stage.global->name()))) {
            return false;
        }
    }
    return true;
}

} // namespace passes
