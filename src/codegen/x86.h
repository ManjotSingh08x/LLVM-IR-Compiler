#ifndef CODEGEN_X86_H
#define CODEGEN_X86_H

#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ir/cfg.h"

//x86-64 pieces shared by the backend: registers, operand widths, memory operands, symbols
namespace codegen {

//one function's assembly, a line each, kept as text so a peephole pass can rewrite it
using AsmLines = std::vector<std::string>;

enum class Reg {
    RAX, RCX, RDX, RBX, RSI, RDI, RBP, RSP,
    R8, R9, R10, R11, R12, R13, R14, R15,
    XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7,
    XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15
};

//the width an integer or pointer is worked on with: i1 and i8 are bytes, i32 long, i64 and ptr quad
enum class Width { Byte, Long, Quad };

//-O3 works on virtual registers until graph colouring gives each a real one: %v<n>.q/.l/.b for
//integers and pointers, %x<n> for floats. they are Reg values far above the physical ones
Reg  virtualInt(unsigned n);
Reg  virtualFloat(unsigned n);
bool isVirtual(Reg reg);

bool        isXmm(Reg reg);
std::string regName(Reg reg, Width width = Width::Quad);  //%rax, %eax, %al. an xmm ignores width
char        suffix(Width width);                         //b, l or q

Width widthOf(const sema::Type *type);
bool  isFloatType(const sema::Type *type);   //float or double
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

//the assembly name of a global or function: private symbols get the local .L prefix
std::string symbolName(const ir::Value *value);

//floating constants, written once each into .rodata
class ConstantPool {
public:
    std::string label(double value, bool isDouble);
    void        emit(std::ostream &out) const;

private:
    std::map<std::pair<uint64_t, bool>, std::string> labels;  //(bits, isDouble)
};

//the bits of a floating constant as stored in memory: 32 for float, 64 for double
uint64_t floatBits(double value, bool isDouble);

//bytes as the body of a .ascii directive
std::string escapeAscii(const std::string &bytes);

} // namespace codegen

#endif
