#include "semantic/type_layout.h"

namespace sema {

uint64_t alignTo(uint64_t value, uint64_t align) {
    if (align <= 1) return value;
    return (value + align - 1) / align * align;
}

uint64_t TypeLayout::sizeOf(const Type *type) {
    if (isIntegral(type)) return integerSize(type);
    switch (type->getKind()) {
    case TypeKind::Float:
        return 4;
    case TypeKind::Double:
    case TypeKind::Pointer:
    case TypeKind::Reference:
    case TypeKind::Null:
        return 8;
    case TypeKind::Array: {
        const ArrayType *array = asArray(type);
        return array->size * sizeOf(array->element);
    }
    case TypeKind::Record:
    case TypeKind::Closure:
        return structLayout(type).size;
    default:
        return 0;
    }
}

uint64_t TypeLayout::alignOf(const Type *type) {
    switch (type->getKind()) {
    case TypeKind::Array:
        return alignOf(asArray(type)->element);
    case TypeKind::Record:
    case TypeKind::Closure:
        return structLayout(type).align;
    default: {
        uint64_t size = sizeOf(type);
        return size == 0 ? 1 : size;
    }
    }
}

uint64_t TypeLayout::elementOffset(const Type *aggregate, unsigned index) {
    const StructLayout &layout = structLayout(aggregate);
    return index < layout.offsets.size() ? layout.offsets[index] : 0;
}

const Type *TypeLayout::elementType(const Type *aggregate, unsigned index) {
    const StructLayout &layout = structLayout(aggregate);
    return index < layout.elements.size() ? layout.elements[index] : nullptr;
}

void TypeLayout::place(StructLayout &layout) {
    uint64_t offset = 0;
    for (const Type *element : layout.elements) {
        uint64_t align = alignOf(element);
        offset = alignTo(offset, align);
        layout.offsets.push_back(offset);
        offset += sizeOf(element);
        if (align > layout.align) layout.align = align;
    }
    layout.size = alignTo(offset, layout.align);
}

const TypeLayout::StructLayout &TypeLayout::structLayout(const Type *type) {
    static const StructLayout empty;

    if (type->is(TypeKind::Record)) {
        RecordSymbol *record = asRecord(type);
        std::map<const void *, StructLayout>::iterator it = cache.find(record);
        if (it != cache.end()) return it->second;
        StructLayout layout;
        for (const BaseClass &base : record->bases) layout.elements.push_back(symTab.recordType(base.record));
        for (const Symbol *field : record->fields) layout.elements.push_back(field->type);
        place(layout);
        return cache[record] = layout;
    }
    if (type->is(TypeKind::Closure)) {
        const ast::LambdaExpr *lambda = asClosure(type)->lambda;
        std::map<const void *, StructLayout>::iterator it = cache.find(lambda);
        if (it != cache.end()) return it->second;
        StructLayout layout;
        for (const ast::LambdaCapture &capture : lambda->usedCaptures) {
            if (capture.kind == ast::LambdaCapture::Kind::ByCopy) {
                layout.elements.push_back(stripReference(capture.symbol->type));
            } else {
                layout.elements.push_back(symTab.pointerTo(voidType()));
            }
        }
        place(layout);
        return cache[lambda] = layout;
    }
    return empty;
}

}
