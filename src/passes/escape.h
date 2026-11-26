#ifndef PASSES_ESCAPE_H
#define PASSES_ESCAPE_H

#include <set>

#include "ir/cfg.h"

namespace passes {

//tracks which stack slots may be accessed indirectly
class EscapeInfo {
public:
    explicit EscapeInfo(const ir::Function &fn);

    static const ir::Value *baseOf(const ir::Value *address);

    bool escapes(const ir::Value *alloca) const { return escaped.count(alloca) != 0; }

    bool isPromotable(const ir::Instruction *alloca) const { return promotable.count(alloca) != 0; }

    bool mayAlias(const ir::Value *a, const ir::Value *b) const;

    //checks whether a call might read or write memory
    bool callMayTouch(const ir::Value *address) const;

private:
    std::set<const ir::Value *> escaped;
    std::set<const ir::Value *> promotable;
};

}

#endif
