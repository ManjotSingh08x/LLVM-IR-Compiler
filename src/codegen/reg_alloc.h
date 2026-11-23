#ifndef CODEGEN_REG_ALLOC_H
#define CODEGEN_REG_ALLOC_H

#include <map>
#include <set>
#include <vector>

#include "codegen/frame.h"
#include "codegen/next_use.h"
#include "codegen/x86.h"

namespace codegen {

//register and address descriptors with getReg, the simple code generator of the dragon book (8.6)
//registers cache values within one block. at a block's end every live value is back in memory
//  integer: rbx, r12-r15 (callee-saved, survive calls), r10 (caller-saved)
//  float:   xmm8-xmm15 (caller-saved)
//rax, rcx, rdx, r11, xmm0, xmm1 are scratch for single instructions, the argument registers carry calls
class RegisterAllocator {
public:
    RegisterAllocator(Frame &frame, AsmLines &out) : frame(frame), out(out) {}

    //a new block: no register holds anything, every value is in its home
    void beginBlock();

    //the register holding value, or false
    bool registerOf(const ir::Value *value, Reg &reg) const;

    //getReg for a value about to be defined: a free register, else the one whose value
    //is needed furthest away, stored to its home first when that is its only copy
    Reg allocate(const ir::Value *value, bool isFloat, const NextUseMap &nextUse);

    //value is dead: its register is free again
    void release(const ir::Value *value);

    //before a call: values still needed in caller-saved registers go home, the registers are emptied
    void spillCallerSaved(const NextUseMap &nextUse);

    //at a block's end: every value live on exit whose only copy is a register is stored home
    void flush(const passes::ValueSet &liveOut);

    //callee-saved registers this function wrote, for its prologue and epilogue
    const std::set<Reg> &usedCalleeSaved() const { return calleeSavedUsed; }

private:
    void storeHome(const ir::Value *value, Reg reg);
    void drop(Reg reg);

    Frame                          &frame;
    AsmLines                       &out;
    std::map<Reg, const ir::Value *> holds;         //register descriptor
    std::map<const ir::Value *, Reg> where;         //address descriptor: register part
    std::set<const ir::Value *>      registerOnly;  //address descriptor: not in memory
    std::set<Reg>                    calleeSavedUsed;
};

//true for registers a call does not preserve
bool isCallerSaved(Reg reg);

} // namespace codegen

#endif
