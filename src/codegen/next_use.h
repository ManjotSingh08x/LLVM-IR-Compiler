#ifndef CODEGEN_NEXT_USE_H
#define CODEGEN_NEXT_USE_H

#include <climits>
#include <map>
#include <vector>

#include "ir/cfg.h"
#include "dataflow/dfa_liveness.h"

namespace codegen {

const long kDead = -1;
const long kLiveOut = LONG_MAX;

using NextUseMap = std::map<const ir::Value *, long>;

struct InstNextUse {
    NextUseMap operands;
    long       result = kDead;
};

//records where each value is used next
class NextUse {
public:
    NextUse(const ir::BasicBlock &block, const passes::ValueSet &liveOut);

    const InstNextUse &at(size_t index) const { return table[index]; }

private:
    std::vector<InstNextUse> table;
};

}

#endif
