#ifndef IR_RECORD_LAYOUT_H
#define IR_RECORD_LAYOUT_H

#include <map>
#include <string>
#include <vector>

#include "semantic/symbols.h"
#include "semantic/types.h"

namespace ir {

//answers questions about the layout and the life of records, without writing
//any ir. a record is laid out as its bases in order, then its own fields:
//  class Registered : public Animal, public Trackable { int slot; }
//  %class.Registered = type { %class.Animal, %class.Trackable, i32 }
class RecordLayout {
public:
    //the struct index of a non-static field inside the record that declares it
    static unsigned fieldIndex(const sema::RecordSymbol *record, const sema::Symbol *field);

    //the struct indices that lead from derived to its base subobject, searching
    //every level: Registered -> Trackable is { 1 }. false when base is not a base
    static bool findBasePath(const sema::RecordSymbol *derived, const sema::RecordSymbol *base,
                             std::vector<unsigned> &path);

    //the user constructor that takes no arguments, or null
    static sema::FunctionSymbol *defaultConstructor(const sema::RecordSymbol *record);

    //the record an array of records (of any depth) is made of, or the record
    //itself, or null for anything else
    static sema::RecordSymbol *elementRecord(const sema::Type *type);

    //Outer::Inner as it appears in mangled names: 5Outer5Inner
    static std::string recordPath(const sema::RecordSymbol *record);

    //true when making an object of this type must run code: a user
    //constructor, a field with an initializer such as int start{0}, or a base
    //or field that needs construction itself. arrays ask about their elements
    bool needsConstruction(const sema::Type *type);
    bool needsConstruction(const sema::RecordSymbol *record);

    //true when ending the life of an object must run code: a user destructor,
    //or a base or field that needs destruction itself
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
