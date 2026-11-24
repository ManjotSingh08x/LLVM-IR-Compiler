#include "codegen/reg_alloc.h"

namespace codegen {

namespace {

const Reg kIntPool[] = {Reg::RBX, Reg::R12, Reg::R13, Reg::R14, Reg::R15, Reg::R10};
const Reg kFloatPool[] = {Reg::XMM8,  Reg::XMM9,  Reg::XMM10, Reg::XMM11,
                          Reg::XMM12, Reg::XMM13, Reg::XMM14, Reg::XMM15};

long nextUseOf(const NextUseMap &nextUse, const ir::Value *value) {
    NextUseMap::const_iterator it = nextUse.find(value);
    return it == nextUse.end() ? kDead : it->second;
}

} // namespace

bool isCallerSaved(Reg reg) {
    if (isXmm(reg)) return true;
    switch (reg) {
    case Reg::RBX:
    case Reg::RBP:
    case Reg::RSP:
    case Reg::R12:
    case Reg::R13:
    case Reg::R14:
    case Reg::R15:
        return false;
    default:
        return true;
    }
}

void RegisterAllocator::beginBlock() {
    holds.clear();
    where.clear();
    registerOnly.clear();
}

bool RegisterAllocator::registerOf(const ir::Value *value, Reg &reg) const {
    std::map<const ir::Value *, Reg>::const_iterator it = where.find(value);
    if (it == where.end()) return false;
    reg = it->second;
    return true;
}

void RegisterAllocator::storeHome(const ir::Value *value, Reg reg) {
    MInst store;
    store.op = !isXmm(reg) ? "movq" : isDoubleType(value->getType()) ? "movsd" : "movss";
    store.operands = { use(reg), memOp(Mem::frame(frame.home(value))) };
    out.push_back(store);
}

void RegisterAllocator::drop(Reg reg) {
    std::map<Reg, const ir::Value *>::iterator it = holds.find(reg);
    if (it == holds.end()) return;
    where.erase(it->second);
    holds.erase(it);
}

Reg RegisterAllocator::allocate(const ir::Value *value, bool isFloat, const NextUseMap &nextUse) {
    const Reg *pool = isFloat ? kFloatPool : kIntPool;
    size_t     count = isFloat ? sizeof kFloatPool / sizeof kFloatPool[0] : sizeof kIntPool / sizeof kIntPool[0];

    Reg  chosen = pool[0];
    bool found = false;
    //1. an empty register
    for (size_t i = 0; i < count && !found; i++) {
        if (!holds.count(pool[i])) {
            chosen = pool[i];
            found = true;
        }
    }
    //2. otherwise the register whose value is needed furthest away (dead values first)
    if (!found) {
        long furthest = -2;
        for (size_t i = 0; i < count; i++) {
            long next = nextUseOf(nextUse, holds[pool[i]]);
            if (next == kLiveOut) next = kLiveOut - 1;  //not read again here, but must be saved
            if (next == kDead) next = kLiveOut;         //nothing to save, the best victim
            if (next > furthest) {
                furthest = next;
                chosen = pool[i];
            }
        }
        const ir::Value *victim = holds[chosen];
        if (registerOnly.count(victim) && nextUseOf(nextUse, victim) != kDead) storeHome(victim, chosen);
        registerOnly.erase(victim);
        drop(chosen);
    }

    holds[chosen] = value;
    where[value] = chosen;
    registerOnly.insert(value);
    if (!isCallerSaved(chosen)) calleeSavedUsed.insert(chosen);
    return chosen;
}

void RegisterAllocator::release(const ir::Value *value) {
    std::map<const ir::Value *, Reg>::iterator it = where.find(value);
    registerOnly.erase(value);
    if (it == where.end()) return;
    holds.erase(it->second);
    where.erase(it);
}

void RegisterAllocator::spillCallerSaved(const NextUseMap &nextUse) {
    std::vector<Reg> emptied;
    for (const std::pair<const Reg, const ir::Value *> &entry : holds) {
        if (!isCallerSaved(entry.first)) continue;
        if (registerOnly.count(entry.second) && nextUseOf(nextUse, entry.second) != kDead) {
            storeHome(entry.second, entry.first);
        }
        registerOnly.erase(entry.second);
        emptied.push_back(entry.first);
    }
    for (Reg reg : emptied) drop(reg);
}

void RegisterAllocator::flush(const passes::ValueSet &liveOut) {
    for (const std::pair<const Reg, const ir::Value *> &entry : holds) {
        if (registerOnly.count(entry.second) && liveOut.count(entry.second)) {
            storeHome(entry.second, entry.first);
            registerOnly.erase(entry.second);
        }
    }
}

} // namespace codegen
