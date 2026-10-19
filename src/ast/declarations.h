#ifndef AST_DECLARATIONS_H
#define AST_DECLARATIONS_H

#include "ast/statements.h"

namespace ast {

class ParamDecl;

//a name that may be qualified, Shape::area. lastTok is the token index of the
//final part, which the token table records the declared type against
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

//what sits between a parenthesis and the name: int (*f)(int), int (*p)[3]
enum class DeclaratorForm { Plain, FunctionPointer, PointerToArray, ReferenceToArray };

//how a variable is initialised: int a = 1, int a{1}, Dog d(3)
enum class InitStyle { None, Copy, Brace, CtorArgs };

//the initializer as parsed, before it is attached to a declarator
struct Initializer {
    InitStyle             style = InitStyle::None;
    std::unique_ptr<Expr> value;
};

//everything after the type in one declarator. the parser builds this first and
//then joins it with the type into a VarDecl or ParamDecl
struct Declarator {
    QualifiedName   name;  //empty for an unnamed parameter
    int             line = 0;
    int             col = 0;
    int             pointerDepth = 0;
    bool            isReference = false;
    DeclaratorForm  form = DeclaratorForm::Plain;
    NodeList<Expr>  dims;      //one entry per [], null for an unsized []
    NodeList<ParamDecl> fnParams;  //parameters of a function pointer
    InitStyle       initStyle = InitStyle::None;
    std::unique_ptr<Expr> init;  //for Copy and Brace
    NodeList<Expr>  ctorArgs;    //for CtorArgs

    void setInitializer(Initializer *initializer) {
        if (initializer == nullptr) return;
        initStyle = initializer->style;
        init = std::move(initializer->value);
        delete initializer;
    }
};

enum class AccessKind { None, Public, Private, Protected };

class VarDecl : public Decl {
public:
    VarDecl(std::unique_ptr<TypeSpec> type, Declarator declarator, int line, int col)
        : Decl(ASTKind::VarDecl, line, col), type(std::move(type)),
          declarator(std::move(declarator)) {}


    std::unique_ptr<TypeSpec> type;
    Declarator                declarator;
    bool                      isStatic = false;
};

class ParamDecl : public Decl {
public:
    ParamDecl(std::unique_ptr<TypeSpec> type, Declarator declarator, int line, int col)
        : Decl(ASTKind::ParamDecl, line, col), type(std::move(type)),
          declarator(std::move(declarator)) {}


    std::unique_ptr<TypeSpec> type;
    Declarator                declarator;
};

//a function declaration or, with a body, a definition. constructors and
//destructors derive from it and have no return type
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
};

//public: private: protected: inside a class, kept in member order
class AccessSpecDecl : public Decl {
public:
    AccessSpecDecl(AccessKind access, int line, int col)
        : Decl(ASTKind::AccessSpecDecl, line, col), access(access) {}


    AccessKind access;
};

} // namespace ast

#endif
