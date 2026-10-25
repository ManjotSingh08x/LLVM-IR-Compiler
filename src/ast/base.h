#ifndef AST_BASE_H
#define AST_BASE_H

#include <memory>
#include <string>
#include <vector>

//semantic results are held by pointer so the ast does not depend on semantic headers
namespace sema {
class Type;
class Symbol;
class FunctionSymbol;
class RecordSymbol;
class Scope;
} // namespace sema

namespace ast {

//one kind per concrete node, First/Last markers bound each family
enum class ASTKind {
    Program,
    TypeSpec,

    //declarations
    VarDecl,
    FirstDecl = VarDecl,
    ParamDecl,
    FunctionDecl,
    ConstructorDecl,
    DestructorDecl,
    RecordDecl,
    AccessSpecDecl,
    TypedefDecl,
    LastDecl = TypedefDecl,

    //statements
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

    //expressions
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
    CallExpr,
    ArraySubscriptExpr,
    MemberExpr,
    NewExpr,
    DeleteExpr,
    InitListExpr,
    LambdaExpr,
    ImplicitCastExpr,
    LastExpr = ImplicitCastExpr
};

//an owning list of child nodes
template <class T>
using NodeList = std::vector<std::unique_ptr<T>>;

//base of every node: a fixed kind and the source location it is reported at
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

//family bases
class Decl : public ASTNode {
protected:
    using ASTNode::ASTNode;
};

class Stmt : public ASTNode {
protected:
    using ASTNode::ASTNode;
};

//filled by the type checker: the type and whether the expression is an lvalue
class Expr : public ASTNode {
public:
    const sema::Type *type = nullptr;
    bool              isLValue = false;

protected:
    using ASTNode::ASTNode;
};

//the whole translation unit
class Program : public ASTNode {
public:
    explicit Program(NodeList<Decl> decls)
        : ASTNode(ASTKind::Program, 1, 1), decls(std::move(decls)) {}


    NodeList<Decl> decls;
    sema::Scope   *scope = nullptr;  //the global scope
};

} // namespace ast

#endif
