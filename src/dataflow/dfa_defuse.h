//def-use chains: the instructions that read each value
#ifndef DATAFLOW_DFA_DEFUSE_H
#define DATAFLOW_DFA_DEFUSE_H

#include <map>
#include <ostream>
#include <vector>

#include "ir/cfg.h"

namespace passes {

class DefUse {
public:
    explicit DefUse(const ir::Function &fn);

    const std::vector<ir::Instruction *> &users(const ir::Value *value) const;
    bool                                  hasUsers(const ir::Value *value) const { return !users(value).empty(); }

    void dump(std::ostream &out) const;

private:
    const ir::Function                                          &fn;
    std::map<const ir::Value *, std::vector<ir::Instruction *>> userMap;
};

} // namespace passes

#endif
