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
    Variable,
    Parameter,
    Field,
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
    std::string mangledName;
    const Type *type;
    Scope      *scope = nullptr;
    ast::Decl  *decl = nullptr;
    int         line;
    int         col;

    RecordSymbol   *parentRecord = nullptr;
    ast::AccessKind access = ast::AccessKind::None;

    bool isStatic = false;
};

class FunctionSymbol : public Symbol {
public:
    FunctionSymbol(std::string name, const FunctionType *type, int line, int col)
        : Symbol(SymbolKind::Function, std::move(name), type, line, col) {}

    const FunctionType *signature() const { return static_cast<const FunctionType *>(type); }

    bool isMethod() const { return parentRecord != nullptr && !isStatic; }

    const FunctionType *loweredType = nullptr;

    bool                isConstructor = false;
    bool                isDestructor = false;
    bool                isBuiltin = false;
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
    std::vector<Symbol *>         fields;
    std::vector<FunctionSymbol *> constructors;
    FunctionSymbol               *destructor = nullptr;
};

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

    Symbol *add(std::unique_ptr<Symbol> symbol, bool findable = true);

    //looks up all symbols with a name
    const std::vector<Symbol *> *find(const std::string &name) const;

    //returns the nearest enclosing function scope
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

std::string namespacePath(const Scope *scope);

std::string namespacePrefix(const Scope *scope);

struct QualifierTarget {
    Scope        *scope = nullptr;
    RecordSymbol *record = nullptr;
    std::string   error;
    std::string   spelling() const;
};

//resolves a qualified name prefix to its scope
QualifierTarget resolveQualifier(Scope *from, const std::vector<std::string> &qualifier, int line, int col);

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

    const Type         *constOf(const Type *type);
    ClosureType        *closureType(ast::LambdaExpr *lambda);

    void dump(std::ostream &out);

private:
    const Type *keep(Type *type);

    std::vector<std::unique_ptr<Type>> types;
    std::unique_ptr<Scope>             root;
};

}

#endif
