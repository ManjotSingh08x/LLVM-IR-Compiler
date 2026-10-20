#ifndef SEMANTIC_SYMBOLS_H
#define SEMANTIC_SYMBOLS_H

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "llvm/ADT/StringMap.h"

#include "ast/ast.h"
#include "semantic/types.h"

namespace sema {

class Scope;

enum class SymbolKind {
    Variable,   //a global, a local, or a static data member
    Parameter,
    Field,      //a non-static data member
    Function,   //a free function, a method, a constructor or a destructor
    Record
};

//one declared name
class Symbol {
public:
    Symbol(SymbolKind kind, std::string name, const Type *type, int line, int col)
        : kind(kind), name(std::move(name)), type(type), line(line), col(col) {}
    virtual ~Symbol() = default;

    Symbol(const Symbol &) = delete;
    Symbol &operator=(const Symbol &) = delete;

    SymbolKind  kind;
    std::string name;
    //the name later passes give the symbol: set for functions, globals and
    //static members, empty for locals and fields
    std::string mangledName;
    const Type *type;
    Scope      *scope = nullptr;  //the scope the symbol is declared in
    ast::Decl  *decl = nullptr;   //its declaration, null for builtins
    int         line;
    int         col;

    //for members of a record
    RecordSymbol   *parentRecord = nullptr;
    ast::AccessKind access = ast::AccessKind::None;

    //static storage: a static global or local, a static member or a static
    //function
    bool isStatic = false;
};

class FunctionSymbol : public Symbol {
public:
    FunctionSymbol(std::string name, const FunctionType *type, int line, int col)
        : Symbol(SymbolKind::Function, std::move(name), type, line, col) {}

    //the signature callers see, without this
    const FunctionType *signature() const { return static_cast<const FunctionType *>(type); }

    //a method that is called on an object, so it receives this
    bool isMethod() const { return parentRecord != nullptr && !isStatic; }

    //the signature of the global function the method becomes: a pointer to
    //the record is put in front as this. the same as signature() for free and
    //static functions
    const FunctionType *loweredType = nullptr;

    bool                isConstructor = false;
    bool                isDestructor = false;
    bool                isBuiltin = false;      //printf and scanf
    ast::FunctionDecl  *definition = nullptr;   //the declaration with the body
};

struct BaseClass {
    RecordSymbol   *record;
    ast::AccessKind access;
};

class RecordSymbol : public Symbol {
public:
    RecordSymbol(std::string name, bool isClass, int line, int col)
        : Symbol(SymbolKind::Record, std::move(name), nullptr, line, col), isClass(isClass) {}

    //Outer::Inner
    std::string qualifiedName() const {
        return outer ? outer->qualifiedName() + "::" + name : name;
    }

    bool                          isClass;
    bool                          isComplete = false;  //set once all members are declared
    Scope                        *members = nullptr;
    RecordSymbol                 *outer = nullptr;     //the enclosing record of a nested one
    std::vector<BaseClass>        bases;
    std::vector<Symbol *>         fields;  //non-static data members, in layout order
    std::vector<FunctionSymbol *> constructors;
    FunctionSymbol               *destructor = nullptr;
};

enum class ScopeKind { Global, Record, Function, Block, Lambda };

//one node of the scope tree. every scope is kept after it is built, and the
//ast nodes that open a scope point at theirs, so later passes walk back into
//exactly the scopes the symbol builder made
class Scope {
public:
    Scope(ScopeKind kind, Scope *parent, int line, int col)
        : kind(kind), parent(parent), line(line), col(col) {}

    Scope(const Scope &) = delete;
    Scope &operator=(const Scope &) = delete;

    Scope *addChild(ScopeKind childKind, int childLine, int childCol);

    //the scope takes ownership of the symbol. a symbol that is not findable
    //is kept but never found by name: constructors and destructors, which are
    //reached through their record instead
    Symbol *add(std::unique_ptr<Symbol> symbol, bool findable = true);

    //the symbols with this name declared directly in this scope. more than one
    //only for overloaded functions. null when there are none
    const std::vector<Symbol *> *find(const std::string &name) const;

    //the nearest enclosing function or lambda scope, the one labels belong to
    Scope *functionScope();

    ScopeKind kind;
    Scope    *parent;
    int       line;
    int       col;

    RecordSymbol    *record = nullptr;    //for a record scope
    FunctionSymbol  *function = nullptr;  //for a function scope
    ast::LambdaExpr *lambda = nullptr;    //for a lambda scope

    std::vector<std::unique_ptr<Scope>>       children;
    std::vector<std::unique_ptr<Symbol>>      symbols;  //in declaration order
    llvm::StringMap<std::vector<Symbol *>>    table;
    llvm::StringMap<ast::LabelStmt *>         labels;   //for function and lambda scopes
};

//the root of the scope tree. it also owns every compound type, since types are
//shared: the type of &x points at the type of x
class SymbolTable {
public:
    SymbolTable();

    Scope *global() { return root.get(); }

    const Type         *pointerTo(const Type *pointee);
    const Type         *referenceTo(const Type *referee);
    const Type         *arrayOf(const Type *element, uint64_t size);
    const FunctionType *functionType(const Type *returnType, std::vector<const Type *> params,
                                     bool isVariadic);
    const Type         *recordType(RecordSymbol *record);
    ClosureType        *closureType(ast::LambdaExpr *lambda);

    //writes the scope tree with every symbol, its type and its mangled name
    void dump(std::ostream &out);

private:
    const Type *keep(Type *type);

    std::vector<std::unique_ptr<Type>> types;
    std::unique_ptr<Scope>             root;
};

} // namespace sema

#endif
