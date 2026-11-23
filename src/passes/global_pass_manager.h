#ifndef PASSES_GLOBAL_PASS_MANAGER_H
#define PASSES_GLOBAL_PASS_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "passes/module_pass_manager.h"
#include "passes/pass.h"
#include "passes/pass_options.h"

namespace passes {

//the top-level pass driver, the only one the compiler driver talks to
//it runs an ordered list of stages, each either one global pass (sees every function at once,
//such as inlining) or one module stage (a ModulePassManager of function passes). a module stage
//finishes every function before the next stage starts, so a global pass always sees every
//function in the same state. the inter-function facts global passes need come from the call
//graph in the AnalysisManager, rebuilt after anything changes
class GlobalPassManager {
public:
    void addGlobal(std::unique_ptr<GlobalPass> pass);
    void addModule(std::unique_ptr<ModulePassManager> stage);

    size_t stageCount() const { return stages.size(); }

    void setOptions(const PassOptions &newOptions) { options = newOptions; }

    //false when --verify-each found broken ir, see error()
    bool run(ir::Module &module, AnalysisManager &am);

    const std::string &error() const { return errorText; }

private:
    struct Stage {
        std::unique_ptr<GlobalPass>        global;  //exactly one of the two is set
        std::unique_ptr<ModulePassManager> module;
    };

    bool verifyAll(ir::Module &module, AnalysisManager &am, const std::string &when);

    std::vector<Stage> stages;
    PassOptions        options;
    std::string        errorText;
};

} // namespace passes

#endif
