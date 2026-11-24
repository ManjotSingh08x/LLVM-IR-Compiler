#ifndef CODEGEN_REG_ALLOC_H
#define CODEGEN_REG_ALLOC_H

#include <map>
#include <set>
#include <vector>

#include "codegen/frame.h"
#include "codegen/mir.h"
#include "codegen/next_use.h"
#include "codegen/x86.h"

namespace codegen {

//per-block register allocation with register and address descriptors
class RegisterAllocator {
public:
    RegisterAllocator(Frame &frame, MCode &out) : frame(frame), out(out) {}

    void beginBlock();

    bool registerOf(const ir::Value *value, Reg &reg) const;

    //getReg: a free register, else the one whose value is needed furthest away, saved home first if needed
    Reg allocate(const ir::Value *value, bool isFloat, const NextUseMap &nextUse);

    void release(const ir::Value *value);

    //before a call: values still needed in caller-saved registers are stored home
    void spillCallerSaved(const NextUseMap &nextUse);

    //at a block's end: live values held only in a register are stored home
    void flush(const passes::ValueSet &liveOut);

    const std::set<Reg> &usedCalleeSaved() const { return calleeSavedUsed; }

private:
    void storeHome(const ir::Value *value, Reg reg);
    void drop(Reg reg);

    Frame                          &frame;
    MCode                          &out;
    std::map<Reg, const ir::Value *> holds;         //register descriptor
    std::map<const ir::Value *, Reg> where;         //address descriptor: register part
    std::set<const ir::Value *>      registerOnly;  //address descriptor: not in memory
    std::set<Reg>                    calleeSavedUsed;
};

bool isCallerSaved(Reg reg);

} // namespace codegen

#endif
