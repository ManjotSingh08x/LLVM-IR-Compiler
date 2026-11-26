#ifndef AST_BASE_H
#define AST_BASE_H

#include <memory>
#include <string>
#include <vector>

namespace sema {
class Type;
class Symbol;
class FunctionSymbol;
class RecordSymbol;
class Scope;
}

namespace ast {

enum class ASTKind {
    Program,
    TypeSpec,

    VarDecl,
    FirstDecl = VarDecl,
    ParamDecl,
    FunctionDecl,
    ConstructorDecl,
    DestructorDecl,
    RecordDecl,
    AccessSpecDecl,
    TypedefDecl,
    NamespaceDecl,
    LastDecl = NamespaceDecl,

    CompoundStmt,
    FirstStmt = CompoundStmt,
    DeclStmt,
    ExprStmt,
    IfStmt,
    SwitchStmt,
    WhileStmt,
    DoStmt,
    ForStmt,
    ReturnStmt,
    BreakStmt,
    ContinueStmt,
    GotoStmt,
    LabelStmt,
    CaseStmt,
    DefaultStmt,
    LastStmt = DefaultStmt,

    IntLiteral,
    FirstExpr = IntLiteral,
    FloatLiteral,
    CharLiteral,
    StringLiteral,
    BoolLiteral,
    NullLiteral,
    ThisExpr,
    DeclRefExpr,
    ParenExpr,
    UnaryExpr,
    BinaryExpr,
    AssignExpr,
    ConditionalExpr,
    CommaExpr,
    CallExpr,
    ArraySubscriptExpr,
    MemberExpr,
    NewExpr,
    DeleteExpr,
    InitListExpr,
    LambdaExpr,
    CastExpr,
    SizeofExpr,
    ImplicitCastExpr,
    LastExpr = ImplicitCastExpr
};

template <class T>
using NodeList = std::vector<std::unique_ptr<T>>;

class ASTNode {
public:
    virtual ~ASTNode() = default;

    ASTNode(const ASTNode &) = delete;
    ASTNode &operator=(const ASTNode &) = delete;

    ASTKind getKind() const { return kind; }

    bool isDecl() const { return kind >= ASTKind::FirstDecl && kind <= ASTKind::LastDecl; }
    bool isStmt() const { return kind >= ASTKind::FirstStmt && kind <= ASTKind::LastStmt; }
    bool isExpr() const { return kind >= ASTKind::FirstExpr && kind <= ASTKind::LastExpr; }

    int line;
    int col;

protected:
    ASTNode(ASTKind kind, int line, int col) : line(line), col(col), kind(kind) {}

private:
    const ASTKind kind;
};

class Decl : public ASTNode {
protected:
    using ASTNode::ASTNode;
};

class Stmt : public ASTNode {
protected:
    using ASTNode::ASTNode;
};

class Expr : public ASTNode {
public:
    const sema::Type *type = nullptr;
    bool              isLValue = false;

protected:
    using ASTNode::ASTNode;
};

class Program : public ASTNode {
public:
    explicit Program(NodeList<Decl> decls)
        : ASTNode(ASTKind::Program, 1, 1), decls(std::move(decls)) {}

    NodeList<Decl> decls;
    sema::Scope   *scope = nullptr;
};

}

#endif
