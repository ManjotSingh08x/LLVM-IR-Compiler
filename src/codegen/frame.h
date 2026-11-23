#ifndef CODEGEN_FRAME_H
#define CODEGEN_FRAME_H

#include <cstdint>
#include <map>

#include "codegen/x86.h"

namespace codegen {

//the stack frame of one function, every offset relative to %rbp
//  16(%rbp) and up: arguments passed on the stack
//  below %rbp: allocas, then a home slot per value (made on first need), then callee-saved registers
class Frame {
public:
    void addAlloca(const ir::Value *alloca, uint64_t size, uint64_t align);
    bool isAlloca(const ir::Value *value) const { return allocas.count(value) != 0; }
    long long allocaOffset(const ir::Value *alloca) const;

    //the 8-byte memory location of a value (the dragon book's address descriptor home)
    long long home(const ir::Value *value);
    void      setHome(const ir::Value *value, long long offset) { homes[value] = offset; }

    //the bytes of an aggregate value (a loaded record or array), kept whole in the frame
    long long aggregateSlot(const ir::Value *value, uint64_t size, uint64_t align);
    bool      isAggregate(const ir::Value *value) const { return aggregates.count(value) != 0; }

    //the 8-byte slot a phi's incoming value is copied into at the end of each predecessor
    long long phiSlot(const ir::Value *phi);

    //where a callee-saved register is kept between prologue and epilogue
    long long saveSlot(Reg reg);

    //bytes to reserve below %rbp, a multiple of 16 so calls see an aligned stack
    uint64_t size() const;

private:
    long long allocate(uint64_t size, uint64_t align);

    uint64_t                                used = 0;
    std::map<const ir::Value *, long long> allocas;
    std::map<const ir::Value *, long long> homes;
    std::map<const ir::Value *, long long> aggregates;
    std::map<const ir::Value *, long long> phis;
    std::map<Reg, long long>               saves;
};

} // namespace codegen

#endif
