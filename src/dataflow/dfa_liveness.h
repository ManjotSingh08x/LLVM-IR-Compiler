//live variables: which values are live into and out of each block
#ifndef DATAFLOW_DFA_LIVENESS_H
#define DATAFLOW_DFA_LIVENESS_H

#include <map>
#include <ostream>
#include <set>

#include "ir/cfg.h"

namespace passes {

using ValueSet = std::set<const ir::Value *>;

class Liveness {
public:
    explicit Liveness(const ir::Function &fn);

    //arguments and instructions that produce a value
    static bool isTracked(const ir::Value *value);

    const ValueSet &liveIn(const ir::BasicBlock *block) const { return find(in, block); }
    const ValueSet &liveOut(const ir::BasicBlock *block) const { return find(out, block); }

    const ValueSet &uses(const ir::BasicBlock *block) const { return find(use, block); }
    const ValueSet &defs(const ir::BasicBlock *block) const { return find(def, block); }

    unsigned iterations() const { return rounds; }

    void dump(std::ostream &out) const;

private:
    static const ValueSet &find(const std::map<const ir::BasicBlock *, ValueSet> &sets, const ir::BasicBlock *block);

    const ir::Function                         &fn;
    std::map<const ir::BasicBlock *, ValueSet> use;
    std::map<const ir::BasicBlock *, ValueSet> def;
    std::map<const ir::BasicBlock *, ValueSet> in;
    std::map<const ir::BasicBlock *, ValueSet> out;
    unsigned                                   rounds = 0;
};

void dumpValueSet(std::ostream &out, const ValueSet &values);

} // namespace passes

#endif
