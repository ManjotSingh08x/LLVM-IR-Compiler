#ifndef PASSES_ANALYSIS_MANAGER_H
#define PASSES_ANALYSIS_MANAGER_H

#include <map>
#include <memory>

#include "ir/cfg.h"
#include "passes/dfa_defuse.h"
#include "passes/dfa_liveness.h"

namespace passes {

//works out each analysis of a function on first request and keeps it until the function changes
class AnalysisManager {
public:
    const Liveness &getLiveness(const ir::Function &fn);
    const DefUse   &getDefUse(const ir::Function &fn);

    //forgets everything known about fn, called after a pass changes it
    void invalidate(const ir::Function &fn);

private:
    std::map<const ir::Function *, std::unique_ptr<Liveness>> liveness;
    std::map<const ir::Function *, std::unique_ptr<DefUse>>   defUse;
};

} // namespace passes

#endif
