#include "passes/const_prop_dense.h"

#include <map>

#include "passes/dfa_framework.h"
#include "passes/escape.h"
#include "passes/fold.h"
#include "passes/local_dag.h"

namespace passes {

using namespace ir;

namespace {

//one variable's value at a point (9.4.1)
struct Lattice {
    enum Kind { Undef, Const, NAC } kind = Undef;
    const Constant *value = nullptr;

    static Lattice nac() { Lattice l; l.kind = NAC; return l; }
    static Lattice of(const Constant *c) { Lattice l; l.kind = Const; l.value = c; return l; }

    bool operator==(const Lattice &other) const {
        if (kind != other.kind) return false;
        return kind != Const || sameNumber(value, other.value);
    }
};

Lattice meet(const Lattice &a, const Lattice &b) {
    if (a.kind == Lattice::Undef) return b;
    if (b.kind == Lattice::Undef) return a;
    if (a.kind == Lattice::NAC || b.kind == Lattice::NAC) return Lattice::nac();
    return sameNumber(a.value, b.value) ? a : Lattice::nac();
}

//a missing variable is UNDEF
using Env = std::map<const Value *, Lattice>;

Lattice lookup(const Env &env, const Value *var) {
    Env::const_iterator it = env.find(var);
    return it == env.end() ? Lattice() : it->second;
}

bool sameEnv(const Env &a, const Env &b) {
    for (const auto &entry : a) {
        if (!(lookup(b, entry.first) == entry.second)) return false;
    }
    for (const auto &entry : b) {
        if (!(lookup(a, entry.first) == entry.second)) return false;
    }
    return true;
}

class Solver {
public:
    Solver(Function &fn, Module &module) : fn(fn), module(module), escape(fn) {
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                if (inst->getOpcode() != Opcode::Alloca || !escape.isPromotable(inst.get())) continue;
                const sema::Type *type = static_cast<AllocaInst *>(inst.get())->getAccessType();
                if (sema::isIntegral(type) || sema::isFloating(type)) tracked.insert(inst.get());
            }
        }
    }

    bool trackedVariable(const Value *address) const { return tracked.count(address) != 0; }

    //runs the block from in; when loadValues is given, records what each tracked load reads
    Env transfer(BasicBlock *block, Env env, std::map<Instruction *, Lattice> *loadValues) {
        std::map<const Value *, Lattice> temps;
        auto eval = [&](const Value *value) {
            if (isNumber(value)) return Lattice::of(static_cast<const Constant *>(value));
            std::map<const Value *, Lattice>::iterator it = temps.find(value);
            return it == temps.end() ? Lattice::nac() : it->second;
        };
        for (std::unique_ptr<Instruction> &owned : block->instructions) {
            Instruction *inst = owned.get();
            switch (inst->getOpcode()) {
            case Opcode::Load: {
                const Value *address = static_cast<LoadInst *>(inst)->address;
                if (!trackedVariable(address)) break;
                temps[inst] = lookup(env, address);
                if (loadValues) (*loadValues)[inst] = temps[inst];
                break;
            }
            case Opcode::Store: {
                StoreInst *store = static_cast<StoreInst *>(inst);
                if (trackedVariable(store->address)) env[store->address] = eval(store->value);
                break;
            }
            case Opcode::Binary:
            case Opcode::Unary:
            case Opcode::Cast:
            case Opcode::Select: {
                std::vector<const Constant *> constants;
                bool anyNac = false, anyUndef = false;
                for (Value *operand : inst->getOperands()) {
                    Lattice l = eval(operand);
                    anyNac = anyNac || l.kind == Lattice::NAC;
                    anyUndef = anyUndef || l.kind == Lattice::Undef;
                    constants.push_back(l.kind == Lattice::Const ? l.value : nullptr);
                }
                Value *folded = (anyNac || anyUndef) && inst->getOpcode() != Opcode::Select
                                    ? nullptr : foldInstruction(module, *inst, constants);
                if (folded) {
                    temps[inst] = eval(folded);
                } else if (anyNac || !anyUndef) {
                    temps[inst] = Lattice::nac();
                } else {
                    temps[inst] = Lattice();
                }
                break;
            }
            default:
                break;
            }
        }
        return env;
    }

    bool solveAndRewrite() {
        if (tracked.empty()) return false;
        std::vector<BasicBlock *> order = reversePostorder(fn);
        std::map<const BasicBlock *, Env> in, out;
        std::set<const BasicBlock *> reachable(order.begin(), order.end());
        bool changed = true;
        while (changed) {
            changed = false;
            for (BasicBlock *block : order) {
                Env merged;  //entry: every variable UNDEF
                if (block != fn.getEntryBlock()) {
                    bool first = true;
                    for (BasicBlock *pred : block->predecessors) {
                        if (!reachable.count(pred)) continue;
                        if (first) {
                            merged = out[pred];
                            first = false;
                            continue;
                        }
                        Env next;
                        for (const Value *var : tracked) next[var] = meet(lookup(merged, var), lookup(out[pred], var));
                        merged.swap(next);
                    }
                }
                in[block] = merged;
                Env result = transfer(block, merged, nullptr);
                if (!sameEnv(result, out[block])) {
                    out[block] = result;
                    changed = true;
                }
            }
        }

        //every load of a variable known to be a constant there becomes that constant
        std::map<Value *, Value *> replaced;
        for (BasicBlock *block : order) {
            std::map<Instruction *, Lattice> loads;
            transfer(block, in[block], &loads);
            for (const auto &entry : loads) {
                if (entry.second.kind != Lattice::Const) continue;
                const Constant *c = entry.second.value;
                const sema::Type *type = entry.first->getType();
                replaced[entry.first] = c->constKind == ConstantKind::Int ? module.getInt(type, c->intValue)
                                                                          : module.getFloat(type, c->floatValue);
            }
        }
        if (replaced.empty()) return false;
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                for (Value *operand : inst->getOperands()) {
                    std::map<Value *, Value *>::iterator it = replaced.find(operand);
                    if (it != replaced.end()) inst->replaceOperand(operand, it->second);
                }
            }
        }
        removeUnusedValues(fn);
        return true;
    }

private:
    Function                   &fn;
    Module                     &module;
    EscapeInfo                  escape;
    std::set<const Value *>     tracked;
};

} // namespace

bool ConstPropDense::run(Function &fn, Module &module, AnalysisManager &am) {
    (void)am;
    Solver solver(fn, module);
    return solver.solveAndRewrite();
}

} // namespace passes
