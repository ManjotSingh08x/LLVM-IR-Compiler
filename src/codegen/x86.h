#ifndef CODEGEN_X86_H
#define CODEGEN_X86_H

#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ir/cfg.h"

namespace codegen {

using AsmLines = std::vector<std::string>;

enum class Reg {
    RAX, RCX, RDX, RBX, RSI, RDI, RBP, RSP,
    R8, R9, R10, R11, R12, R13, R14, R15,
    XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7,
    XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15
};

enum class Width { Byte, Word, Long, Quad };

Reg  virtualInt(unsigned n);
Reg  virtualFloat(unsigned n);
bool isVirtual(Reg reg);

bool        isXmm(Reg reg);
std::string regName(Reg reg, Width width = Width::Quad);
char        suffix(Width width);

Width widthOf(const sema::Type *type);
bool  isFloatType(const sema::Type *type);
bool  isDoubleType(const sema::Type *type);
bool  isIntegerLike(const sema::Type *type);

bool fitsInt32(long long value);

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

std::string symbolName(const ir::Value *value);

//collects floating-point constants for the data section
class ConstantPool {
public:
    std::string label(double value, bool isDouble);
    void        emit(std::ostream &out) const;

private:
    std::map<std::pair<uint64_t, bool>, std::string> labels;
};

uint64_t floatBits(double value, bool isDouble);

std::string escapeAscii(const std::string &bytes);

}

#endif
