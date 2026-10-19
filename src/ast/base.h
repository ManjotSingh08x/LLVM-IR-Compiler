#ifndef AST_BASE_H
#define AST_BASE_H

#include <memory>
#include <string>
#include <vector>

namespace ast {

//every concrete node has its own kind, which is how a node is told apart from
//the others: visitors switch on it and static_cast to the concrete class. the
//First/Last markers group the kinds of one family for range checks
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
    LastDecl = AccessSpecDecl,

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
    LastExpr = LambdaExpr
};

//an owning list of child nodes
template <class T>
using NodeList = std::vector<std::unique_ptr<T>>;

//base of every node. the kind is fixed at construction, the location is the
//line and column of the token the node is reported at
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

class Expr : public ASTNode {
protected:
    using ASTNode::ASTNode;
};

//the written type in front of a declarator: int, Dog, struct Node
enum class TypeSpecKind { Builtin, Named, Struct };

class TypeSpec : public ASTNode {
public:
    TypeSpec(TypeSpecKind specKind, std::string name, int tokIndex, int line, int col)
        : ASTNode(ASTKind::TypeSpec, line, col), specKind(specKind),
          name(std::move(name)), tokIndex(tokIndex) {}


    //one declaration line shares its type between declarators, each gets a copy
    std::unique_ptr<TypeSpec> clone() const {
        return std::unique_ptr<TypeSpec>(new TypeSpec(specKind, name, tokIndex, line, col));
    }

    TypeSpecKind specKind;
    std::string  name;
    int          tokIndex;  //index of the naming token in the token table
};

//the whole translation unit
class Program : public ASTNode {
public:
    explicit Program(NodeList<Decl> decls)
        : ASTNode(ASTKind::Program, 1, 1), decls(std::move(decls)) {}


    NodeList<Decl> decls;
};

} // namespace ast

#endif
