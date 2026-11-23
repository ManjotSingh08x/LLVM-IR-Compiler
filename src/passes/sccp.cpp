#include "passes/sccp.h"

#include <map>
#include <set>

#include "passes/fold.h"
#include "passes/local_dag.h"
#include "passes/simplify_cfg.h"

namespace passes {

using namespace ir;

namespace {

//a value's lattice cell: TOP (no evidence yet), a constant, or BOTTOM (not a constant)
struct Cell {
    enum Kind { Top, Const, Bottom } kind = Top;
    const Constant *value = nullptr;

    static Cell bottom() { Cell c; c.kind = Bottom; return c; }
    static Cell of(const Constant *v) { Cell c; c.kind = Const; c.value = v; return c; }
};

bool sameConstant(const Constant *a, const Constant *b) {
    if (a->constKind == ConstantKind::Null || b->constKind == ConstantKind::Null) {
        return a->constKind == b->constKind;
    }
    return sameNumber(a, b);
}

bool sameCell(const Cell &a, const Cell &b) {
    return a.kind == b.kind && (a.kind != Cell::Const || sameConstant(a.value, b.value));
}

Cell meet(const Cell &a, const Cell &b) {
    if (a.kind == Cell::Top) return b;
    if (b.kind == Cell::Top) return a;
    if (a.kind == Cell::Bottom || b.kind == Cell::Bottom) return Cell::bottom();
    return sameConstant(a.value, b.value) ? a : Cell::bottom();
}

class Solver {
public:
    Solver(Function &fn, Module &module, const DefUse &defUse) : fn(fn), module(module), defUse(defUse) {}

    void solve() {
        markEdge(nullptr, fn.getEntryBlock());
        for (;;) {
            while (!edgeWork.empty() || !valueWork.empty()) {
                if (!edgeWork.empty()) {
                    std::pair<BasicBlock *, BasicBlock *> edge = edgeWork.back();
                    edgeWork.pop_back();
                    BasicBlock *block = edge.second;
                    bool first = executable.insert(block).second;
                    for (std::unique_ptr<Instruction> &inst : block->instructions) {
                        if (!first && inst->getOpcode() != Opcode::Phi) break;  //a new edge only changes phis
                        visit(inst.get());
                    }
                    continue;
                }
                Instruction *changed = valueWork.back();
                valueWork.pop_back();
                for (Instruction *user : defUse.users(changed)) {
                    if (executable.count(user->getParent())) visit(user);
                }
            }
            //a branch on a value still TOP would decide nothing: let it go both ways and go on
            bool resumed = false;
            for (BasicBlock *block : executable) {
                Value *cond = conditionOf(block->getTerminator());
                if (cond && cellOf(cond).kind == Cell::Top && cond->getKind() == ValueKind::Instruction) {
                    lower(static_cast<Instruction *>(cond), Cell::bottom());
                    resumed = true;
                }
            }
            if (!resumed) return;
        }
    }

    bool rewrite() {
        bool changed = false;
        //constants replace the values they stand for
        std::map<Value *, Value *> constants;
        for (BasicBlock *block : fn.getBlocks()) {
            if (!executable.count(block)) continue;
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                if (inst->getName().empty() || inst->hasSideEffects()) continue;
                Cell cell = cellOf(inst.get());
                if (cell.kind != Cell::Const) continue;
                constants[inst.get()] = materialize(cell.value, inst->getType());
            }
        }
        if (!constants.empty()) {
            for (BasicBlock *block : fn.getBlocks()) {
                for (std::unique_ptr<Instruction> &inst : block->instructions) {
                    for (Value *operand : inst->getOperands()) {
                        std::map<Value *, Value *>::iterator it = constants.find(operand);
                        if (it != constants.end()) inst->replaceOperand(operand, it->second);
                    }
                }
            }
            changed = true;
        }

        //branches on constants take their one edge
        for (BasicBlock *block : fn.getBlocks()) {
            if (!executable.count(block)) continue;
            Instruction *term = block->getTerminator();
            Value *cond = conditionOf(term);
            if (cond == nullptr) continue;
            const Constant *c = asConstant(cond);
            if (c == nullptr || c->constKind != ConstantKind::Int) continue;
            branchTo(block, takenTarget(term, c->intValue));
            changed = true;
        }
        fn.recomputeCFG();
        //what never became executable is unreachable now
        if (removeUnreachable(fn)) changed = true;
        if (removeUnusedValues(fn)) changed = true;
        return changed;
    }

private:
    static Value *conditionOf(Instruction *term) {
        if (term == nullptr) return nullptr;
        if (term->getOpcode() == Opcode::Br) {
            BranchInst *br = static_cast<BranchInst *>(term);
            return br->isConditional() ? br->condition : nullptr;
        }
        if (term->getOpcode() == Opcode::Switch) return static_cast<SwitchInst *>(term)->condition;
        return nullptr;
    }

    static BasicBlock *takenTarget(Instruction *term, long long value) {
        if (term->getOpcode() == Opcode::Br) {
            BranchInst *br = static_cast<BranchInst *>(term);
            return value != 0 ? br->target : br->falseTarget;
        }
        SwitchInst *sw = static_cast<SwitchInst *>(term);
        for (const std::pair<long long, BasicBlock *> &c : sw->cases) {
            if (c.first == value) return c.second;
        }
        return sw->defaultTarget;
    }

    Value *materialize(const Constant *c, const sema::Type *type) {
        if (c->constKind == ConstantKind::Null) return module.getNull(type);
        if (c->constKind == ConstantKind::Int) return module.getInt(type, c->intValue);
        return module.getFloat(type, c->floatValue);
    }

    Cell cellOf(const Value *value) const {
        if (value->getKind() == ValueKind::Constant) {
            const Constant *c = static_cast<const Constant *>(value);
            if (isNumber(c) || c->constKind == ConstantKind::Null) return Cell::of(c);
            return Cell::bottom();
        }
        if (value->getKind() != ValueKind::Instruction) return Cell::bottom();  //arguments, globals
        std::map<const Value *, Cell>::const_iterator it = cells.find(value);
        return it == cells.end() ? Cell() : it->second;
    }

    void lower(Instruction *inst, Cell next) {
        Cell now = cellOf(inst);
        next = meet(now, next);  //cells only go down: TOP -> constant -> BOTTOM
        if (sameCell(now, next)) return;
        cells[inst] = next;
        valueWork.push_back(inst);
    }

    void markEdge(BasicBlock *from, BasicBlock *to) {
        if (edges.insert(std::make_pair(from, to)).second) edgeWork.push_back(std::make_pair(from, to));
    }

    void visit(Instruction *inst) {
        BasicBlock *block = inst->getParent();
        switch (inst->getOpcode()) {
        case Opcode::Phi: {
            Cell result;
            for (const std::pair<Value *, BasicBlock *> &in : static_cast<PhiInst *>(inst)->incoming) {
                if (edges.count(std::make_pair(in.second, block))) result = meet(result, cellOf(in.first));
            }
            lower(inst, result);
            return;
        }
        case Opcode::Binary:
        case Opcode::Unary:
        case Opcode::Cast:
        case Opcode::Select: {
            std::vector<Value *> operands = inst->getOperands();
            if (inst->getOpcode() == Opcode::Select) {
                SelectInst *s = static_cast<SelectInst *>(inst);
                Cell cond = cellOf(s->condition);
                if (cond.kind == Cell::Top) return;
                if (cond.kind == Cell::Bottom) return lower(inst, meet(cellOf(s->ifTrue), cellOf(s->ifFalse)));
                return lower(inst, cellOf(cond.value->intValue ? s->ifTrue : s->ifFalse));
            }
            std::vector<const Constant *> constants;
            for (Value *operand : operands) {
                Cell c = cellOf(operand);
                if (c.kind == Cell::Bottom) return lower(inst, Cell::bottom());
                if (c.kind == Cell::Top) return;  //wait for the operand
                constants.push_back(c.value);
            }
            Value *folded = foldInstruction(module, *inst, constants);
            const Constant *c = folded ? asConstant(folded) : nullptr;
            return lower(inst, c ? Cell::of(c) : Cell::bottom());
        }
        case Opcode::Br: {
            BranchInst *br = static_cast<BranchInst *>(inst);
            if (!br->isConditional()) return markEdge(block, br->target);
            Cell cond = cellOf(br->condition);
            if (cond.kind == Cell::Top) return;
            if (cond.kind == Cell::Const && cond.value->constKind == ConstantKind::Int) {
                return markEdge(block, cond.value->intValue ? br->target : br->falseTarget);
            }
            markEdge(block, br->target);
            return markEdge(block, br->falseTarget);
        }
        case Opcode::Switch: {
            SwitchInst *sw = static_cast<SwitchInst *>(inst);
            Cell cond = cellOf(sw->condition);
            if (cond.kind == Cell::Top) return;
            if (cond.kind == Cell::Const && cond.value->constKind == ConstantKind::Int) {
                return markEdge(block, takenTarget(sw, cond.value->intValue));
            }
            for (BasicBlock *succ : sw->getSuccessors()) markEdge(block, succ);
            return;
        }
        default:
            //loads, calls, allocas and address arithmetic are never constants here
            if (!inst->getName().empty()) lower(inst, Cell::bottom());
            return;
        }
    }

    Function                                            &fn;
    Module                                              &module;
    const DefUse                                        &defUse;
    std::map<const Value *, Cell>                        cells;
    std::set<std::pair<BasicBlock *, BasicBlock *>>      edges;
    std::set<BasicBlock *>                               executable;
    std::vector<std::pair<BasicBlock *, BasicBlock *>>   edgeWork;
    std::vector<Instruction *>                           valueWork;
};

} // namespace

bool SCCP::run(Function &fn, Module &module, AnalysisManager &am) {
    Solver solver(fn, module, am.getDefUse(fn));
    solver.solve();
    return solver.rewrite();
}

} // namespace passes
