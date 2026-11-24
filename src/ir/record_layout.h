#ifndef IR_RECORD_LAYOUT_H
#define IR_RECORD_LAYOUT_H

#include <map>
#include <string>
#include <vector>

#include "semantic/symbols.h"
#include "semantic/types.h"

namespace ir {

//layout and lifetime queries for records, laid out as bases then fields
class RecordLayout {
public:
    //struct index of a field inside the record that declares it
    static unsigned fieldIndex(const sema::RecordSymbol *record, const sema::Symbol *field);

    //struct indices from derived to its base subobject, false when not a base
    static bool findBasePath(const sema::RecordSymbol *derived, const sema::RecordSymbol *base,
                             std::vector<unsigned> &path);

    //the user constructor taking no arguments, or null
    static sema::FunctionSymbol *defaultConstructor(const sema::RecordSymbol *record);

    //the record an array of records is made of, the record itself, or null
    static sema::RecordSymbol *elementRecord(const sema::Type *type);

    //the record's mangled path: Outer::Inner becomes 5Outer5Inner
    static std::string recordPath(const sema::RecordSymbol *record);

    bool needsConstruction(const sema::Type *type);
    bool needsConstruction(const sema::RecordSymbol *record);

    bool needsDestruction(const sema::Type *type);
    bool needsDestruction(const sema::RecordSymbol *record);

    //the declaration holding a field's in-class initializer, or null
    static ast::VarDecl *fieldDeclaration(const sema::Symbol *field);

private:
    std::map<const sema::RecordSymbol *, bool> constructionCache;
    std::map<const sema::RecordSymbol *, bool> destructionCache;
};

} // namespace ir

#endif
