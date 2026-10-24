#include "ir/record_layout.h"

namespace ir {

unsigned RecordLayout::fieldIndex(const sema::RecordSymbol *record, const sema::Symbol *field) {
    for (size_t i = 0; i < record->fields.size(); i++) {
        if (record->fields[i] == field) return (unsigned)(record->bases.size() + i);
    }
    return 0;
}

bool RecordLayout::findBasePath(const sema::RecordSymbol *derived, const sema::RecordSymbol *base,
                                std::vector<unsigned> &path) {
    for (size_t i = 0; i < derived->bases.size(); i++) {
        const sema::RecordSymbol *direct = derived->bases[i].record;
        path.push_back((unsigned)i);
        if (direct == base || findBasePath(direct, base, path)) return true;
        path.pop_back();
    }
    return false;
}

sema::FunctionSymbol *RecordLayout::defaultConstructor(const sema::RecordSymbol *record) {
    for (sema::FunctionSymbol *ctor : record->constructors) {
        if (ctor->signature()->params.empty()) return ctor;
    }
    return nullptr;
}

sema::RecordSymbol *RecordLayout::elementRecord(const sema::Type *type) {
    while (const sema::ArrayType *array = sema::asArray(type)) type = array->element;
    return sema::asRecord(type);
}

std::string RecordLayout::recordPath(const sema::RecordSymbol *record) {
    std::string prefix = record->outer ? recordPath(record->outer) : "";
    return prefix + std::to_string(record->name.size()) + record->name;
}

ast::VarDecl *RecordLayout::fieldDeclaration(const sema::Symbol *field) {
    if (field->decl == nullptr || field->decl->getKind() != ast::ASTKind::VarDecl) return nullptr;
    ast::VarDecl *decl = static_cast<ast::VarDecl *>(field->decl);
    if (decl->initStyle == ast::InitStyle::None) return nullptr;
    return decl;
}

bool RecordLayout::needsConstruction(const sema::Type *type) {
    sema::RecordSymbol *record = elementRecord(type);
    return record != nullptr && needsConstruction(record);
}

bool RecordLayout::needsConstruction(const sema::RecordSymbol *record) {
    std::map<const sema::RecordSymbol *, bool>::iterator it = constructionCache.find(record);
    if (it != constructionCache.end()) return it->second;

    bool needed = !record->constructors.empty();
    for (const sema::BaseClass &base : record->bases) {
        if (needsConstruction(base.record)) needed = true;
    }
    for (const sema::Symbol *field : record->fields) {
        if (fieldDeclaration(field) != nullptr || needsConstruction(field->type)) needed = true;
    }
    constructionCache[record] = needed;
    return needed;
}

bool RecordLayout::needsDestruction(const sema::Type *type) {
    sema::RecordSymbol *record = elementRecord(type);
    return record != nullptr && needsDestruction(record);
}

bool RecordLayout::needsDestruction(const sema::RecordSymbol *record) {
    std::map<const sema::RecordSymbol *, bool>::iterator it = destructionCache.find(record);
    if (it != destructionCache.end()) return it->second;

    bool needed = record->destructor != nullptr;
    for (const sema::BaseClass &base : record->bases) {
        if (needsDestruction(base.record)) needed = true;
    }
    for (const sema::Symbol *field : record->fields) {
        if (needsDestruction(field->type)) needed = true;
    }
    destructionCache[record] = needed;
    return needed;
}

} // namespace ir
