#include "passes/dfa_dominators.h"

#include "passes/dfa_framework.h"

namespace passes {

Dominators::Dominators(const ir::Function &fn) : fn(fn) {
    std::vector<ir::BasicBlock *> rpo = reversePostorder(fn);
    if (rpo.empty()) return;

    //9.6.1: boundary OUT[ENTRY] = {}, every other OUT starts as the universe, gen = {B}
    DataflowProblem<const ir::BasicBlock *> problem;
    problem.direction = Direction::Forward;
    problem.meet = Meet::Intersection;
    for (ir::BasicBlock *block : rpo) {
        problem.top.insert(block);
        problem.gen[block].insert(block);
    }
    DataflowResult<const ir::BasicBlock *> result = solve(fn, problem);
    for (ir::BasicBlock *block : rpo) dom[block] = result.out[block];

    //idom(n): the strict dominator of n that every other strict dominator of n dominates, which is
    //the one with the most dominators itself
    for (ir::BasicBlock *block : rpo) {
        ir::BasicBlock *best = nullptr;
        size_t bestSize = 0;
        for (const ir::BasicBlock *d : dom[block]) {
            if (d == block) continue;
            if (best == nullptr || dom[d].size() > bestSize) {
                best = const_cast<ir::BasicBlock *>(d);
                bestSize = dom[d].size();
            }
        }
        immediate[block] = best;
    }

    //tree children in layout order, so walks do not depend on addresses
    for (ir::BasicBlock *block : fn.getBlocks()) {
        if (!isReachable(block)) continue;
        if (ir::BasicBlock *parent = immediate[block]) kids[parent].push_back(block);
    }
    std::vector<ir::BasicBlock *> stack(1, fn.getEntryBlock());
    while (!stack.empty()) {
        ir::BasicBlock *block = stack.back();
        stack.pop_back();
        order.push_back(block);
        const std::vector<ir::BasicBlock *> &c = children(block);
        for (size_t i = c.size(); i-- > 0;) stack.push_back(c[i]);
    }

    //dominance frontiers: from each predecessor of a join, walk up the dominator tree until the
    //join's idom; every block passed has the join in its frontier
    for (ir::BasicBlock *block : rpo) {
        std::vector<ir::BasicBlock *> preds;
        for (ir::BasicBlock *pred : block->predecessors) {
            if (isReachable(pred)) preds.push_back(pred);
        }
        if (preds.size() < 2) continue;
        for (ir::BasicBlock *pred : preds) {
            ir::BasicBlock *runner = pred;
            while (runner != nullptr && runner != immediate[block]) {
                df[runner].insert(block);
                runner = immediate[runner];
            }
        }
    }
}

bool Dominators::dominates(const ir::BasicBlock *a, const ir::BasicBlock *b) const {
    std::map<const ir::BasicBlock *, std::set<const ir::BasicBlock *>>::const_iterator it = dom.find(b);
    return it != dom.end() && it->second.count(a) != 0;
}

bool Dominators::dominatesUse(const ir::Instruction *def, const ir::Instruction *user,
                              const ir::BasicBlock *phiBlock) const {
    const ir::BasicBlock *defBlock = def->getParent();
    if (phiBlock != nullptr) return dominates(defBlock, phiBlock);
    const ir::BasicBlock *useBlock = user->getParent();
    if (defBlock != useBlock) return strictlyDominates(defBlock, useBlock);
    for (const std::unique_ptr<ir::Instruction> &inst : defBlock->instructions) {
        if (inst.get() == def) return true;
        if (inst.get() == user) return false;
    }
    return false;
}

ir::BasicBlock *Dominators::idom(const ir::BasicBlock *block) const {
    std::map<const ir::BasicBlock *, ir::BasicBlock *>::const_iterator it = immediate.find(block);
    return it == immediate.end() ? nullptr : it->second;
}

const std::vector<ir::BasicBlock *> &Dominators::children(const ir::BasicBlock *block) const {
    static const std::vector<ir::BasicBlock *> none;
    std::map<const ir::BasicBlock *, std::vector<ir::BasicBlock *>>::const_iterator it = kids.find(block);
    return it == kids.end() ? none : it->second;
}

const std::set<ir::BasicBlock *> &Dominators::frontier(const ir::BasicBlock *block) const {
    static const std::set<ir::BasicBlock *> none;
    std::map<const ir::BasicBlock *, std::set<ir::BasicBlock *>>::const_iterator it = df.find(block);
    return it == df.end() ? none : it->second;
}

void Dominators::dump(std::ostream &out) const {
    for (ir::BasicBlock *block : fn.getBlocks()) {
        if (!isReachable(block)) {
            out << block->getName() << ": unreachable\n";
            continue;
        }
        ir::BasicBlock *parent = idom(block);
        out << block->getName() << ": idom " << (parent ? parent->getName() : "-") << ", df {";
        //frontier in layout order
        bool first = true;
        for (ir::BasicBlock *f : fn.getBlocks()) {
            if (frontier(block).count(f) == 0) continue;
            out << (first ? "" : ", ") << f->getName();
            first = false;
        }
        out << "}\n";
    }
}

} // namespace passes
