#ifndef PASSES_DFA_LIVENESS_H
#define PASSES_DFA_LIVENESS_H

#include <map>
#include <ostream>
#include <set>

#include "ir/cfg.h"

namespace passes {

using ValueSet = std::set<const ir::Value *>;

//live variables (dragon book 9.2.5): a backward union problem over the function's local values
//a phi's operand is live out of the predecessor it comes from, not live into the phi's block
class Liveness {
public:
    explicit Liveness(const ir::Function &fn);

    //the values liveness follows: arguments and instructions that produce a value
    static bool isTracked(const ir::Value *value);

    const ValueSet &liveIn(const ir::BasicBlock *block) const { return find(in, block); }
    const ValueSet &liveOut(const ir::BasicBlock *block) const { return find(out, block); }

    //use_B: read in the block before any definition there. def_B: defined in the block
    const ValueSet &uses(const ir::BasicBlock *block) const { return find(use, block); }
    const ValueSet &defs(const ir::BasicBlock *block) const { return find(def, block); }

    //rounds the solver took, for tests
    unsigned iterations() const { return rounds; }

    //per block: use, def, in and out, names sorted
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

//writes a set as {%a, %t3}, sorted by name so the output does not depend on addresses
void dumpValueSet(std::ostream &out, const ValueSet &values);

} // namespace passes

#endif
