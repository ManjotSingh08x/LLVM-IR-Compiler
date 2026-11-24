//the call graph of a module: who calls whom, plus the facts inlining needs
#ifndef DATAFLOW_DFA_CALLGRAPH_H
#define DATAFLOW_DFA_CALLGRAPH_H

#include <map>
#include <ostream>
#include <vector>

#include "ir/cfg.h"

namespace passes {

class CallGraph {
public:
    struct CallSite {
        ir::CallInst *call;
        ir::Function *callee;  //null for an indirect call
    };

    struct Node {
        ir::Function            *fn = nullptr;
        std::vector<CallSite>    calls;
        std::vector<ir::Function *> callees;
        std::vector<ir::Function *> callers;
        size_t                   callSiteCount = 0;  //direct calls of fn from anywhere
        size_t                   size = 0;           //instructions in fn
        bool                     hasIndirectCall = false;
        bool                     addressTaken = false;  //used other than as a direct callee
        bool                     recursive = false;     //in a cycle of the call graph
        size_t                   scc = 0;
    };

    explicit CallGraph(const ir::Module &module);

    const Node &node(const ir::Function *fn) const;

    //strongly connected components, callees before callers
    const std::vector<std::vector<ir::Function *>> &sccs() const { return components; }

    std::vector<ir::Function *> bottomUp() const;

    void dump(std::ostream &out) const;

private:
    void strongConnect(ir::Function *fn);

    const ir::Module                        &module;
    std::map<const ir::Function *, Node>     nodes;
    std::vector<std::vector<ir::Function *>> components;

    std::map<const ir::Function *, size_t> indexOf;
    std::map<const ir::Function *, size_t> lowLink;
    std::vector<ir::Function *>            stack;
    std::map<const ir::Function *, bool>   onStack;
    size_t                                 nextIndex = 0;
};

} // namespace passes

#endif
