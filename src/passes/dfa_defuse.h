#ifndef PASSES_DFA_DEFUSE_H
#define PASSES_DFA_DEFUSE_H

#include <map>
#include <ostream>
#include <vector>

#include "ir/cfg.h"

namespace passes {

//def-use chains (dragon book 9.2.4): for every value, the instructions that read it
//in ssa each value has one definition, the value itself, so the chain is just its users
class DefUse {
public:
    explicit DefUse(const ir::Function &fn);

    //every instruction that reads value, once each, in layout order
    const std::vector<ir::Instruction *> &users(const ir::Value *value) const;
    bool                                  hasUsers(const ir::Value *value) const { return !users(value).empty(); }

    //every local value (argument or instruction) with its users, names sorted
    void dump(std::ostream &out) const;

private:
    const ir::Function                                          &fn;
    std::map<const ir::Value *, std::vector<ir::Instruction *>> userMap;
};

} // namespace passes

#endif
