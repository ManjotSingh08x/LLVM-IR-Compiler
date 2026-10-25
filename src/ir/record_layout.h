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
    //the struct index of a non-static field inside the record that declares it
    static unsigned fieldIndex(const sema::RecordSymbol *record, const sema::Symbol *field);

    //struct indices from derived to its base subobject, false when not a base
    static bool findBasePath(const sema::RecordSymbol *derived, const sema::RecordSymbol *base,
                             std::vector<unsigned> &path);

    //the user constructor that takes no arguments, or null
    static sema::FunctionSymbol *defaultConstructor(const sema::RecordSymbol *record);

    //the record an array of records is made of, the record itself, or null
    static sema::RecordSymbol *elementRecord(const sema::Type *type);

    //Outer::Inner as it appears in mangled names: 5Outer5Inner
    static std::string recordPath(const sema::RecordSymbol *record);

    //true when constructing this type must run code
    bool needsConstruction(const sema::Type *type);
    bool needsConstruction(const sema::RecordSymbol *record);

    //true when destroying this type must run code
    bool needsDestruction(const sema::Type *type);
    bool needsDestruction(const sema::RecordSymbol *record);

    //the in-class initializer of a field, int start{0}, or null when it has none
    static ast::VarDecl *fieldDeclaration(const sema::Symbol *field);

private:
    //the answers are remembered: a record is asked about many times
    std::map<const sema::RecordSymbol *, bool> constructionCache;
    std::map<const sema::RecordSymbol *, bool> destructionCache;
};

} // namespace ir

#endif
