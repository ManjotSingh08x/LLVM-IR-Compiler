#include "passes/local_dag.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

#include "passes/escape.h"
#include "passes/fold.h"

namespace passes {

using namespace ir;

namespace {

bool isScalarType(const sema::Type *type) {
    return sema::isIntegral(type) || sema::isFloating(type) || type->is(sema::TypeKind::Pointer);
}

bool isPure(const Instruction &inst) {
    switch (inst.getOpcode()) {
    case Opcode::Binary:
    case Opcode::Unary:
    case Opcode::Cast:
    case Opcode::Select:
    case Opcode::GEP:
        return true;
    default:
        return false;
    }
}

//a memory value known until something may change that memory
struct Known {
    Value            *address;
    const sema::Type *type;
    Value            *value;
};

} // namespace

bool removeUnusedValues(Function &fn) {
    bool removedAny = false;
    for (;;) {
        std::map<const Value *, size_t> uses;
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                for (Value *operand : inst->getOperands()) uses[operand]++;
            }
        }
        std::vector<Instruction *> unused;
        for (BasicBlock *block : fn.getBlocks()) {
            for (std::unique_ptr<Instruction> &inst : block->instructions) {
                bool removable = isPure(*inst) || inst->getOpcode() == Opcode::Load || inst->getOpcode() == Opcode::Phi;
                if (removable && uses[inst.get()] == 0) unused.push_back(inst.get());
            }
        }
        if (unused.empty()) return removedAny;
        for (Instruction *inst : unused) inst->getParent()->erase(inst);
        removedAny = true;
    }
}

bool LocalDAG::run(Function &fn, Module &module, AnalysisManager &am) {
    (void)am;
    EscapeInfo escape(fn);
    std::map<Value *, Value *> replaced;  //an instruction -> the value that replaces it
    std::set<Instruction *> dead;
    bool changed = false;

    auto resolve = [&](Value *value) {
        std::map<Value *, Value *>::iterator it = replaced.find(value);
        while (it != replaced.end()) {
            value = it->second;
            it = replaced.find(value);
        }
        return value;
    };
    auto replaceWith = [&](Instruction *inst, Value *value) {
        replaced[inst] = value;
        dead.insert(inst);
        changed = true;
    };

    for (BasicBlock *block : fn.getBlocks()) {
        std::map<std::string, Instruction *> available;  //value numbers of the block's expressions
        std::vector<Known> known;
        std::vector<StoreInst *> unread;

        std::vector<Instruction *> order;
        for (std::unique_ptr<Instruction> &inst : block->instructions) order.push_back(inst.get());

        for (Instruction *inst : order) {
            for (Value *operand : inst->getOperands()) {
                Value *now = resolve(operand);
                if (now != operand) {
                    inst->replaceOperand(operand, now);
                    changed = true;
                }
            }

            switch (inst->getOpcode()) {
            case Opcode::Load: {
                LoadInst *load = static_cast<LoadInst *>(inst);
                unread.erase(std::remove_if(unread.begin(), unread.end(),
                                            [&](StoreInst *s) { return escape.mayAlias(s->address, load->address); }),
                             unread.end());
                if (!isScalarType(load->getType())) break;
                Value *same = nullptr;
                for (size_t i = known.size(); i-- > 0;) {
                    if (known[i].address == load->address && sema::isSameUnqualified(known[i].type, load->getType())) {
                        same = known[i].value;
                        break;
                    }
                }
                if (same) {
                    replaceWith(load, same);
                } else {
                    known.push_back(Known{ load->address, load->getType(), load });
                }
                break;
            }
            case Opcode::Store: {
                StoreInst *store = static_cast<StoreInst *>(inst);
                //an unread store to the same place is dead
                for (size_t i = 0; i < unread.size();) {
                    if (unread[i]->address == store->address &&
                        formatType(unread[i]->getAccessType()) == formatType(store->getAccessType())) {
                        dead.insert(unread[i]);
                        unread.erase(unread.begin() + i);
                        changed = true;
                    } else {
                        i++;
                    }
                }
                known.erase(std::remove_if(known.begin(), known.end(),
                                           [&](const Known &k) { return escape.mayAlias(k.address, store->address); }),
                            known.end());
                if (isScalarType(store->getAccessType())) {
                    known.push_back(Known{ store->address, store->getAccessType(), store->value });
                }
                unread.push_back(store);
                break;
            }
            case Opcode::Call:
                known.erase(std::remove_if(known.begin(), known.end(),
                                           [&](const Known &k) { return escape.callMayTouch(k.address); }),
                            known.end());
                unread.erase(std::remove_if(unread.begin(), unread.end(),
                                            [&](StoreInst *s) { return escape.callMayTouch(s->address); }),
                             unread.end());
                break;
            case Opcode::Binary:
            case Opcode::Unary:
            case Opcode::Cast:
            case Opcode::Select:
            case Opcode::GEP: {
                //fold when every operand is constant (for select, the condition is enough)
                std::vector<const Constant *> constants;
                for (Value *operand : inst->getOperands()) constants.push_back(asConstant(operand));
                if (inst->getOpcode() != Opcode::GEP) {
                    if (Value *folded = foldInstruction(module, *inst, constants)) {
                        replaceWith(inst, folded);
                        break;
                    }
                }
                bool rewritten = false;
                if (Value *same = simplifyInstruction(module, *inst, rewritten)) {
                    replaceWith(inst, same);
                    break;
                }
                if (rewritten) changed = true;
                std::string key = expressionKey(*inst);
                std::map<std::string, Instruction *>::iterator it = available.find(key);
                if (it != available.end()) {
                    replaceWith(inst, it->second);
                } else {
                    available[key] = inst;
                }
                break;
            }
            default:
                break;
            }
        }
    }

    //uses in other blocks follow the replacements too
    for (BasicBlock *block : fn.getBlocks()) {
        for (std::unique_ptr<Instruction> &inst : block->instructions) {
            if (dead.count(inst.get())) continue;
            for (Value *operand : inst->getOperands()) {
                Value *now = resolve(operand);
                if (now != operand) inst->replaceOperand(operand, now);
            }
        }
    }
    for (Instruction *inst : dead) inst->getParent()->erase(inst);
    if (removeUnusedValues(fn)) changed = true;
    return changed;
}

} // namespace passes
