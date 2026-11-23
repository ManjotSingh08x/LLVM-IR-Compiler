#ifndef PASSES_DFA_FRAMEWORK_H
#define PASSES_DFA_FRAMEWORK_H

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <set>
#include <vector>

#include "ir/cfg.h"

//the iterative data-flow framework of the dragon book (9.3), shared by every dfa_ analysis
namespace passes {

//the blocks entry reaches, in reverse postorder: every block comes before its successors, back edges aside
std::vector<ir::BasicBlock *> reversePostorder(const ir::Function &fn);

enum class Direction { Forward, Backward };
enum class Meet { Union, Intersection };

//one problem in gen/kill form: f(x) = gen U (x - kill)
template <typename T>
struct DataflowProblem {
    using Set = std::set<T>;

    Direction direction = Direction::Forward;
    Meet      meet = Meet::Union;

    //what flows in from the dragon book's ENTRY (forward) or EXIT (backward) node
    Set boundary;
    //the starting value of every block: empty for union, the whole universe for intersection
    Set top;

    std::map<const ir::BasicBlock *, Set> gen;
    std::map<const ir::BasicBlock *, Set> kill;

    //optional: changes the value carried along the edge from -> to before the meet, phis need it
    std::function<void(const ir::BasicBlock *from, const ir::BasicBlock *to, Set &value)> edge;
};

//IN and OUT of every block, in the direction of the cfg whatever the problem's direction
template <typename T>
struct DataflowResult {
    using Set = std::set<T>;

    std::map<const ir::BasicBlock *, Set> in;
    std::map<const ir::BasicBlock *, Set> out;
    unsigned                              iterations = 0;  //rounds until nothing changed
};

namespace detail {

template <typename T>
const std::set<T> &lookup(const std::map<const ir::BasicBlock *, std::set<T>> &sets, const ir::BasicBlock *block) {
    static const std::set<T> empty;
    typename std::map<const ir::BasicBlock *, std::set<T>>::const_iterator it = sets.find(block);
    return it == sets.end() ? empty : it->second;
}

template <typename T>
void meetInto(std::set<T> &acc, const std::set<T> &value, Meet meet) {
    if (meet == Meet::Union) {
        acc.insert(value.begin(), value.end());
        return;
    }
    std::set<T> both;
    std::set_intersection(acc.begin(), acc.end(), value.begin(), value.end(), std::inserter(both, both.end()));
    acc.swap(both);
}

} // namespace detail

//round-robin iteration until no OUT (forward) or IN (backward) changes
//forward visits blocks in reverse postorder, backward in postorder, so most values settle in one round
template <typename T>
DataflowResult<T> solve(const ir::Function &fn, const DataflowProblem<T> &problem) {
    using Set = std::set<T>;
    const bool forward = problem.direction == Direction::Forward;

    std::vector<ir::BasicBlock *> order = reversePostorder(fn);
    if (!forward) std::reverse(order.begin(), order.end());
    std::set<const ir::BasicBlock *> reachable(order.begin(), order.end());

    DataflowResult<T> result;
    for (ir::BasicBlock *block : order) {
        result.in[block] = problem.top;
        result.out[block] = problem.top;
    }

    bool changed = true;
    while (changed) {
        changed = false;
        result.iterations++;
        for (ir::BasicBlock *block : order) {
            //meet over the predecessors (forward) or successors (backward)
            Set  merged;
            bool first = true;
            auto combine = [&](const Set &value) {
                if (first) {
                    merged = value;
                    first = false;
                } else {
                    detail::meetInto(merged, value, problem.meet);
                }
            };

            bool atBoundary = forward ? block == fn.getEntryBlock() : block->successors.empty();
            if (atBoundary) combine(problem.boundary);

            const std::vector<ir::BasicBlock *> &neighbours = forward ? block->predecessors : block->successors;
            for (ir::BasicBlock *other : neighbours) {
                if (!reachable.count(other)) continue;
                Set value = forward ? result.out[other] : result.in[other];
                if (problem.edge) {
                    if (forward) {
                        problem.edge(other, block, value);
                    } else {
                        problem.edge(block, other, value);
                    }
                }
                combine(value);
            }
            if (first) merged = problem.top;

            //transfer: gen U (merged - kill)
            const Set &kill = detail::lookup(problem.kill, block);
            const Set &gen = detail::lookup(problem.gen, block);
            Set transferred;
            std::set_difference(merged.begin(), merged.end(), kill.begin(), kill.end(),
                                std::inserter(transferred, transferred.end()));
            transferred.insert(gen.begin(), gen.end());

            Set &meetSide = forward ? result.in[block] : result.out[block];
            Set &transferSide = forward ? result.out[block] : result.in[block];
            meetSide.swap(merged);
            if (transferred != transferSide) {
                transferSide.swap(transferred);
                changed = true;
            }
        }
    }
    return result;
}

} // namespace passes

#endif
