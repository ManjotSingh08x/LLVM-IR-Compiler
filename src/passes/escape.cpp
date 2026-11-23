#include "passes/escape.h"

#include <map>
#include <vector>

namespace passes {

using namespace ir;

namespace {

bool isScalarSlot(const sema::Type *type) {
    return sema::isIntegral(type) || sema::isFloating(type) || type->is(sema::TypeKind::Pointer) || isInt64(type);
}

} // namespace

const Value *EscapeInfo::baseOf(const Value *address) {
    while (address->getKind() == ValueKind::Instruction &&
           static_cast<const Instruction *>(address)->getOpcode() == Opcode::GEP) {
        address = static_cast<const GEPInst *>(address)->base;
    }
    return address;
}

EscapeInfo::EscapeInfo(const Function &fn) {
    //every use of every value, found once
    std::map<const Value *, std::vector<const Instruction *>> users;
    std::vector<const Instruction *> allocas;
    for (const BasicBlock *block : fn.getBlocks()) {
        for (const std::unique_ptr<Instruction> &inst : block->instructions) {
            if (inst->getOpcode() == Opcode::Alloca) allocas.push_back(inst.get());
            for (const Value *operand : inst->getOperands()) users[operand].push_back(inst.get());
        }
    }

    for (const Instruction *alloca : allocas) {
        const sema::Type *slotType = static_cast<const AllocaInst *>(alloca)->getAccessType();
        bool escapes = false;
        bool wholeAccessOnly = true;  //only direct loads and stores of the slot's own type
        std::vector<const Value *> work(1, alloca);
        while (!work.empty() && !escapes) {
            const Value *address = work.back();
            work.pop_back();
            for (const Instruction *user : users[address]) {
                switch (user->getOpcode()) {
                case Opcode::Load:
                    if (address != alloca || !sema::isSameType(user->getType(), slotType)) wholeAccessOnly = false;
                    break;
                case Opcode::Store: {
                    const StoreInst *store = static_cast<const StoreInst *>(user);
                    if (store->value == address) {
                        escapes = true;  //the address itself is written to memory
                    } else if (address != alloca || !sema::isSameType(store->getAccessType(), slotType)) {
                        wholeAccessOnly = false;
                    }
                    break;
                }
                case Opcode::GEP:
                    if (static_cast<const GEPInst *>(user)->base == address) {
                        wholeAccessOnly = false;
                        work.push_back(user);
                    } else {
                        escapes = true;  //used as an index, so as a number
                    }
                    break;
                default:
                    escapes = true;
                }
            }
        }
        if (escapes) {
            escaped.insert(alloca);
        } else if (wholeAccessOnly && isScalarSlot(slotType)) {
            promotable.insert(alloca);
        }
    }
}

bool EscapeInfo::mayAlias(const Value *a, const Value *b) const {
    if (a == b) return true;
    const Value *baseA = baseOf(a);
    const Value *baseB = baseOf(b);
    auto isObject = [](const Value *base) {
        if (base->getKind() == ValueKind::GlobalVariable) return true;
        return base->getKind() == ValueKind::Instruction &&
               static_cast<const Instruction *>(base)->getOpcode() == Opcode::Alloca;
    };
    //two different stack slots or globals never overlap
    if (isObject(baseA) && isObject(baseB) && baseA != baseB) return false;
    //a slot nobody else can reach is only touched through its own addresses
    auto isPrivate = [&](const Value *base) {
        return base->getKind() == ValueKind::Instruction && !escapes(base) &&
               static_cast<const Instruction *>(base)->getOpcode() == Opcode::Alloca;
    };
    if ((isPrivate(baseA) || isPrivate(baseB)) && baseA != baseB) return false;
    return true;
}

bool EscapeInfo::callMayTouch(const Value *address) const {
    const Value *base = baseOf(address);
    if (base->getKind() == ValueKind::Instruction &&
        static_cast<const Instruction *>(base)->getOpcode() == Opcode::Alloca) {
        return escapes(base);
    }
    return true;
}

} // namespace passes
