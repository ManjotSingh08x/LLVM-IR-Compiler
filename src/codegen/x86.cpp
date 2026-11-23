#include "codegen/x86.h"

#include <cstdio>
#include <cstring>

#include "ir/ir_types.h"

namespace codegen {

namespace {

struct RegNames {
    const char *q;
    const char *l;
    const char *b;
};

const RegNames kGprNames[] = {
    {"rax", "eax", "al"},   {"rcx", "ecx", "cl"},   {"rdx", "edx", "dl"},   {"rbx", "ebx", "bl"},
    {"rsi", "esi", "sil"},  {"rdi", "edi", "dil"},  {"rbp", "ebp", "bpl"},  {"rsp", "esp", "spl"},
    {"r8", "r8d", "r8b"},   {"r9", "r9d", "r9b"},   {"r10", "r10d", "r10b"}, {"r11", "r11d", "r11b"},
    {"r12", "r12d", "r12b"}, {"r13", "r13d", "r13b"}, {"r14", "r14d", "r14b"}, {"r15", "r15d", "r15b"},
};

} // namespace

bool isXmm(Reg reg) { return reg >= Reg::XMM0; }

std::string regName(Reg reg, Width width) {
    if (isXmm(reg)) return "%xmm" + std::to_string((int)reg - (int)Reg::XMM0);
    const RegNames &names = kGprNames[(int)reg];
    switch (width) {
    case Width::Byte: return std::string("%") + names.b;
    case Width::Long: return std::string("%") + names.l;
    case Width::Quad: return std::string("%") + names.q;
    }
    return "";
}

char suffix(Width width) {
    switch (width) {
    case Width::Byte: return 'b';
    case Width::Long: return 'l';
    case Width::Quad: return 'q';
    }
    return 'q';
}

Width widthOf(const sema::Type *type) {
    switch (ir::integerBits(type)) {
    case 1:
    case 8:  return Width::Byte;
    case 32: return Width::Long;
    default: return Width::Quad;  //i64 and every pointer
    }
}

bool isFloatType(const sema::Type *type) { return sema::isFloating(type); }

bool isDoubleType(const sema::Type *type) { return type->is(sema::TypeKind::Double); }

bool isIntegerLike(const sema::Type *type) {
    if (ir::integerBits(type) != 0) return true;
    switch (type->getKind()) {
    case sema::TypeKind::Pointer:
    case sema::TypeKind::Reference:
    case sema::TypeKind::Null:
        return true;
    default:
        return false;
    }
}

bool fitsInt32(long long value) { return value >= -2147483648LL && value <= 2147483647LL; }

std::string Mem::str() const {
    if (!symbol.empty()) {
        if (disp == 0) return symbol + "(%rip)";
        return symbol + (disp > 0 ? "+" : "") + std::to_string(disp) + "(%rip)";
    }
    return (disp == 0 ? "" : std::to_string(disp)) + "(" + regName(base) + ")";
}

std::string symbolName(const ir::Value *value) {
    if (value->getKind() == ir::ValueKind::GlobalVariable) {
        const ir::GlobalVariable *global = static_cast<const ir::GlobalVariable *>(value);
        if (global->linkage == ir::Linkage::Private) return ".L" + global->getName();
    }
    return value->getName();
}

uint64_t floatBits(double value, bool isDouble) {
    if (isDouble) {
        uint64_t bits;
        std::memcpy(&bits, &value, sizeof bits);
        return bits;
    }
    float narrow = (float)value;
    uint32_t bits;
    std::memcpy(&bits, &narrow, sizeof bits);
    return bits;
}

std::string ConstantPool::label(double value, bool isDouble) {
    std::pair<uint64_t, bool> key(floatBits(value, isDouble), isDouble);
    std::map<std::pair<uint64_t, bool>, std::string>::iterator it = labels.find(key);
    if (it != labels.end()) return it->second;
    std::string name = ".LCPI" + std::to_string(labels.size());
    labels[key] = name;
    return name;
}

void ConstantPool::emit(std::ostream &out) const {
    if (labels.empty()) return;
    out << "\n\t.section\t.rodata\n";
    for (const std::pair<const std::pair<uint64_t, bool>, std::string> &entry : labels) {
        bool isDouble = entry.first.second;
        out << "\t.p2align\t" << (isDouble ? 3 : 2) << "\n";
        out << entry.second << ":\n";
        out << (isDouble ? "\t.quad\t" : "\t.long\t") << entry.first.first << "\n";
    }
}

std::string escapeAscii(const std::string &bytes) {
    std::string out;
    for (unsigned char c : bytes) {
        if (c >= 0x20 && c <= 0x7E && c != '"' && c != '\\') {
            out += (char)c;
        } else {
            char buffer[8];
            std::snprintf(buffer, sizeof buffer, "\\%03o", c);
            out += buffer;
        }
    }
    return out;
}

} // namespace codegen
