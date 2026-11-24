#include "codegen/graph_color.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace codegen {

namespace {

//the registers handed out, in order of preference: caller-saved ones first (no prologue save),
//rax, rcx and rdx last (isel's fixed sequences use them most)
const std::vector<Reg> kIntColors = { Reg::RSI, Reg::RDI, Reg::R8,  Reg::R9,  Reg::R10, Reg::R11, Reg::RCX,
                                      Reg::RDX, Reg::RAX, Reg::RBX, Reg::R12, Reg::R13, Reg::R14, Reg::R15 };
const std::vector<Reg> kFloatColors = {
    Reg::XMM2, Reg::XMM3, Reg::XMM4,  Reg::XMM5,  Reg::XMM6,  Reg::XMM7,  Reg::XMM8,  Reg::XMM9,
    Reg::XMM10, Reg::XMM11, Reg::XMM12, Reg::XMM13, Reg::XMM14, Reg::XMM15, Reg::XMM0, Reg::XMM1 };

size_t kFor(Reg reg) { return isXmm(reg) ? kFloatColors.size() : kIntColors.size(); }

bool isCalleeSaved(Reg reg) {
    return reg == Reg::RBX || reg == Reg::R12 || reg == Reg::R13 || reg == Reg::R14 || reg == Reg::R15;
}

bool isJump(const MInst &inst) { return inst.kind == MInst::Op && !inst.op.empty() && inst.op[0] == 'j'; }
bool endsFlow(const MInst &inst) {
    return inst.kind == MInst::Op && (inst.op == "jmp" || inst.op == "ret" || inst.op == "ud2");
}

class Colorer {
public:
    Colorer(MCode &code, Frame &frame, const std::map<std::string, unsigned> &labelDepth, ColorStats &stats)
        : code(code), frame(frame), labelDepth(labelDepth), stats(stats) {}

    bool run(std::string &error) {
        for (const MInst &inst : code) {
            if (inst.kind == MInst::Raw) {
                error = "an instruction without register information reached graph colouring: " + inst.op;
                return false;
            }
        }
        std::set<Reg> virtuals;
        std::vector<Reg> u, d;
        for (const MInst &inst : code) {
            readsAndWrites(inst, u, d);
            for (Reg r : u) if (isVirtual(r)) virtuals.insert(r);
            for (Reg r : d) if (isVirtual(r)) virtuals.insert(r);
        }
        stats.virtuals = (unsigned)virtuals.size();
        for (int round = 0; round < 40; round++) {
            stats.rounds++;
            build();
            coalesce();
            std::vector<Reg> spills = colour();
            if (spills.empty()) {
                rewrite();
                return true;
            }
            spill(spills);
        }
        error = "graph colouring did not converge";
        return false;
    }

private:
    Reg find(Reg r) const {
        std::map<Reg, Reg>::const_iterator it = alias.find(r);
        while (it != alias.end()) {
            r = it->second;
            it = alias.find(r);
        }
        return r;
    }

    void addEdge(Reg a, Reg b) {
        if (a == b || isXmm(a) != isXmm(b)) return;
        if (!isVirtual(a) && !isVirtual(b)) return;  //two physical registers never share anyway
        adj[a].insert(b);
        adj[b].insert(a);
    }

    //machine cfg, liveness, interference, spill costs and copies
    void build() {
        alias.clear();
        adj.clear();
        cost.clear();
        copies.clear();

        std::vector<size_t> starts;
        std::map<std::string, size_t> blockOfLabel;
        for (size_t i = 0; i < code.size(); i++) {
            bool leader = i == 0 || code[i].kind == MInst::Label || isJump(code[i - 1]) || endsFlow(code[i - 1]);
            if (leader && (starts.empty() || starts.back() != i)) starts.push_back(i);
            if (code[i].kind == MInst::Label) blockOfLabel[code[i].op] = starts.size() - 1;
        }
        size_t blocks = starts.size();
        std::vector<size_t> ends(blocks);
        for (size_t b = 0; b < blocks; b++) ends[b] = b + 1 < blocks ? starts[b + 1] : code.size();
        std::vector<std::vector<size_t>> succs(blocks);
        for (size_t b = 0; b < blocks; b++) {
            const MInst *last = nullptr;
            for (size_t i = starts[b]; i < ends[b]; i++) {
                if (code[i].kind == MInst::Op) last = &code[i];
            }
            bool falls = true;
            if (last && isJump(*last)) {
                if (last->operands.size() == 1 && last->operands[0].kind == MOperand::Text) {
                    std::map<std::string, size_t>::iterator t = blockOfLabel.find(last->operands[0].text);
                    if (t != blockOfLabel.end()) succs[b].push_back(t->second);
                }
                falls = last->op != "jmp";
            }
            if (last && endsFlow(*last)) falls = false;
            if (falls && b + 1 < blocks) succs[b].push_back(b + 1);
        }

        //loop depth of each instruction, from the ir block label before it
        std::vector<unsigned> depth(code.size(), 0);
        unsigned current = 0;
        for (size_t i = 0; i < code.size(); i++) {
            if (code[i].kind == MInst::Label) {
                std::map<std::string, unsigned>::const_iterator dd = labelDepth.find(code[i].op);
                if (dd != labelDepth.end()) current = dd->second;
            }
            depth[i] = current;
        }

        std::vector<std::set<Reg>> use(blocks), def(blocks), in(blocks), out(blocks);
        std::vector<Reg> u, d;
        for (size_t b = 0; b < blocks; b++) {
            for (size_t i = starts[b]; i < ends[b]; i++) {
                readsAndWrites(code[i], u, d);
                double weight = std::pow(10.0, std::min(depth[i], 6u));
                for (Reg r : u) {
                    if (!def[b].count(r)) use[b].insert(r);
                    if (isVirtual(r)) cost[r] += weight;
                }
                for (Reg r : d) {
                    def[b].insert(r);
                    if (isVirtual(r)) cost[r] += weight;
                }
                for (Reg r : u) if (isVirtual(r)) adj[r];
                for (Reg r : d) if (isVirtual(r)) adj[r];
            }
        }
        bool changed = true;
        while (changed) {
            changed = false;
            for (size_t b = blocks; b-- > 0;) {
                std::set<Reg> o;
                for (size_t s : succs[b]) o.insert(in[s].begin(), in[s].end());
                std::set<Reg> i2 = use[b];
                for (Reg r : o) {
                    if (!def[b].count(r)) i2.insert(r);
                }
                if (o != out[b] || i2 != in[b]) {
                    out[b].swap(o);
                    in[b].swap(i2);
                    changed = true;
                }
            }
        }

        for (size_t b = 0; b < blocks; b++) {
            std::set<Reg> live = out[b];
            for (size_t i = ends[b]; i-- > starts[b];) {
                readsAndWrites(code[i], u, d);
                Reg from, to;
                if (isRegisterCopy(code[i], from, to) && (isVirtual(from) || isVirtual(to))) {
                    copies.push_back(std::make_pair(from, to));
                    live.erase(from);  //a copy's source and target may share a register
                }
                for (Reg x : d) {
                    for (Reg y : live) addEdge(x, y);
                }
                for (Reg x : d) live.erase(x);
                for (Reg x : u) live.insert(x);
            }
        }
    }

    size_t degree(Reg r) const {
        std::map<Reg, std::set<Reg>>::const_iterator it = adj.find(r);
        return it == adj.end() ? 0 : it->second.size();
    }

    //merges b into a
    void merge(Reg a, Reg b) {
        alias[b] = a;
        for (Reg n : adj[b]) {
            adj[n].erase(b);
            if (n != a) {
                adj[n].insert(a);
                adj[a].insert(n);
            }
        }
        adj.erase(b);
        cost[a] += cost[b];
        stats.coalesced++;
    }

    //two phases: every virtual pair first (briggs), then virtual into physical (george). merging a
    //value into a fixed register early would stop it merging with the other values it is copied to
    void coalesce() {
        coalescePhase(false);
        coalescePhase(true);
    }

    void coalescePhase(bool withPhysical) {
        bool merged = true;
        while (merged) {
            merged = false;
            for (const std::pair<Reg, Reg> &copy : copies) {
                Reg a = find(copy.first), b = find(copy.second);
                if (a == b || adj[a].count(b)) continue;
                if (noSpill.count(a) || noSpill.count(b)) continue;
                if (!isVirtual(a) && !isVirtual(b)) continue;
                if ((!isVirtual(a) || !isVirtual(b)) != withPhysical) continue;
                if (!isVirtual(a) || !isVirtual(b)) {
                    //george: the virtual one joins the physical one when none of its neighbours could
                    //be hurt: each is pre-coloured, of low degree, or already next to the physical one
                    Reg phys = isVirtual(a) ? b : a;
                    Reg virt = isVirtual(a) ? a : b;
                    bool safe = true;
                    for (Reg t : adj[virt]) {
                        if (!isVirtual(t) || degree(t) < kFor(t) || adj[t].count(phys)) continue;
                        safe = false;
                        break;
                    }
                    if (!safe) continue;
                    merge(phys, virt);
                    merged = true;
                    continue;
                }
                //briggs: fewer than k neighbours of significant degree after the merge
                std::set<Reg> neighbours = adj[a];
                neighbours.insert(adj[b].begin(), adj[b].end());
                size_t significant = 0;
                for (Reg n : neighbours) {
                    if (!isVirtual(n) || degree(n) >= kFor(n)) significant++;
                }
                if (significant >= kFor(a)) continue;
                merge(a, b);
                merged = true;
            }
        }
    }

    //simplify and select; the virtual nodes that found no register
    std::vector<Reg> colour() {
        colours.clear();
        std::set<Reg> remaining;
        for (const auto &entry : adj) {
            if (isVirtual(entry.first)) remaining.insert(entry.first);
        }
        std::vector<Reg> stack;
        while (!remaining.empty()) {
            Reg pick = Reg::RAX;
            bool found = false;
            for (Reg n : remaining) {
                size_t deg = 0;
                for (Reg m : adj[n]) deg += !isVirtual(m) || remaining.count(m);
                if (deg < kFor(n)) {
                    pick = n;
                    found = true;
                    break;
                }
            }
            if (!found) {
                double best = 0;
                for (Reg n : remaining) {
                    size_t deg = 0;
                    for (Reg m : adj[n]) deg += !isVirtual(m) || remaining.count(m);
                    double c = noSpill.count(n) ? 1e30 : cost[n] / (double)(deg + 1);
                    if (!found || c < best) {
                        pick = n;
                        best = c;
                        found = true;
                    }
                }
            }
            remaining.erase(pick);
            stack.push_back(pick);
        }
        std::vector<Reg> spills;
        while (!stack.empty()) {
            Reg n = stack.back();
            stack.pop_back();
            std::set<Reg> taken;
            for (Reg m : adj[n]) {
                if (!isVirtual(m)) {
                    taken.insert(m);
                    continue;
                }
                std::map<Reg, Reg>::iterator c = colours.find(m);
                if (c != colours.end()) taken.insert(c->second);
            }
            bool done = false;
            for (Reg r : isXmm(n) ? kFloatColors : kIntColors) {
                if (!taken.count(r)) {
                    colours[n] = r;
                    done = true;
                    break;
                }
            }
            if (!done) spills.push_back(n);
        }
        return spills;
    }

    Reg colourOf(Reg r) const {
        Reg root = find(r);
        if (!isVirtual(root)) return root;
        std::map<Reg, Reg>::const_iterator it = colours.find(root);
        return it == colours.end() ? root : it->second;
    }

    //a slot per spilled node; each instruction that reads one of its registers loads it into a new
    //short-lived register first, each that writes one stores it after
    void spill(const std::vector<Reg> &nodes) {
        std::map<Reg, long long> slotOf;
        for (Reg n : nodes) {
            slotOf[n] = frame.spillSlot();
            stats.spilled++;
        }
        MCode rewritten;
        std::vector<Reg> u, d;
        for (MInst inst : code) {
            readsAndWrites(inst, u, d);
            std::map<Reg, Reg> temp;  //spilled register -> its temporary in this instruction
            for (Reg r : u) {
                if (isVirtual(r) && slotOf.count(find(r)) && !temp.count(r)) temp[r] = fresh(isXmm(r));
            }
            for (Reg r : d) {
                if (isVirtual(r) && slotOf.count(find(r)) && !temp.count(r)) temp[r] = fresh(isXmm(r));
            }
            if (temp.empty()) {
                rewritten.push_back(inst);
                continue;
            }
            std::vector<MInst> after;
            for (const std::pair<const Reg, Reg> &t : temp) {
                Mem slot = Mem::frame(slotOf[find(t.first)]);
                std::string mov = isXmm(t.first) ? "movsd" : "movq";
                if (std::find(u.begin(), u.end(), t.first) != u.end()) {
                    MInst load;
                    load.op = mov;
                    load.operands = { memOp(slot), def(t.second) };
                    rewritten.push_back(load);
                }
                if (std::find(d.begin(), d.end(), t.first) != d.end()) {
                    MInst store;
                    store.op = mov;
                    store.operands = { use(t.second), memOp(slot) };
                    after.push_back(store);
                }
            }
            for (MOperand &o : inst.operands) {
                if (o.kind == MOperand::Register && temp.count(o.reg)) o.reg = temp[o.reg];
                if (o.kind == MOperand::Memory && o.mem.symbol.empty() && temp.count(o.mem.base)) {
                    o.mem.base = temp[o.mem.base];
                }
            }
            rewritten.push_back(inst);
            rewritten.insert(rewritten.end(), after.begin(), after.end());
        }
        code.swap(rewritten);
    }

    Reg fresh(bool isFloat) {
        Reg r = isFloat ? virtualFloat(nextFloat++) : virtualInt(nextInt++);
        noSpill.insert(r);
        return r;
    }

    void rewrite() {
        for (MInst &inst : code) {
            for (MOperand &o : inst.operands) {
                if (o.kind == MOperand::Register && isVirtual(o.reg)) {
                    o.reg = colourOf(o.reg);
                    if (isCalleeSaved(o.reg)) stats.calleeSavedUsed.insert(o.reg);
                }
                if (o.kind == MOperand::Memory && o.mem.symbol.empty() && isVirtual(o.mem.base)) {
                    o.mem.base = colourOf(o.mem.base);
                    if (isCalleeSaved(o.mem.base)) stats.calleeSavedUsed.insert(o.mem.base);
                }
            }
        }
    }

    MCode                                  &code;
    Frame                                  &frame;
    const std::map<std::string, unsigned>  &labelDepth;
    ColorStats                             &stats;

    std::map<Reg, Reg>               alias;
    std::map<Reg, std::set<Reg>>     adj;
    std::map<Reg, double>            cost;
    std::vector<std::pair<Reg, Reg>> copies;
    std::map<Reg, Reg>               colours;
    std::set<Reg>                    noSpill;
    unsigned                         nextInt = 500000;  //spill temporaries, far above isel's numbers
    unsigned                         nextFloat = 500000;
};

} // namespace

bool colorMachineCode(MCode &code, Frame &frame, const std::map<std::string, unsigned> &labelDepth,
                      ColorStats &stats, std::string &error) {
    Colorer colorer(code, frame, labelDepth, stats);
    return colorer.run(error);
}

} // namespace codegen
