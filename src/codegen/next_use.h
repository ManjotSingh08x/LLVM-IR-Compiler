#ifndef CODEGEN_NEXT_USE_H
#define CODEGEN_NEXT_USE_H

#include <climits>
#include <map>
#include <vector>

#include "ir/cfg.h"
#include "passes/dfa_liveness.h"

namespace codegen {

//next-use information (dragon book 8.4.2): after an instruction, where each name is read next
const long kDead = -1;          //never read again: not later in the block and not live on exit
const long kLiveOut = LONG_MAX; //not read again in the block but live on exit

using NextUseMap = std::map<const ir::Value *, long>;

//what is attached to one instruction
struct InstNextUse {
    NextUseMap operands;        //per operand: its next use after this instruction
    long       result = kDead;  //the next use of the value the instruction defines
};

//one backward scan of a block, starting from the block's live-out set
class NextUse {
public:
    NextUse(const ir::BasicBlock &block, const passes::ValueSet &liveOut);

    const InstNextUse &at(size_t index) const { return table[index]; }

private:
    std::vector<InstNextUse> table;
};

} // namespace codegen

#endif
