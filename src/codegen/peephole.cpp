#include "codegen/peephole.h"

#include <map>
#include <string>
#include <vector>

namespace codegen {

namespace {

//one line split into its parts: "\tmovl\t%eax, -4(%rbp)" -> movl [%eax, -4(%rbp)]
struct Line {
    enum Kind { Instruction, Label, Other } kind = Other;
    std::string              mnemonic;
    std::vector<std::string> operands;
    std::string              label;  //for Label, without the colon
};

Line parse(const std::string &text) {
    Line line;
    if (!text.empty() && text[0] != '\t' && text.back() == ':') {
        line.kind = Line::Label;
        line.label = text.substr(0, text.size() - 1);
        return line;
    }
    if (text.size() < 2 || text[0] != '\t' || text[1] == '.') return line;
    line.kind = Line::Instruction;
    size_t tab = text.find('\t', 1);
    line.mnemonic = text.substr(1, tab == std::string::npos ? std::string::npos : tab - 1);
    if (tab == std::string::npos) return line;
    //operands are separated by ", "; a memory operand has no space inside
    std::string rest = text.substr(tab + 1);
    size_t start = 0;
    while (true) {
        size_t comma = rest.find(", ", start);
        line.operands.push_back(rest.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
        if (comma == std::string::npos) break;
        start = comma + 2;
    }
    return line;
}

std::string format(const std::string &mnemonic, const std::vector<std::string> &operands) {
    std::string text = "\t" + mnemonic;
    for (size_t i = 0; i < operands.size(); i++) text += (i == 0 ? "\t" : ", ") + operands[i];
    return text;
}

bool isRegister(const std::string &op) { return !op.empty() && op[0] == '%'; }
bool isMemory(const std::string &op) { return op.find('(') != std::string::npos; }
bool startsWith(const std::string &s, const std::string &prefix) { return s.compare(0, prefix.size(), prefix) == 0; }

bool isMove(const std::string &m) {
    return m == "movq" || m == "movl" || m == "movw" || m == "movb" || m == "movss" || m == "movsd" || m == "movaps";
}

//moves where the destination gets all of the source, so moving back is a no-op
bool isFullCopy(const std::string &m) { return m == "movq" || m == "movaps"; }

bool isUnconditionalExit(const std::string &m) { return m == "jmp" || m == "ret" || m == "ud2"; }

bool isConditionalJump(const std::string &m) { return m.size() > 1 && m[0] == 'j' && m != "jmp"; }

//what an instruction does to the flags as far as a rewrite is concerned
enum class Flags { Neutral, Writes, Reads, Unknown };

Flags flagEffect(const std::string &m) {
    if (isConditionalJump(m) || startsWith(m, "set") || startsWith(m, "cmov") || startsWith(m, "adc") ||
        startsWith(m, "sbb")) {
        return Flags::Reads;
    }
    //the flags are never live across a block boundary or a call in this backend
    if (isUnconditionalExit(m) || startsWith(m, "call") || m == "leave") return Flags::Writes;
    static const char *writers[] = { "cmp", "test", "add", "sub", "and", "or", "xor", "neg", "imul",
                                     "ucomis", "comis", "idiv", "div" };
    for (const char *w : writers) {
        if (startsWith(m, w)) {
            //xorps/xorpd, andps... are sse logic ops that leave the flags alone
            if ((startsWith(m, "xorp") || startsWith(m, "andp") || startsWith(m, "orp")) ) return Flags::Neutral;
            //sub/add on sse registers (subsd, addss...) leave the flags alone
            if ((startsWith(m, "adds") || startsWith(m, "subs") || startsWith(m, "addp") || startsWith(m, "subp")) &&
                m.size() > 4) {
                return Flags::Neutral;
            }
            return Flags::Writes;
        }
    }
    //shifts by %cl keep the flags when the count is 0, inc/dec keep the carry, bt sets only the carry
    if (startsWith(m, "sh") || startsWith(m, "sa") || startsWith(m, "ro") || startsWith(m, "inc") ||
        startsWith(m, "dec") || startsWith(m, "bt")) {
        return Flags::Unknown;
    }
    return Flags::Neutral;
}

//true when the flags after line i are overwritten before anything reads them
bool flagsDeadAfter(const std::vector<Line> &lines, size_t i) {
    for (size_t j = i + 1; j < lines.size(); j++) {
        if (lines[j].kind == Line::Label) return false;  //be safe at a local label
        if (lines[j].kind != Line::Instruction) continue;
        switch (flagEffect(lines[j].mnemonic)) {
        case Flags::Writes:  return true;
        case Flags::Neutral: continue;
        default:             return false;
        }
    }
    return true;
}

//the 32-bit name of a 64-bit general register
std::string low32(const std::string &reg) {
    static const std::map<std::string, std::string> names = {
        { "%rax", "%eax" }, { "%rbx", "%ebx" }, { "%rcx", "%ecx" }, { "%rdx", "%edx" },
        { "%rsi", "%esi" }, { "%rdi", "%edi" }, { "%r8", "%r8d" }, { "%r9", "%r9d" },
        { "%r10", "%r10d" }, { "%r11", "%r11d" }, { "%r12", "%r12d" }, { "%r13", "%r13d" },
        { "%r14", "%r14d" }, { "%r15", "%r15d" } };
    std::map<std::string, std::string>::const_iterator it = names.find(reg);
    return it == names.end() ? "" : it->second;
}

bool isGeneral32(const std::string &reg) {
    for (const char *r : { "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "%r8d", "%r9d", "%r10d", "%r11d",
                           "%r12d", "%r13d", "%r14d", "%r15d" }) {
        if (reg == r) return true;
    }
    return false;
}

//one sweep; true when anything changed
bool sweep(AsmLines &text) {
    std::vector<Line> lines;
    for (const std::string &t : text) lines.push_back(parse(t));
    std::vector<bool> drop(text.size(), false);
    bool changed = false;

    //the first instruction after each label, for jumps to jumps
    std::map<std::string, size_t> target;
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i].kind != Line::Label) continue;
        size_t j = i + 1;
        while (j < lines.size() && lines[j].kind != Line::Instruction) j++;
        if (j < lines.size()) target[lines[i].label] = j;
    }

    auto next = [&](size_t i) {
        size_t j = i + 1;
        while (j < lines.size() && lines[j].kind == Line::Other) j++;
        return j;
    };

    for (size_t i = 0; i < lines.size(); i++) {
        Line &l = lines[i];
        if (l.kind != Line::Instruction || drop[i]) continue;
        const std::string &m = l.mnemonic;

        //unreachable code: after jmp, ret or ud2 until the next label
        if (isUnconditionalExit(m)) {
            for (size_t j = i + 1; j < lines.size() && lines[j].kind != Line::Label; j++) {
                if (lines[j].kind == Line::Instruction && !drop[j]) {
                    drop[j] = true;
                    changed = true;
                }
            }
        }

        //jumps to jumps, and a jmp to the label right after it
        if ((m == "jmp" || isConditionalJump(m)) && l.operands.size() == 1) {
            std::map<std::string, size_t>::iterator t = target.find(l.operands[0]);
            if (t != target.end() && lines[t->second].mnemonic == "jmp" && lines[t->second].operands.size() == 1 &&
                lines[t->second].operands[0] != l.operands[0]) {
                l.operands[0] = lines[t->second].operands[0];
                text[i] = format(m, l.operands);
                changed = true;
            }
            if (m == "jmp") {
                for (size_t j = i + 1; j < lines.size() && lines[j].kind != Line::Instruction; j++) {
                    if (lines[j].kind == Line::Label && lines[j].label == l.operands[0]) {
                        drop[i] = true;
                        changed = true;
                        break;
                    }
                }
                if (drop[i]) continue;
            }
        }

        if (l.operands.size() == 2) {
            const std::string &a = l.operands[0], &b = l.operands[1];

            //mov R, R
            if (isFullCopy(m) && a == b) {
                drop[i] = true;
                changed = true;
                continue;
            }

            //pairs with the instruction right after (no label in between)
            size_t j = next(i);
            if (isMove(m) && j < lines.size() && lines[j].kind == Line::Instruction && !drop[j] &&
                lines[j].mnemonic == m && lines[j].operands.size() == 2 && lines[j].operands[0] == b &&
                lines[j].operands[1] == a) {
                //store then reload, load then store back: the second does nothing
                bool memoryPair = (isRegister(a) && isMemory(b)) || (isMemory(a) && isRegister(b));
                //register copy and copy back of the whole register
                bool registerPair = isRegister(a) && isRegister(b) && isFullCopy(m);
                if (memoryPair || registerPair) {
                    drop[j] = true;
                    changed = true;
                }
            }

            //cmp $0, R -> test R, R: the same flags for every condition
            if (startsWith(m, "cmp") && m.size() == 4 && a == "$0" && isRegister(b)) {
                text[i] = format("test" + m.substr(3), { b, b });
                changed = true;
                continue;
            }

            //the rest change the flags: only when nothing reads them before they are set again
            bool integerOp = m.size() == 4 && (m.back() == 'l' || m.back() == 'q');
            if (!integerOp || !isRegister(b) || !flagsDeadAfter(lines, i)) continue;
            std::string base = m.substr(0, 3);
            char width = m.back();
            if ((base == "add" || base == "sub") && a == "$0") {
                drop[i] = true;
                changed = true;
            } else if (m.substr(0, 4) == "imul" && a == "$1") {
                drop[i] = true;
                changed = true;
            } else if ((base == "add" || base == "sub") && a == "$1") {
                text[i] = format((base == "add" ? "inc" : "dec") + std::string(1, width), { b });
                changed = true;
            } else if (base == "mov" && a == "$0") {
                std::string reg = width == 'q' ? low32(b) : (isGeneral32(b) ? b : "");
                if (!reg.empty()) {
                    text[i] = format("xorl", { reg, reg });
                    changed = true;
                }
            }
        }
    }

    if (!changed) return false;
    AsmLines kept;
    for (size_t i = 0; i < text.size(); i++) {
        if (!drop[i]) kept.push_back(text[i]);
    }
    text.swap(kept);
    return true;
}

} // namespace

void peephole(AsmLines &lines) {
    for (int round = 0; round < 16 && sweep(lines); round++) {
    }
}

} // namespace codegen
