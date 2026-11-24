//runs function passes in order over every function of the module
#ifndef PIPELINE_MODULE_PASS_MANAGER_H
#define PIPELINE_MODULE_PASS_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "pipeline/pass.h"
#include "pipeline/pass_options.h"

namespace passes {

class ModulePassManager {
public:
    void   add(std::unique_ptr<FunctionPass> pass) { passes.push_back(std::move(pass)); }
    size_t size() const { return passes.size(); }

    //false when --verify-each found broken ir; error names the pass and function
    bool run(ir::Module &module, AnalysisManager &am, const PassOptions &options, std::string &error);

private:
    std::vector<std::unique_ptr<FunctionPass>> passes;
};

} // namespace passes

#endif
