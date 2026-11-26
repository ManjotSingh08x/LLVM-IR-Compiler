#ifndef IR_RECORD_LAYOUT_H
#define IR_RECORD_LAYOUT_H

#include <map>
#include <string>
#include <vector>

#include "semantic/symbols.h"
#include "semantic/types.h"

namespace ir {

class RecordLayout {
public:

    //finds the index of a field in the record
    static unsigned fieldIndex(const sema::RecordSymbol *record, const sema::Symbol *field);

    static bool findBasePath(const sema::RecordSymbol *derived, const sema::RecordSymbol *base,
                             std::vector<unsigned> &path);

    static sema::FunctionSymbol *defaultConstructor(const sema::RecordSymbol *record);

    static sema::RecordSymbol *elementRecord(const sema::Type *type);

    static std::string recordPath(const sema::RecordSymbol *record);

    //tells if type needs constructor calls
    bool needsConstruction(const sema::Type *type);
    bool needsConstruction(const sema::RecordSymbol *record);

    //tells if type needs destructor calls
    bool needsDestruction(const sema::Type *type);
    bool needsDestruction(const sema::RecordSymbol *record);

    static ast::VarDecl *fieldDeclaration(const sema::Symbol *field);

private:
    std::map<const sema::RecordSymbol *, bool> constructionCache;
    std::map<const sema::RecordSymbol *, bool> destructionCache;
};

}

#endif
