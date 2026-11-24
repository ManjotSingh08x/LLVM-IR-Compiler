#include "dataflow/dfa_framework.h"

namespace passes {

std::vector<ir::BasicBlock *> reversePostorder(const ir::Function &fn) {
    std::vector<ir::BasicBlock *> postorder;
    ir::BasicBlock *entry = fn.getEntryBlock();
    if (entry == nullptr) return postorder;

    std::set<const ir::BasicBlock *> visited;
    std::vector<std::pair<ir::BasicBlock *, size_t>> stack;
    visited.insert(entry);
    stack.push_back(std::make_pair(entry, 0));
    while (!stack.empty()) {
        ir::BasicBlock *block = stack.back().first;
        size_t &next = stack.back().second;
        if (next < block->successors.size()) {
            ir::BasicBlock *succ = block->successors[next++];
            if (visited.insert(succ).second) stack.push_back(std::make_pair(succ, 0));
            continue;
        }
        postorder.push_back(block);
        stack.pop_back();
    }
    std::reverse(postorder.begin(), postorder.end());
    return postorder;
}

} // namespace passes
