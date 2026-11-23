#ifndef CODEGEN_TARGET_LAYOUT_H
#define CODEGEN_TARGET_LAYOUT_H

#include <cstdint>
#include <map>
#include <vector>

#include "semantic/symbols.h"
#include "semantic/types.h"

namespace codegen {

//sizes, alignments and field offsets of sema types, matching llvm's x86-64 data layout
//records are laid out as the ir writes them: bases in order, then fields, each at its natural alignment
class TargetLayout {
public:
    explicit TargetLayout(sema::SymbolTable &symTab) : symTab(symTab) {}

    uint64_t sizeOf(const sema::Type *type);
    uint64_t alignOf(const sema::Type *type);

    //for a record or closure: where element index starts, and its type (a base is its record type)
    uint64_t          elementOffset(const sema::Type *aggregate, unsigned index);
    const sema::Type *elementType(const sema::Type *aggregate, unsigned index);

private:
    struct StructLayout {
        std::vector<const sema::Type *> elements;
        std::vector<uint64_t>           offsets;
        uint64_t                        size = 0;
        uint64_t                        align = 1;
    };

    const StructLayout &structLayout(const sema::Type *type);
    void                place(StructLayout &layout);

    sema::SymbolTable                &symTab;
    std::map<const void *, StructLayout> cache;  //by RecordSymbol or LambdaExpr
};

uint64_t alignTo(uint64_t value, uint64_t align);

} // namespace codegen

#endif
