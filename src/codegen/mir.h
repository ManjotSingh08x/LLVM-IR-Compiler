#ifndef CODEGEN_MIR_H
#define CODEGEN_MIR_H

#include <string>
#include <vector>

#include "codegen/x86.h"

namespace codegen {

//the machine ir: x86-64 instructions as isel makes them, before they become text. every register
//an instruction touches is stated, with what the instruction does to it, so the register
//allocator never has to guess from the mnemonic
//  explicit operands carry a role: read (Use), written (Def) or both (UseDef)
//  registers touched without being written as operands are listed as implicit uses and defs:
//    idiv reads and writes rax and rdx, cltd reads rax and writes rdx, a call reads its argument
//    registers and overwrites every caller-saved register, a return reads rax and xmm0
//a register may be physical or virtual (-O3 only, see x86.h)

enum class Role { Use, Def, UseDef };

struct MOperand {
    enum Kind { Register, Immediate, Memory, Text } kind = Text;
    Reg         reg = Reg::RAX;
    Width       width = Width::Quad;
    Role        role = Role::Use;
    bool        indirect = false;  //*%r11 of an indirect call
    long long   value = 0;         //Immediate
    Mem         mem;               //Memory: its base register (not rbp, rsp or %rip) is read
    std::string text;              //Text: a label, a symbol, a hex immediate

    std::string str() const;
};

MOperand use(Reg reg, Width width = Width::Quad);
MOperand def(Reg reg, Width width = Width::Quad);
MOperand useDef(Reg reg, Width width = Width::Quad);
MOperand immOp(long long value);
MOperand memOp(const Mem &mem);
MOperand textOp(const std::string &text);

struct MInst {
    enum Kind {
        Op,      //an instruction
        Label,   //label:
        Raw,     //a line of text with no register information (only allowed at -O0 to -O2)
        Marker   //not printed: only implicit uses (the return value at the end of a function)
    } kind = Op;
    std::string           op;  //the mnemonic, or the label name, or the raw line
    std::vector<MOperand> operands;
    std::vector<Reg>      implicitUses;
    std::vector<Reg>      implicitDefs;
    bool                  isCall = false;

    std::string str() const;  //the line of assembly; empty for a marker
};

using MCode = std::vector<MInst>;

//the registers an instruction reads and writes, explicit and implicit; a memory operand's base
//register is read. rbp and rsp are never listed, nothing allocates them
void readsAndWrites(const MInst &inst, std::vector<Reg> &uses, std::vector<Reg> &defs);

//a whole-register copy between two registers of one class, the moves coalescing removes
bool isRegisterCopy(const MInst &inst, Reg &from, Reg &to);

void printCode(const MCode &code, AsmLines &out);

} // namespace codegen

#endif
