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

//local per-block register allocator
class RegisterAllocator {
public:
    RegisterAllocator(Frame &frame, MCode &out) : frame(frame), out(out) {}

    void beginBlock();

    bool registerOf(const ir::Value *value, Reg &reg) const;

    //picks a free register, spilling the farthest-used value
    Reg allocate(const ir::Value *value, bool isFloat, const NextUseMap &nextUse);

    void release(const ir::Value *value);

    //saves live values held in call-clobbered registers
    void spillCallerSaved(const NextUseMap &nextUse);

    //stores live-out values and frees all registers
    void flush(const passes::ValueSet &liveOut);

    const std::set<Reg> &usedCalleeSaved() const { return calleeSavedUsed; }

private:
    void storeHome(const ir::Value *value, Reg reg);
    void drop(Reg reg);

    Frame                          &frame;
    MCode                          &out;
    std::map<Reg, const ir::Value *> holds;
    std::map<const ir::Value *, Reg> where;
    std::set<const ir::Value *>      registerOnly;
    std::set<Reg>                    calleeSavedUsed;
};

bool isCallerSaved(Reg reg);

}

#endif
