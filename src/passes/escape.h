//escape analysis: which stack slots other code can reach through their address
#ifndef PASSES_ESCAPE_H
#define PASSES_ESCAPE_H

#include <set>

#include "ir/cfg.h"

namespace passes {

class EscapeInfo {
public:
    explicit EscapeInfo(const ir::Function &fn);

    //the alloca or global an address is computed from, else the address itself
    static const ir::Value *baseOf(const ir::Value *address);

    bool escapes(const ir::Value *alloca) const { return escaped.count(alloca) != 0; }

    //a scalar slot used only by whole loads and stores of its type, so it can live in a register
    bool isPromotable(const ir::Instruction *alloca) const { return promotable.count(alloca) != 0; }

    //false only when the two addresses surely name different memory
    bool mayAlias(const ir::Value *a, const ir::Value *b) const;

    //false only when a call cannot reach this address
    bool callMayTouch(const ir::Value *address) const;

private:
    std::set<const ir::Value *> escaped;
    std::set<const ir::Value *> promotable;
};

} // namespace passes

#endif
