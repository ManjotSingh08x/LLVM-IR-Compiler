#ifndef PASSES_ESCAPE_H
#define PASSES_ESCAPE_H

#include <set>

#include "ir/cfg.h"

namespace passes {

//which stack slots other code can reach. an alloca escapes when its address, or an address
//computed from it by getelementptr, is used for anything but the address of a load or a store
//(passed to a call, stored somewhere, cast, compared, returned). a slot that does not escape
//can only be read or written through addresses derived from it, so a call or a store through
//some other pointer can never touch it
class EscapeInfo {
public:
    explicit EscapeInfo(const ir::Function &fn);

    //the alloca or global an address is computed from through getelementptr, else the address
    static const ir::Value *baseOf(const ir::Value *address);

    //true for an alloca whose address leaves load/store/getelementptr use
    bool escapes(const ir::Value *alloca) const { return escaped.count(alloca) != 0; }

    //a scalar slot (integer, floating or pointer) used only as the address of whole loads and
    //stores of its own type: it can live in a register (mem2reg) or be tracked as a variable
    //(const_prop_dense)
    bool isPromotable(const ir::Instruction *alloca) const { return promotable.count(alloca) != 0; }

    //false only when the two addresses can be proven to name different memory
    bool mayAlias(const ir::Value *a, const ir::Value *b) const;

    //false only when a call cannot read or write memory at this address
    bool callMayTouch(const ir::Value *address) const;

private:
    std::set<const ir::Value *> escaped;
    std::set<const ir::Value *> promotable;
};

} // namespace passes

#endif
