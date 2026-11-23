#ifndef PASSES_MODULE_PASS_MANAGER_H
#define PASSES_MODULE_PASS_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "passes/pass.h"
#include "passes/pass_options.h"

namespace passes {

//runs a list of function passes, in order, over every function the module defines
//one module stage of the GlobalPassManager
class ModulePassManager {
public:
    void   add(std::unique_ptr<FunctionPass> pass) { passes.push_back(std::move(pass)); }
    size_t size() const { return passes.size(); }

    //false when --verify-each found broken ir, error then names the pass and function
    bool run(ir::Module &module, AnalysisManager &am, const PassOptions &options, std::string &error);

private:
    std::vector<std::unique_ptr<FunctionPass>> passes;
};

} // namespace passes

#endif
