#ifndef CODEGEN_X86_H
#define CODEGEN_X86_H

#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ir/cfg.h"

//x86-64 registers, operand widths, memory operands and symbols shared by the backend
namespace codegen {

//one function's assembly, one line each, kept as text for the peephole pass
using AsmLines = std::vector<std::string>;

enum class Reg {
    RAX, RCX, RDX, RBX, RSI, RDI, RBP, RSP,
    R8, R9, R10, R11, R12, R13, R14, R15,
    XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7,
    XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15
};

//i1 and i8 are bytes, i16 words, i32 long, i64 and ptr quad
enum class Width { Byte, Word, Long, Quad };

//-O3 virtual registers (%v<n> for integers, %x<n> for floats), numbered above the physical ones
Reg  virtualInt(unsigned n);
Reg  virtualFloat(unsigned n);
bool isVirtual(Reg reg);

bool        isXmm(Reg reg);
std::string regName(Reg reg, Width width = Width::Quad);
char        suffix(Width width);

Width widthOf(const sema::Type *type);
bool  isFloatType(const sema::Type *type);
bool  isDoubleType(const sema::Type *type);
bool  isIntegerLike(const sema::Type *type);  //bool, char, int, i64 and every pointer kind

bool fitsInt32(long long value);

//a memory operand: off(%base), or symbol+off(%rip) when symbol is set
struct Mem {
    std::string symbol;
    Reg         base = Reg::RBP;
    long long   disp = 0;

    static Mem frame(long long offset) { Mem m; m.disp = offset; return m; }
    static Mem at(Reg reg) { Mem m; m.base = reg; return m; }
    static Mem global(const std::string &name) { Mem m; m.symbol = name; return m; }

    Mem         plus(long long bytes) const { Mem m = *this; m.disp += bytes; return m; }
    std::string str() const;
};

//the assembly name of a global or function (private ones get the .L prefix)
std::string symbolName(const ir::Value *value);

//floating constants, written once each into .rodata
class ConstantPool {
public:
    std::string label(double value, bool isDouble);
    void        emit(std::ostream &out) const;

private:
    std::map<std::pair<uint64_t, bool>, std::string> labels;  //(bits, isDouble)
};

uint64_t floatBits(double value, bool isDouble);

std::string escapeAscii(const std::string &bytes);

} // namespace codegen

#endif
