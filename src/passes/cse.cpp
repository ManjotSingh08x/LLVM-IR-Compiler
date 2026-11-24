#include "passes/cse.h"

#include <map>

#include "dataflow/dfa_available_exprs.h"
#include "passes/fold.h"
#include "passes/local_dag.h"

namespace passes {

using namespace ir;

namespace {

class Eliminator {
public:
    Eliminator(const Dominators &doms, const AvailableExpressions &available) : doms(doms), available(available) {}

    void walk(BasicBlock *block, std::map<std::string, Instruction *> table) {
        const std::set<std::string> &availableIn = available.in(block);
        std::set<std::string> local;
        for (std::unique_ptr<Instruction> &owned : block->instructions) {
            Instruction *inst = owned.get();
            for (Value *operand : inst->getOperands()) {
                std::map<Value *, Value *>::iterator it = replaced.find(operand);
                if (it != replaced.end()) inst->replaceOperand(operand, it->second);
            }
            if (!isExpression(*inst)) continue;
            std::string key = expressionKey(*inst);
            std::map<std::string, Instruction *>::iterator earlier = table.find(key);
            if (earlier != table.end() && (availableIn.count(key) || local.count(key))) {
                replaced[inst] = earlier->second;
                continue;
            }
            table[key] = inst;
            local.insert(key);
        }
        for (BasicBlock *child : doms.children(block)) walk(child, table);
    }

    std::map<Value *, Value *> replaced;

private:
    const Dominators            &doms;
    const AvailableExpressions  &available;
};

}

bool CSE::run(Function &fn, Module &module, AnalysisManager &am) {
    (void)module;
    bool changed = false;
    for (int round = 0; round < 8; round++) {
        AvailableExpressions available(fn);
        Eliminator eliminator(am.getDominators(fn), available);
        eliminator.walk(fn.getEntryBlock(), std::map<std::string, Instruction *>());
        if (eliminator.replaced.empty()) break;
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                for (Value *operand : inst->getOperands()) {
                    std::map<Value *, Value *>::iterator it = eliminator.replaced.find(operand);
                    if (it != eliminator.replaced.end()) inst->replaceOperand(operand, it->second);
                }
            }
        }
        removeUnusedValues(fn);
        changed = true;
    }
    return changed;
}

}
