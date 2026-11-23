#ifndef PASSES_DFA_CALLGRAPH_H
#define PASSES_DFA_CALLGRAPH_H

#include <map>
#include <ostream>
#include <vector>

#include "ir/cfg.h"

namespace passes {

//the call graph of a module (dragon 12.1.1): who calls whom, plus the simple facts about each
//function that interprocedural passes such as inlining decide on
class CallGraph {
public:
    struct CallSite {
        ir::CallInst *call;
        ir::Function *callee;  //null for an indirect call (a function pointer or a virtual call)
    };

    struct Node {
        ir::Function            *fn = nullptr;
        std::vector<CallSite>    calls;          //every call in fn, in block order
        std::vector<ir::Function *> callees;     //distinct direct callees, first call first
        std::vector<ir::Function *> callers;     //distinct functions with a direct call to fn
        size_t                   callSiteCount = 0;  //direct calls of fn from anywhere
        size_t                   size = 0;           //instructions in fn
        bool                     hasIndirectCall = false;
        bool                     addressTaken = false;  //used other than as a direct callee
        bool                     recursive = false;     //in a cycle of the call graph
        size_t                   scc = 0;               //index into sccs()
    };

    explicit CallGraph(const ir::Module &module);

    const Node &node(const ir::Function *fn) const;

    //the strongly connected components, callees before callers (bottom-up)
    const std::vector<std::vector<ir::Function *>> &sccs() const { return components; }

    //every function in bottom-up order, the order inlining visits them
    std::vector<ir::Function *> bottomUp() const;

    //readable form, checked by the regression tests
    void dump(std::ostream &out) const;

private:
    void strongConnect(ir::Function *fn);

    const ir::Module                        &module;
    std::map<const ir::Function *, Node>     nodes;
    std::vector<std::vector<ir::Function *>> components;

    //tarjan's algorithm state
    std::map<const ir::Function *, size_t> indexOf;
    std::map<const ir::Function *, size_t> lowLink;
    std::vector<ir::Function *>            stack;
    std::map<const ir::Function *, bool>   onStack;
    size_t                                 nextIndex = 0;
};

} // namespace passes

#endif
