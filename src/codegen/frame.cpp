#include "codegen/frame.h"

#include "codegen/target_layout.h"

namespace codegen {

long long Frame::allocate(uint64_t size, uint64_t align) {
    if (align > 16) align = 16;
    used = alignTo(used + size, align == 0 ? 1 : align);
    return -(long long)used;
}

void Frame::addAlloca(const ir::Value *alloca, uint64_t size, uint64_t align) {
    //an empty record still gets its own address
    allocas[alloca] = allocate(size == 0 ? 1 : size, align);
}

long long Frame::allocaOffset(const ir::Value *alloca) const {
    std::map<const ir::Value *, long long>::const_iterator it = allocas.find(alloca);
    return it == allocas.end() ? 0 : it->second;
}

long long Frame::home(const ir::Value *value) {
    std::map<const ir::Value *, long long>::iterator it = homes.find(value);
    if (it != homes.end()) return it->second;
    long long offset = allocate(8, 8);
    homes[value] = offset;
    return offset;
}

long long Frame::aggregateSlot(const ir::Value *value, uint64_t size, uint64_t align) {
    std::map<const ir::Value *, long long>::iterator it = aggregates.find(value);
    if (it != aggregates.end()) return it->second;
    long long offset = allocate(size == 0 ? 1 : size, align);
    aggregates[value] = offset;
    return offset;
}

long long Frame::saveSlot(Reg reg) {
    std::map<Reg, long long>::iterator it = saves.find(reg);
    if (it != saves.end()) return it->second;
    long long offset = allocate(8, 8);
    saves[reg] = offset;
    return offset;
}

uint64_t Frame::size() const { return alignTo(used, 16); }

} // namespace codegen
