#include "codegen/target_layout.h"

#include "ir/ir_types.h"

namespace codegen {

uint64_t alignTo(uint64_t value, uint64_t align) {
    if (align <= 1) return value;
    return (value + align - 1) / align * align;
}

uint64_t TargetLayout::sizeOf(const sema::Type *type) {
    if (ir::isInt64(type)) return 8;
    switch (type->getKind()) {
    case sema::TypeKind::Bool:
    case sema::TypeKind::Char:
        return 1;
    case sema::TypeKind::Int:
    case sema::TypeKind::Float:
        return 4;
    case sema::TypeKind::Double:
    case sema::TypeKind::Pointer:
    case sema::TypeKind::Reference:
    case sema::TypeKind::Null:
        return 8;
    case sema::TypeKind::Array: {
        const sema::ArrayType *array = sema::asArray(type);
        return array->size * sizeOf(array->element);
    }
    case sema::TypeKind::Record:
    case sema::TypeKind::Closure:
        return structLayout(type).size;
    default:
        return 0;
    }
}

uint64_t TargetLayout::alignOf(const sema::Type *type) {
    if (ir::isInt64(type)) return 8;
    switch (type->getKind()) {
    case sema::TypeKind::Array:
        return alignOf(sema::asArray(type)->element);
    case sema::TypeKind::Record:
    case sema::TypeKind::Closure:
        return structLayout(type).align;
    default: {
        uint64_t size = sizeOf(type);
        return size == 0 ? 1 : size;
    }
    }
}

uint64_t TargetLayout::elementOffset(const sema::Type *aggregate, unsigned index) {
    const StructLayout &layout = structLayout(aggregate);
    return index < layout.offsets.size() ? layout.offsets[index] : 0;
}

const sema::Type *TargetLayout::elementType(const sema::Type *aggregate, unsigned index) {
    const StructLayout &layout = structLayout(aggregate);
    return index < layout.elements.size() ? layout.elements[index] : nullptr;
}

void TargetLayout::place(StructLayout &layout) {
    uint64_t offset = 0;
    for (const sema::Type *element : layout.elements) {
        uint64_t align = alignOf(element);
        offset = alignTo(offset, align);
        layout.offsets.push_back(offset);
        offset += sizeOf(element);
        if (align > layout.align) layout.align = align;
    }
    layout.size = alignTo(offset, layout.align);
}

const TargetLayout::StructLayout &TargetLayout::structLayout(const sema::Type *type) {
    static const StructLayout empty;

    //the same shape as formatRecordBody and formatClosureBody in ir/ir_types.cpp
    if (type->is(sema::TypeKind::Record)) {
        sema::RecordSymbol *record = sema::asRecord(type);
        std::map<const void *, StructLayout>::iterator it = cache.find(record);
        if (it != cache.end()) return it->second;
        StructLayout layout;
        for (const sema::BaseClass &base : record->bases) layout.elements.push_back(symTab.recordType(base.record));
        for (const sema::Symbol *field : record->fields) layout.elements.push_back(field->type);
        place(layout);
        return cache[record] = layout;
    }
    if (type->is(sema::TypeKind::Closure)) {
        const ast::LambdaExpr *lambda = sema::asClosure(type)->lambda;
        std::map<const void *, StructLayout>::iterator it = cache.find(lambda);
        if (it != cache.end()) return it->second;
        StructLayout layout;
        for (const ast::LambdaCapture &capture : lambda->usedCaptures) {
            if (capture.kind == ast::LambdaCapture::Kind::ByCopy) {
                layout.elements.push_back(sema::stripReference(capture.symbol->type));
            } else {
                layout.elements.push_back(symTab.pointerTo(sema::voidType()));
            }
        }
        place(layout);
        return cache[lambda] = layout;
    }
    return empty;
}

} // namespace codegen
