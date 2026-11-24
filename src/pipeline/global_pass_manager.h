//the top-level pass driver: runs global passes and module stages in order
#ifndef PIPELINE_GLOBAL_PASS_MANAGER_H
#define PIPELINE_GLOBAL_PASS_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "pipeline/module_pass_manager.h"
#include "pipeline/pass.h"
#include "pipeline/pass_options.h"

namespace passes {

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
