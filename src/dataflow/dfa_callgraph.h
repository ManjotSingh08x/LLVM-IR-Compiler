#ifndef DATAFLOW_DFA_CALLGRAPH_H
#define DATAFLOW_DFA_CALLGRAPH_H

#include <map>
#include <ostream>
#include <vector>

#include "ir/cfg.h"

namespace passes {

//records which functions call which others
class CallGraph {
public:
    struct CallSite {
        ir::CallInst *call;
        ir::Function *callee;
    };

    struct Node {
        ir::Function            *fn = nullptr;
        std::vector<CallSite>    calls;
        std::vector<ir::Function *> callees;
        std::vector<ir::Function *> callers;
        size_t                   callSiteCount = 0;
        size_t                   size = 0;
        bool                     hasIndirectCall = false;
        bool                     addressTaken = false;
        bool                     recursive = false;
        size_t                   scc = 0;
    };

    explicit CallGraph(const ir::Module &module);

    const Node &node(const ir::Function *fn) const;

    const std::vector<std::vector<ir::Function *>> &sccs() const { return components; }

    //orders functions so callees come before callers
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

}

#endif
