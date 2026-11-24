#include "codegen/mir.h"

namespace codegen {

namespace {

MOperand registerOp(Reg reg, Width width, Role role) {
    MOperand m;
    m.kind = MOperand::Register;
    m.reg = reg;
    m.width = width;
    m.role = role;
    return m;
}

bool tracked(Reg reg) { return reg != Reg::RBP && reg != Reg::RSP; }

}

std::string MOperand::str() const {
    switch (kind) {
    case Register:  return (indirect ? "*" : "") + regName(reg, width);
    case Immediate: return "$" + std::to_string(value);
    case Memory:    return mem.str();
    case Text:      return text;
    }
    return text;
}

MOperand use(Reg reg, Width width) { return registerOp(reg, width, Role::Use); }
MOperand def(Reg reg, Width width) { return registerOp(reg, width, Role::Def); }
MOperand useDef(Reg reg, Width width) { return registerOp(reg, width, Role::UseDef); }

MOperand immOp(long long value) {
    MOperand m;
    m.kind = MOperand::Immediate;
    m.value = value;
    return m;
}

MOperand memOp(const Mem &mem) {
    MOperand m;
    m.kind = MOperand::Memory;
    m.mem = mem;
    return m;
}

MOperand textOp(const std::string &text) {
    MOperand m;
    m.kind = MOperand::Text;
    m.text = text;
    return m;
}

std::string MInst::str() const {
    switch (kind) {
    case Label:  return op + ":";
    case Raw:    return op;
    case Marker: return "";
    case Op:     break;
    }
    std::string text = "\t" + op;
    for (size_t i = 0; i < operands.size(); i++) text += (i == 0 ? "\t" : ", ") + operands[i].str();
    return text;
}

void readsAndWrites(const MInst &inst, std::vector<Reg> &uses, std::vector<Reg> &defs) {
    uses.clear();
    defs.clear();
    for (const MOperand &o : inst.operands) {
        if (o.kind == MOperand::Memory) {
            if (o.mem.symbol.empty() && tracked(o.mem.base)) uses.push_back(o.mem.base);
            continue;
        }
        if (o.kind != MOperand::Register || !tracked(o.reg)) continue;
        if (o.role == Role::Use || o.role == Role::UseDef) uses.push_back(o.reg);
        if (o.role == Role::Def || o.role == Role::UseDef) defs.push_back(o.reg);
    }
    for (Reg r : inst.implicitUses) {
        if (tracked(r)) uses.push_back(r);
    }
    for (Reg r : inst.implicitDefs) {
        if (tracked(r)) defs.push_back(r);
    }
}

bool isRegisterCopy(const MInst &inst, Reg &from, Reg &to) {
    if (inst.kind != MInst::Op || inst.operands.size() != 2) return false;
    if (inst.op != "movq" && inst.op != "movaps") return false;
    const MOperand &a = inst.operands[0];
    const MOperand &b = inst.operands[1];
    if (a.kind != MOperand::Register || b.kind != MOperand::Register || a.indirect) return false;
    if (isXmm(a.reg) != isXmm(b.reg)) return false;
    if (!isXmm(a.reg) && (a.width != Width::Quad || b.width != Width::Quad)) return false;
    from = a.reg;
    to = b.reg;
    return true;
}

void printCode(const MCode &code, AsmLines &out) {
    for (const MInst &inst : code) {
        if (inst.kind == MInst::Marker) continue;
        out.push_back(inst.str());
    }
}

}
