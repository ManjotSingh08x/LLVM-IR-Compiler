#ifndef SEMANTIC_TYPE_LAYOUT_H
#define SEMANTIC_TYPE_LAYOUT_H

#include <cstdint>
#include <map>
#include <vector>

#include "semantic/symbols.h"
#include "semantic/types.h"

namespace sema {

//sizes, alignments and field offsets of types, matching llvm's x86-64 data layout
class TypeLayout {
public:
    explicit TypeLayout(SymbolTable &symTab) : symTab(symTab) {}

    uint64_t sizeOf(const Type *type);
    uint64_t alignOf(const Type *type);

    //for a record or closure: where element index starts, and its type (a base is its record type)
    uint64_t          elementOffset(const Type *aggregate, unsigned index);
    const Type *elementType(const Type *aggregate, unsigned index);

private:
    struct StructLayout {
        std::vector<const Type *> elements;
        std::vector<uint64_t>           offsets;
        uint64_t                        size = 0;
        uint64_t                        align = 1;
    };

    const StructLayout &structLayout(const Type *type);
    void                place(StructLayout &layout);

    SymbolTable                &symTab;
    std::map<const void *, StructLayout> cache;  //by RecordSymbol or LambdaExpr
};

uint64_t alignTo(uint64_t value, uint64_t align);

} // namespace sema

#endif
