#ifndef AST_DECLARATIONS_H
#define AST_DECLARATIONS_H

#include "ast/statements.h"

namespace ast {

class ParamDecl;

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

inline std::string qualifiedSpelling(const std::vector<std::string> &qualifier, const std::string &name) {
    std::string out;
    for (const std::string &part : qualifier) out += part + "::";
    return out + name;
}

enum class TypeSpecKind { Builtin, Named, Struct, Pointer, Reference, Array, Function };

class TypeSpec : public ASTNode {
public:
    TypeSpec(TypeSpecKind specKind, std::string name, int tokIndex, int line, int col);

    TypeSpec(TypeSpecKind specKind, std::unique_ptr<TypeSpec> inner, int line, int col);

    ~TypeSpec() override;

    bool isName() const {
        return specKind == TypeSpecKind::Builtin || specKind == TypeSpecKind::Named
            || specKind == TypeSpecKind::Struct;
    }

    std::unique_ptr<TypeSpec> clone() const {
        std::unique_ptr<TypeSpec> copy(new TypeSpec(specKind, name, tokIndex, line, col));
        copy->isConst = isConst;
        copy->qualifier = qualifier;
        return copy;
    }

    TypeSpecKind              specKind;
    std::string               name;
    bool                      isConst = false;
    std::vector<std::string>  qualifier;
    int                       tokIndex = -1;
    std::unique_ptr<TypeSpec> inner;
    std::unique_ptr<Expr>     size;
    NodeList<ParamDecl>       params;

    const sema::Type         *resolved = nullptr;
};

enum class InitStyle { None, Copy, Brace, CtorArgs };

enum class AccessKind { None, Public, Private, Protected };

class VarDecl : public Decl {
public:
    VarDecl(QualifiedName name, std::unique_ptr<TypeSpec> type, int line, int col)
        : Decl(ASTKind::VarDecl, line, col), name(std::move(name)), type(std::move(type)) {}

    QualifiedName             name;
    std::unique_ptr<TypeSpec> type;
    InitStyle                 initStyle = InitStyle::None;
    std::unique_ptr<Expr>     init;
    NodeList<Expr>            ctorArgs;
    bool                      isStatic = false;

    sema::Symbol             *symbol = nullptr;
    sema::FunctionSymbol     *constructor = nullptr;
};

class ParamDecl : public Decl {
public:
    ParamDecl(std::string name, std::unique_ptr<TypeSpec> type, int line, int col)
        : Decl(ASTKind::ParamDecl, line, col), name(std::move(name)), type(std::move(type)) {}

    std::string               name;
    std::unique_ptr<TypeSpec> type;

    sema::Symbol             *symbol = nullptr;
};

inline TypeSpec::TypeSpec(TypeSpecKind specKind, std::string name, int tokIndex, int line, int col)
    : ASTNode(ASTKind::TypeSpec, line, col), specKind(specKind), name(std::move(name)),
      tokIndex(tokIndex) {}

inline TypeSpec::TypeSpec(TypeSpecKind specKind, std::unique_ptr<TypeSpec> inner, int line, int col)
    : ASTNode(ASTKind::TypeSpec, line, col), specKind(specKind), inner(std::move(inner)) {}

inline TypeSpec::~TypeSpec() = default;

class FunctionDecl : public Decl {
public:
    FunctionDecl(std::unique_ptr<TypeSpec> returnType, QualifiedName name,
                 NodeList<ParamDecl> params, std::unique_ptr<CompoundStmt> body,
                 int line, int col)
        : FunctionDecl(ASTKind::FunctionDecl, std::move(returnType), std::move(name),
                       std::move(params), std::move(body), line, col) {}

    bool isDefinition() const { return body != nullptr; }

    std::unique_ptr<TypeSpec>     returnType;
    QualifiedName                 name;
    NodeList<ParamDecl>           params;
    std::unique_ptr<CompoundStmt> body;
    bool                          isStatic = false;

    sema::FunctionSymbol         *symbol = nullptr;
    sema::Scope                  *scope = nullptr;

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

class DestructorDecl : public FunctionDecl {
public:
    DestructorDecl(QualifiedName name, NodeList<ParamDecl> params,
                   std::unique_ptr<CompoundStmt> body, int line, int col)
        : FunctionDecl(ASTKind::DestructorDecl, nullptr, std::move(name), std::move(params),
                       std::move(body), line, col) {}

};

struct BaseSpecifier {
    AccessKind               access = AccessKind::None;
    std::string              name;
    std::vector<std::string> qualifier;
};

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
    bool                       hasBody = true;

    sema::RecordSymbol        *symbol = nullptr;
    sema::Scope               *scope = nullptr;
};

class AccessSpecDecl : public Decl {
public:
    AccessSpecDecl(AccessKind access, int line, int col)
        : Decl(ASTKind::AccessSpecDecl, line, col), access(access) {}

    AccessKind access;
};

//one node per declarator of a typedef
class TypedefDecl : public Decl {
public:
    TypedefDecl(std::string name, std::unique_ptr<TypeSpec> type, int line, int col)
        : Decl(ASTKind::TypedefDecl, line, col), name(std::move(name)), type(std::move(type)) {}

    std::string               name;
    std::unique_ptr<TypeSpec> type;

    sema::Symbol             *symbol = nullptr;
};

class NamespaceDecl : public Decl {
public:
    NamespaceDecl(std::string name, NodeList<Decl> decls, int line, int col)
        : Decl(ASTKind::NamespaceDecl, line, col), name(std::move(name)), decls(std::move(decls)) {}

    std::string    name;
    NodeList<Decl> decls;

    sema::Scope   *scope = nullptr;
};

}

#endif
