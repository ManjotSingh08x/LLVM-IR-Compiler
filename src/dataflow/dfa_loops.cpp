#include "dataflow/dfa_loops.h"

#include <algorithm>
#include <map>

namespace passes {

std::vector<ir::BasicBlock *> Loop::outsidePredecessors() const {
    std::vector<ir::BasicBlock *> preds;
    for (ir::BasicBlock *pred : header->predecessors) {
        if (!contains(pred)) preds.push_back(pred);
    }
    return preds;
}

ir::BasicBlock *Loop::preheader() const {
    std::vector<ir::BasicBlock *> preds = outsidePredecessors();
    if (preds.size() != 1) return nullptr;
    if (preds[0]->successors.size() != 1) return nullptr;
    return preds[0];
}

LoopInfo::LoopInfo(const ir::Function &fn, const Dominators &doms) : fn(fn) {
    std::map<ir::BasicBlock *, Loop *> byHeader;
    for (ir::BasicBlock *tail : fn.getBlocks()) {
        if (!doms.isReachable(tail)) continue;
        for (ir::BasicBlock *head : tail->successors) {
            if (!doms.dominates(head, tail)) continue;
            Loop *&loop = byHeader[head];
            if (loop == nullptr) {
                loops.push_back(std::unique_ptr<Loop>(new Loop()));
                loop = loops.back().get();
                loop->header = head;
                loop->blocks.insert(head);
            }
            loop->latches.push_back(tail);
            std::vector<ir::BasicBlock *> work;
            if (loop->blocks.insert(tail).second) work.push_back(tail);
            while (!work.empty()) {
                ir::BasicBlock *block = work.back();
                work.pop_back();
                for (ir::BasicBlock *pred : block->predecessors) {
                    if (doms.isReachable(pred) && loop->blocks.insert(pred).second) work.push_back(pred);
                }
            }
        }
    }

    for (std::unique_ptr<Loop> &loop : loops) {
        std::vector<ir::BasicBlock *> latches;
        std::set<ir::BasicBlock *> exits;
        for (ir::BasicBlock *block : fn.getBlocks()) {
            if (std::find(loop->latches.begin(), loop->latches.end(), block) != loop->latches.end()) {
                latches.push_back(block);
            }
            if (!loop->contains(block)) continue;
            for (ir::BasicBlock *next : block->successors) {
                if (!loop->contains(next)) exits.insert(next);
            }
        }
        loop->latches = latches;
        for (ir::BasicBlock *block : fn.getBlocks()) {
            if (exits.count(block)) loop->exits.push_back(block);
        }
    }

    for (std::unique_ptr<Loop> &loop : loops) {
        Loop *best = nullptr;
        for (std::unique_ptr<Loop> &other : loops) {
            if (other.get() == loop.get() || !other->contains(loop->header)) continue;
            if (other->blocks.size() <= loop->blocks.size()) continue;
            if (best == nullptr || other->blocks.size() < best->blocks.size()) best = other.get();
        }
        loop->parent = best;
    }
    std::vector<Loop *> roots;
    for (std::unique_ptr<Loop> &loop : loops) {
        if (loop->parent) {
            loop->parent->children.push_back(loop.get());
        } else {
            roots.push_back(loop.get());
        }
    }

    std::vector<std::pair<Loop *, bool>> stack;
    for (size_t i = roots.size(); i-- > 0;) stack.push_back(std::make_pair(roots[i], false));
    while (!stack.empty()) {
        Loop *loop = stack.back().first;
        bool expanded = stack.back().second;
        stack.pop_back();
        if (expanded) {
            postorder.push_back(loop);
            continue;
        }
        loop->depth = loop->parent ? loop->parent->depth + 1 : 1;
        stack.push_back(std::make_pair(loop, true));
        for (size_t i = loop->children.size(); i-- > 0;) stack.push_back(std::make_pair(loop->children[i], false));
    }
}

Loop *LoopInfo::loopFor(const ir::BasicBlock *block) const {
    for (Loop *loop : postorder) {
        if (loop->contains(block)) return loop;
    }
    return nullptr;
}

unsigned LoopInfo::depth(const ir::BasicBlock *block) const {
    Loop *loop = loopFor(block);
    return loop ? loop->depth : 0;
}

void LoopInfo::dump(std::ostream &out) const {
    for (Loop *loop : postorder) {
        out << "loop " << loop->header->getName() << " depth " << loop->depth << ":";
        for (ir::BasicBlock *block : fn.getBlocks()) {
            if (loop->contains(block)) out << " " << block->getName();
        }
        out << "; latches";
        for (ir::BasicBlock *block : loop->latches) out << " " << block->getName();
        out << "; exits";
        for (ir::BasicBlock *block : loop->exits) out << " " << block->getName();
        out << "\n";
    }
}

}
