#ifndef PASSES_PASS_MANAGER_H
#define PASSES_PASS_MANAGER_H

#include <memory>
#include <ostream>
#include <vector>

#include "passes/pass.h"

namespace passes {

//runs a list of passes, in order, over every function the module defines
class PassManager {
public:
    void   add(std::unique_ptr<FunctionPass> pass) { passes.push_back(std::move(pass)); }
    size_t size() const { return passes.size(); }

    //when set, each function is written there after every pass that changed it
    void setTrace(std::ostream *out) { trace = out; }

    //true when any pass changed any function
    bool run(ir::Module &module, AnalysisManager &am);

private:
    std::vector<std::unique_ptr<FunctionPass>> passes;
    std::ostream                              *trace = nullptr;
};

} // namespace passes

#endif
