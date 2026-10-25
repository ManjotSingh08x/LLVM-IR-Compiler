#ifndef AST_DECLARATIONS_H
#define AST_DECLARATIONS_H

#include "ast/statements.h"

namespace ast {

class ParamDecl;

//a possibly qualified name, Shape::area. lastTok is the token index of the final part
struct QualifiedName {
    std::vector<std::string> parts;
    int lastTok = -1;

    bool empty() const { return parts.empty(); }

    std::string str() const {
        std::string out;
        for (size_t i = 0; i < parts.size(); i++) {
            if (i > 0) out += "::";
            out += parts[i];
        }
        return out;
    }
};

//a type as written, read inside out: int **(*p)[10] is Pointer -> Array [10] -> Pointer -> Pointer -> int
enum class TypeSpecKind { Builtin, Named, Struct, Pointer, Reference, Array, Function };

class TypeSpec : public ASTNode {
public:
    //a type name: int, Dog, struct Node
    TypeSpec(TypeSpecKind specKind, std::string name, int tokIndex, int line, int col);

    //a pointer, reference, array or function around inner, null until the base type is known
    TypeSpec(TypeSpecKind specKind, std::unique_ptr<TypeSpec> inner, int line, int col);

    //defined below ParamDecl, which the parameter list needs complete
    ~TypeSpec() override;

    bool isName() const {
        return specKind == TypeSpecKind::Builtin || specKind == TypeSpecKind::Named
            || specKind == TypeSpecKind::Struct;
    }

    //each declarator sharing a type name gets its own copy
    std::unique_ptr<TypeSpec> clone() const {
        return std::unique_ptr<TypeSpec>(new TypeSpec(specKind, name, tokIndex, line, col));
    }

    TypeSpecKind              specKind;
    std::string               name;           //for a type name
    int                       tokIndex = -1;  //index of the naming token in the token table
    std::unique_ptr<TypeSpec> inner;          //for the wrapping kinds
    std::unique_ptr<Expr>     size;           //for Array, null for an unsized []
    NodeList<ParamDecl>       params;         //for Function

    const sema::Type         *resolved = nullptr;  //filled in by the symbol builder
};

//how a variable is initialised: int a = 1, int a{1}, Dog d(3)
enum class InitStyle { None, Copy, Brace, CtorArgs };

enum class AccessKind { None, Public, Private, Protected };

class VarDecl : public Decl {
public:
    VarDecl(QualifiedName name, std::unique_ptr<TypeSpec> type, int line, int col)
        : Decl(ASTKind::VarDecl, line, col), name(std::move(name)), type(std::move(type)) {}


    QualifiedName             name;
    std::unique_ptr<TypeSpec> type;
    InitStyle                 initStyle = InitStyle::None;
    std::unique_ptr<Expr>     init;      //for Copy and Brace
    NodeList<Expr>            ctorArgs;  //for CtorArgs
    bool                      isStatic = false;

    sema::Symbol             *symbol = nullptr;
    sema::FunctionSymbol     *constructor = nullptr;  //the constructor that runs, if any
};

class ParamDecl : public Decl {
public:
    ParamDecl(std::string name, std::unique_ptr<TypeSpec> type, int line, int col)
        : Decl(ASTKind::ParamDecl, line, col), name(std::move(name)), type(std::move(type)) {}


    std::string               name;  //empty for an unnamed parameter
    std::unique_ptr<TypeSpec> type;

    sema::Symbol             *symbol = nullptr;
};

inline TypeSpec::TypeSpec(TypeSpecKind specKind, std::string name, int tokIndex, int line, int col)
    : ASTNode(ASTKind::TypeSpec, line, col), specKind(specKind), name(std::move(name)),
      tokIndex(tokIndex) {}

inline TypeSpec::TypeSpec(TypeSpecKind specKind, std::unique_ptr<TypeSpec> inner, int line, int col)
    : ASTNode(ASTKind::TypeSpec, line, col), specKind(specKind), inner(std::move(inner)) {}

inline TypeSpec::~TypeSpec() = default;

//a function declaration, or a definition when it has a body
class FunctionDecl : public Decl {
public:
    FunctionDecl(std::unique_ptr<TypeSpec> returnType, QualifiedName name,
                 NodeList<ParamDecl> params, std::unique_ptr<CompoundStmt> body,
                 int line, int col)
        : FunctionDecl(ASTKind::FunctionDecl, std::move(returnType), std::move(name),
                       std::move(params), std::move(body), line, col) {}


    bool isDefinition() const { return body != nullptr; }

    std::unique_ptr<TypeSpec>     returnType;  //null for constructors and destructors
    QualifiedName                 name;
    NodeList<ParamDecl>           params;
    std::unique_ptr<CompoundStmt> body;  //null for a declaration
    bool                          isStatic = false;

    sema::FunctionSymbol         *symbol = nullptr;
    sema::Scope                  *scope = nullptr;  //holds the parameters

protected:
    FunctionDecl(ASTKind kind, std::unique_ptr<TypeSpec> returnType, QualifiedName name,
                 NodeList<ParamDecl> params, std::unique_ptr<CompoundStmt> body,
                 int line, int col)
        : Decl(kind, line, col), returnType(std::move(returnType)), name(std::move(name)),
          params(std::move(params)), body(std::move(body)) {}
};

class ConstructorDecl : public FunctionDecl {
public:
    ConstructorDecl(QualifiedName name, NodeList<ParamDecl> params,
                    std::unique_ptr<CompoundStmt> body, int line, int col)
        : FunctionDecl(ASTKind::ConstructorDecl, nullptr, std::move(name), std::move(params),
                       std::move(body), line, col) {}

};

//the last part of the name carries the ~, as in Dog::~Dog
class DestructorDecl : public FunctionDecl {
public:
    DestructorDecl(QualifiedName name, NodeList<ParamDecl> params,
                   std::unique_ptr<CompoundStmt> body, int line, int col)
        : FunctionDecl(ASTKind::DestructorDecl, nullptr, std::move(name), std::move(params),
                       std::move(body), line, col) {}

};

struct BaseSpecifier {
    AccessKind  access = AccessKind::None;
    std::string name;
};

//struct or class definition
class RecordDecl : public Decl {
public:
    RecordDecl(bool isClass, std::string name, std::vector<BaseSpecifier> bases,
               NodeList<Decl> members, int line, int col)
        : Decl(ASTKind::RecordDecl, line, col), isClass(isClass), name(std::move(name)),
          bases(std::move(bases)), members(std::move(members)) {}


    bool                       isClass;
    std::string                name;
    std::vector<BaseSpecifier> bases;
    NodeList<Decl>             members;

    sema::RecordSymbol        *symbol = nullptr;
    sema::Scope               *scope = nullptr;  //holds the members
};

//public: private: protected: inside a class, kept in member order
class AccessSpecDecl : public Decl {
public:
    AccessSpecDecl(AccessKind access, int line, int col)
        : Decl(ASTKind::AccessSpecDecl, line, col), access(access) {}


    AccessKind access;
};

//typedef int *IntPtr; one node per declarator, each holding the aliased type
class TypedefDecl : public Decl {
public:
    TypedefDecl(std::string name, std::unique_ptr<TypeSpec> type, int line, int col)
        : Decl(ASTKind::TypedefDecl, line, col), name(std::move(name)), type(std::move(type)) {}


    std::string               name;
    std::unique_ptr<TypeSpec> type;

    sema::Symbol             *symbol = nullptr;
};

} // namespace ast

#endif
