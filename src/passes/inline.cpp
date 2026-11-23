#include "passes/inline.h"

#include "passes/clone.h"

namespace passes {

using namespace ir;

namespace {

size_t sizeOf(const Function &fn) {
    size_t n = 0;
    for (const BasicBlock *block : fn.getBlocks()) n += block->instructions.size();
    return n;
}

//replaces every use of from in fn with to
void replaceUses(Function &fn, Value *from, Value *to) {
    for (BasicBlock *block : fn.getBlocks()) {
        for (std::unique_ptr<Instruction> &inst : block->instructions) inst->replaceOperand(from, to);
    }
}

//copies callee into caller in place of call
void inlineCall(Module &module, Function &caller, CallInst *call, Function &callee) {
    BasicBlock *before = call->getParent();
    BasicBlock *entry = caller.getEntryBlock();
    BasicBlock *after = before->splitAt(call, callee.getName() + ".exit");

    ValueMap map;
    for (size_t i = 0; i < callee.args.size(); i++) map[callee.args[i].get()] = call->args[i];

    //the callee's blocks, in order, between the two halves
    std::vector<BasicBlock *> copies;
    BasicBlock *last = before;
    for (BasicBlock *block : callee.getBlocks()) {
        BasicBlock *copy = caller.createBlock(block->getName());
        caller.placeBlockAfter(copy, last);
        last = copy;
        map[block] = copy;
        copies.push_back(copy);
    }

    //instructions: allocas go to the caller's entry block, the rest to the matching copy.
    //operands are mapped once everything exists, a phi may name a later value
    std::vector<Instruction *> cloned;
    std::vector<std::pair<ReturnInst *, BasicBlock *>> returns;
    const std::vector<BasicBlock *> &blocks = callee.getBlocks();
    for (size_t b = 0; b < blocks.size(); b++) {
        for (std::unique_ptr<Instruction> &inst : blocks[b]->instructions) {
            std::string name = inst->getName().empty() ? "" : caller.uniqueName(inst->getName());
            std::unique_ptr<Instruction> copy = cloneInstruction(*inst, name);
            map[inst.get()] = copy.get();
            cloned.push_back(copy.get());
            if (copy->getOpcode() == Opcode::Ret) returns.push_back(std::make_pair(static_cast<ReturnInst *>(copy.get()), copies[b]));
            if (inst->getOpcode() == Opcode::Alloca) {
                entry->insertAlloca(std::move(copy));
            } else {
                copies[b]->append(std::move(copy));
            }
        }
    }
    for (Instruction *inst : cloned) remapInstruction(*inst, map);

    //the result: the single returned value, or a slot every ret stores to
    Value *result = nullptr;
    const sema::Type *returnType = callee.fnType->returnType;
    bool hasValue = !returnType->is(sema::TypeKind::Void);
    AllocaInst *slot = nullptr;
    if (hasValue && returns.size() == 1) {
        result = returns.front().first->returnValue;
    } else if (hasValue && returns.size() > 1) {
        slot = new AllocaInst(returnType, module.pointerType(), caller.uniqueName(callee.getName() + ".ret"));
        entry->insertAlloca(std::unique_ptr<Instruction>(slot));
    }
    for (std::pair<ReturnInst *, BasicBlock *> &ret : returns) {
        BasicBlock *block = ret.second;
        if (slot && ret.first->returnValue) {
            block->insertBefore(ret.first, std::unique_ptr<Instruction>(new StoreInst(returnType, ret.first->returnValue, slot)));
        }
        block->erase(ret.first);
        block->append(std::unique_ptr<Instruction>(new BranchInst(after)));
    }
    if (slot) {
        LoadInst *load = new LoadInst(returnType, slot, caller.uniqueName(callee.getName() + ".result"));
        after->insertBefore(call, std::unique_ptr<Instruction>(load));
        result = load;
    }

    //the call goes, its uses take the result. a callee that never returns leaves the code after the
    //call unreachable, its uses get a zero until simplify_cfg deletes that code
    if (hasValue && result == nullptr) result = module.getZero(returnType);
    if (hasValue) replaceUses(caller, call, result);
    after->erase(call);
    before->append(std::unique_ptr<Instruction>(new BranchInst(copies.front())));
    caller.recomputeCFG();
}

} // namespace

bool Inliner::run(Module &module, AnalysisManager &am) {
    std::vector<Function *> order = am.getCallGraph(module).bottomUp();
    bool changed = false;
    for (Function *caller : order) {
        if (!caller->isDefinition()) continue;
        //the call graph is rebuilt after each caller, inlining changes call counts and sizes
        const CallGraph &graph = am.getCallGraph(module);
        std::vector<CallGraph::CallSite> sites = graph.node(caller).calls;
        bool callerChanged = false;
        for (const CallGraph::CallSite &site : sites) {
            Function *callee = site.callee;
            if (callee == nullptr || callee == caller || !callee->isDefinition()) continue;
            if (callee->fnType->isVariadic || callee == module.globalInit) continue;
            const CallGraph::Node &info = graph.node(callee);
            if (info.recursive) continue;
            size_t calleeSize = sizeOf(*callee);
            bool small = calleeSize <= kSmallCallee;
            bool onlyUse = info.callSiteCount == 1 && calleeSize <= kSingleCallee;
            if (!small && !onlyUse) continue;
            if (sizeOf(*caller) + calleeSize > kCallerLimit) continue;
            inlineCall(module, *caller, site.call, *callee);
            callerChanged = true;
        }
        if (callerChanged) {
            am.invalidate(*caller);
            am.invalidateModule();
            changed = true;
        }
    }
    return changed;
}

} // namespace passes
