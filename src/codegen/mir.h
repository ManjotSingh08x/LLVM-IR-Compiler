#ifndef CODEGEN_MIR_H
#define CODEGEN_MIR_H

#include <string>
#include <vector>

#include "codegen/x86.h"

namespace codegen {

enum class Role { Use, Def, UseDef };

struct MOperand {
    enum Kind { Register, Immediate, Memory, Text } kind = Text;
    Reg         reg = Reg::RAX;
    Width       width = Width::Quad;
    Role        role = Role::Use;
    bool        indirect = false;
    long long   value = 0;
    Mem         mem;
    std::string text;

    std::string str() const;
};

MOperand use(Reg reg, Width width = Width::Quad);
MOperand def(Reg reg, Width width = Width::Quad);
MOperand useDef(Reg reg, Width width = Width::Quad);
MOperand immOp(long long value);
MOperand memOp(const Mem &mem);
MOperand textOp(const std::string &text);

//one machine instruction with typed operands
struct MInst {
    enum Kind {
        Op,
        Label,
        Raw,
        Marker
    } kind = Op;
    std::string           op;
    std::vector<MOperand> operands;
    std::vector<Reg>      implicitUses;
    std::vector<Reg>      implicitDefs;
    bool                  isCall = false;

    std::string str() const;
};

using MCode = std::vector<MInst>;

//lists registers an instruction reads and writes
void readsAndWrites(const MInst &inst, std::vector<Reg> &uses, std::vector<Reg> &defs);

bool isRegisterCopy(const MInst &inst, Reg &from, Reg &to);

void printCode(const MCode &code, AsmLines &out);

}

#endif
