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
    Function,
    Record,
    Typedef,
    Namespace
};

class Symbol {
public:
    Symbol(SymbolKind kind, std::string name, const Type *type, int line, int col)
        : kind(kind), name(std::move(name)), type(type), line(line), col(col) {}
    virtual ~Symbol() = default;

    Symbol(const Symbol &) = delete;
    Symbol &operator=(const Symbol &) = delete;

    SymbolKind  kind;
    std::string name;
    //the mangled name, empty for locals and fields
    std::string mangledName;
    const Type *type;
    Scope      *scope = nullptr;
    ast::Decl  *decl = nullptr;
    int         line;
    int         col;

    //set for members of a record
    RecordSymbol   *parentRecord = nullptr;
    ast::AccessKind access = ast::AccessKind::None;

    //static storage: static globals, locals, members and functions
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

    //the lowered signature, with this in front for methods
    const FunctionType *loweredType = nullptr;

    bool                isConstructor = false;
    bool                isDestructor = false;
    bool                isBuiltin = false;      //printf and scanf
    ast::FunctionDecl  *definition = nullptr;
};

struct BaseClass {
    RecordSymbol   *record;
    ast::AccessKind access;
};

class RecordSymbol : public Symbol {
public:
    RecordSymbol(std::string name, bool isClass, int line, int col)
        : Symbol(SymbolKind::Record, std::move(name), nullptr, line, col), isClass(isClass) {}

    std::string qualifiedName() const;

    bool                          isClass;
    bool                          isComplete = false;
    Scope                        *members = nullptr;
    RecordSymbol                 *outer = nullptr;
    std::vector<BaseClass>        bases;
    std::vector<Symbol *>         fields;  //non-static data members, in layout order
    std::vector<FunctionSymbol *> constructors;
    FunctionSymbol               *destructor = nullptr;
};

//namespace N: its members live in its scope, which every opening of N shares
class NamespaceSymbol : public Symbol {
public:
    NamespaceSymbol(std::string name, int line, int col)
        : Symbol(SymbolKind::Namespace, std::move(name), nullptr, line, col) {}

    std::string qualifiedName() const;

    Scope *members = nullptr;
};

enum class ScopeKind { Global, Namespace, Record, Function, Block, Lambda };

class Scope {
public:
    Scope(ScopeKind kind, Scope *parent, int line, int col)
        : kind(kind), parent(parent), line(line), col(col) {}

    Scope(const Scope &) = delete;
    Scope &operator=(const Scope &) = delete;

    Scope *addChild(ScopeKind childKind, int childLine, int childCol);

    //takes ownership, non-findable symbols such as constructors are never looked up by name
    Symbol *add(std::unique_ptr<Symbol> symbol, bool findable = true);

    //the symbols declared here under name, several for overloads, null when none
    const std::vector<Symbol *> *find(const std::string &name) const;

    //the nearest enclosing function or lambda scope, the one labels belong to
    Scope *functionScope();

    ScopeKind kind;
    Scope    *parent;
    int       line;
    int       col;

    RecordSymbol    *record = nullptr;
    NamespaceSymbol *space = nullptr;
    FunctionSymbol  *function = nullptr;
    ast::LambdaExpr *lambda = nullptr;

    std::vector<std::unique_ptr<Scope>>       children;
    std::vector<std::unique_ptr<Symbol>>      symbols;
    llvm::StringMap<std::vector<Symbol *>>    table;
    llvm::StringMap<ast::LabelStmt *>         labels;
};

//the mangled prefix of the namespaces scope sits in, such as 1N1M
std::string namespacePath(const Scope *scope);

//the same as written in source: N::M::, empty outside any namespace
std::string namespacePrefix(const Scope *scope);

//where A::B:: in A::B::name points: a namespace's scope, or a class (with its member scope)
struct QualifierTarget {
    Scope        *scope = nullptr;
    RecordSymbol *record = nullptr;
    std::string   error;
    std::string   spelling() const;
};

//resolves a qualifier like A::B:: as seen from scope at line:col
QualifierTarget resolveQualifier(Scope *from, const std::vector<std::string> &qualifier, int line, int col);

//root of the scope tree and owner of every compound type
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

    //the const form of a type: an array's elements become const, a reference or function is unchanged
    const Type         *constOf(const Type *type);
    ClosureType        *closureType(ast::LambdaExpr *lambda);

    void dump(std::ostream &out);

private:
    const Type *keep(Type *type);

    std::vector<std::unique_ptr<Type>> types;
    std::unique_ptr<Scope>             root;
};

} // namespace sema

#endif
