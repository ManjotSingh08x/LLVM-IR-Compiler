#ifndef PIPELINE_MODULE_PASS_MANAGER_H
#define PIPELINE_MODULE_PASS_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "pipeline/pass.h"
#include "pipeline/pass_options.h"

namespace passes {

//runs function passes over every function
class ModulePassManager {
public:
    void   add(std::unique_ptr<FunctionPass> pass) { passes.push_back(std::move(pass)); }
    size_t size() const { return passes.size(); }

    bool run(ir::Module &module, AnalysisManager &am, const PassOptions &options, std::string &error);

private:
    std::vector<std::unique_ptr<FunctionPass>> passes;
};

}

#endif
