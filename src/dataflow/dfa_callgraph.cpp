#include "dataflow/dfa_callgraph.h"

#include <algorithm>

namespace passes {

namespace {

void markAddresses(const ir::Constant *constant, std::map<const ir::Function *, bool> &taken) {
    if (constant == nullptr) return;
    if (constant->constKind == ir::ConstantKind::Address && constant->target != nullptr &&
        constant->target->getKind() == ir::ValueKind::Function) {
        taken[static_cast<const ir::Function *>(constant->target)] = true;
    }
    for (const ir::Constant *element : constant->elements) markAddresses(element, taken);
}

void addOnce(std::vector<ir::Function *> &list, ir::Function *fn) {
    if (std::find(list.begin(), list.end(), fn) == list.end()) list.push_back(fn);
}

}

CallGraph::CallGraph(const ir::Module &module) : module(module) {
    std::map<const ir::Function *, bool> taken;
    for (const std::unique_ptr<ir::GlobalVariable> &global : module.getGlobals()) {
        markAddresses(global->initializer, taken);
    }
    if (module.globalInit != nullptr) taken[module.globalInit] = true;

    for (const std::unique_ptr<ir::Function> &owned : module.getFunctions()) {
        Node &node = nodes[owned.get()];
        node.fn = owned.get();
    }

    for (const std::unique_ptr<ir::Function> &owned : module.getFunctions()) {
        ir::Function *fn = owned.get();
        Node &node = nodes[fn];
        for (const ir::BasicBlock *block : fn->getBlocks()) {
            node.size += block->instructions.size();
            for (const std::unique_ptr<ir::Instruction> &inst : block->instructions) {
                std::vector<ir::Value *> operands = inst->getOperands();
                size_t first = 0;
                if (inst->getOpcode() == ir::Opcode::Call) {
                    ir::CallInst *call = static_cast<ir::CallInst *>(inst.get());
                    ir::Function *callee = nullptr;
                    if (call->callee->getKind() == ir::ValueKind::Function) {
                        callee = static_cast<ir::Function *>(call->callee);
                    }
                    node.calls.push_back(CallSite{ call, callee });
                    if (callee == nullptr) {
                        node.hasIndirectCall = true;
                    } else {
                        addOnce(node.callees, callee);
                        Node &target = nodes[callee];
                        addOnce(target.callers, fn);
                        target.callSiteCount++;
                    }
                    first = 1;
                }
                for (size_t i = first; i < operands.size(); i++) {
                    if (operands[i] != nullptr && operands[i]->getKind() == ir::ValueKind::Function) {
                        taken[static_cast<const ir::Function *>(operands[i])] = true;
                    }
                }
            }
        }
    }
    for (std::pair<const ir::Function *const, Node> &entry : nodes) {
        entry.second.addressTaken = taken.count(entry.first) != 0;
    }

    for (const std::unique_ptr<ir::Function> &owned : module.getFunctions()) {
        if (indexOf.count(owned.get()) == 0) strongConnect(owned.get());
    }
    for (size_t i = 0; i < components.size(); i++) {
        for (ir::Function *fn : components[i]) {
            Node &node = nodes[fn];
            node.scc = i;
            bool callsItself = std::find(node.callees.begin(), node.callees.end(), fn) != node.callees.end();
            node.recursive = components[i].size() > 1 || callsItself;
        }
    }
}

void CallGraph::strongConnect(ir::Function *fn) {
    indexOf[fn] = nextIndex;
    lowLink[fn] = nextIndex;
    nextIndex++;
    stack.push_back(fn);
    onStack[fn] = true;

    for (ir::Function *callee : nodes[fn].callees) {
        if (indexOf.count(callee) == 0) {
            strongConnect(callee);
            lowLink[fn] = std::min(lowLink[fn], lowLink[callee]);
        } else if (onStack[callee]) {
            lowLink[fn] = std::min(lowLink[fn], indexOf[callee]);
        }
    }

    if (lowLink[fn] != indexOf[fn]) return;
    std::vector<ir::Function *> component;
    while (true) {
        ir::Function *member = stack.back();
        stack.pop_back();
        onStack[member] = false;
        component.push_back(member);
        if (member == fn) break;
    }
    std::vector<ir::Function *> ordered;
    for (const std::unique_ptr<ir::Function> &owned : module.getFunctions()) {
        if (std::find(component.begin(), component.end(), owned.get()) != component.end()) {
            ordered.push_back(owned.get());
        }
    }
    components.push_back(ordered);
}

const CallGraph::Node &CallGraph::node(const ir::Function *fn) const {
    return nodes.at(fn);
}

std::vector<ir::Function *> CallGraph::bottomUp() const {
    std::vector<ir::Function *> order;
    for (const std::vector<ir::Function *> &component : components) {
        order.insert(order.end(), component.begin(), component.end());
    }
    return order;
}

void CallGraph::dump(std::ostream &out) const {
    for (const std::unique_ptr<ir::Function> &owned : module.getFunctions()) {
        const Node &n = nodes.at(owned.get());
        out << "function @" << owned->getName() << ": "
            << (owned->isDefinition() ? "definition" : "declaration")
            << (owned->fnType->isVariadic ? ", variadic" : "")
            << ", size " << n.size << ", call sites " << n.callSiteCount
            << (n.recursive ? ", recursive" : "") << (n.addressTaken ? ", address taken" : "")
            << (n.hasIndirectCall ? ", has indirect call" : "") << "\n";
        out << "  calls:";
        if (n.callees.empty()) out << " none";
        for (const ir::Function *callee : n.callees) out << " @" << callee->getName();
        out << "\n";
        out << "  called by:";
        if (n.callers.empty()) out << " none";
        for (const ir::Function *caller : n.callers) out << " @" << caller->getName();
        out << "\n";
    }
    out << "bottom-up:\n";
    for (size_t i = 0; i < components.size(); i++) {
        out << "  scc " << i << ":";
        for (const ir::Function *fn : components[i]) out << " @" << fn->getName();
        bool recursive = !components[i].empty() && nodes.at(components[i].front()).recursive;
        out << (recursive ? " (recursive)" : "") << "\n";
    }
}

}
