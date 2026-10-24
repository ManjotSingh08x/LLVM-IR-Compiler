#ifndef AST_STATEMENTS_H
#define AST_STATEMENTS_H

#include "ast/base.h"

namespace ast {

//{ ... }
class CompoundStmt : public Stmt {
public:
    CompoundStmt(NodeList<Stmt> body, int line, int col)
        : Stmt(ASTKind::CompoundStmt, line, col), body(std::move(body)) {}


    NodeList<Stmt> body;
    sema::Scope   *scope = nullptr;
};

//a local declaration, one Decl per declarator: int a = 1, *b;
class DeclStmt : public Stmt {
public:
    DeclStmt(NodeList<Decl> decls, int line, int col)
        : Stmt(ASTKind::DeclStmt, line, col), decls(std::move(decls)) {}


    NodeList<Decl> decls;
};

//expr; with a null expr for the empty statement
class ExprStmt : public Stmt {
public:
    ExprStmt(std::unique_ptr<Expr> expr, int line, int col)
        : Stmt(ASTKind::ExprStmt, line, col), expr(std::move(expr)) {}


    std::unique_ptr<Expr> expr;
};

class IfStmt : public Stmt {
public:
    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> thenStmt,
           std::unique_ptr<Stmt> elseStmt, int line, int col)
        : Stmt(ASTKind::IfStmt, line, col), cond(std::move(cond)),
          thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}


    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> thenStmt;
    std::unique_ptr<Stmt> elseStmt;  //null without an else
};

class SwitchStmt : public Stmt {
public:
    SwitchStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> body, int line, int col)
        : Stmt(ASTKind::SwitchStmt, line, col), cond(std::move(cond)), body(std::move(body)) {}


    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> body;
};

//until (c) is a while loop that runs while c is false
class WhileStmt : public Stmt {
public:
    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> body, int line, int col)
        : Stmt(ASTKind::WhileStmt, line, col), cond(std::move(cond)), body(std::move(body)) {}


    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> body;
    bool                  isUntil = false;
};

//do ... until (c); likewise repeats while c is false
class DoStmt : public Stmt {
public:
    DoStmt(std::unique_ptr<Stmt> body, std::unique_ptr<Expr> cond, int line, int col)
        : Stmt(ASTKind::DoStmt, line, col), body(std::move(body)), cond(std::move(cond)) {}


    std::unique_ptr<Stmt> body;
    std::unique_ptr<Expr> cond;
    bool                  isUntil = false;
};

//every part but the body may be null. the init declarations live in their own
//scope around the body
class ForStmt : public Stmt {
public:
    ForStmt(std::unique_ptr<Stmt> init, std::unique_ptr<Expr> cond, std::unique_ptr<Expr> inc,
            std::unique_ptr<Stmt> body, int line, int col)
        : Stmt(ASTKind::ForStmt, line, col), init(std::move(init)), cond(std::move(cond)),
          inc(std::move(inc)), body(std::move(body)) {}


    std::unique_ptr<Stmt> init;  //a DeclStmt or an ExprStmt
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> inc;
    std::unique_ptr<Stmt> body;
    sema::Scope          *scope = nullptr;
};

class ReturnStmt : public Stmt {
public:
    ReturnStmt(std::unique_ptr<Expr> value, int line, int col)
        : Stmt(ASTKind::ReturnStmt, line, col), value(std::move(value)) {}


    std::unique_ptr<Expr> value;  //null for a bare return
};

class BreakStmt : public Stmt {
public:
    BreakStmt(int line, int col) : Stmt(ASTKind::BreakStmt, line, col) {}

};

class ContinueStmt : public Stmt {
public:
    ContinueStmt(int line, int col) : Stmt(ASTKind::ContinueStmt, line, col) {}

};

class LabelStmt;

class GotoStmt : public Stmt {
public:
    GotoStmt(std::string label, int line, int col)
        : Stmt(ASTKind::GotoStmt, line, col), label(std::move(label)) {}


    std::string label;
    LabelStmt  *target = nullptr;  //resolved by the type checker
};

//label: statement
class LabelStmt : public Stmt {
public:
    LabelStmt(std::string label, std::unique_ptr<Stmt> sub, int line, int col)
        : Stmt(ASTKind::LabelStmt, line, col), label(std::move(label)), sub(std::move(sub)) {}


    std::string           label;
    std::unique_ptr<Stmt> sub;
};

class CaseStmt : public Stmt {
public:
    CaseStmt(std::unique_ptr<Expr> value, std::unique_ptr<Stmt> sub, int line, int col)
        : Stmt(ASTKind::CaseStmt, line, col), value(std::move(value)), sub(std::move(sub)) {}


    std::unique_ptr<Expr> value;
    std::unique_ptr<Stmt> sub;
    long long             constant = 0;  //the value of the label, worked out by the type checker
};

class DefaultStmt : public Stmt {
public:
    DefaultStmt(std::unique_ptr<Stmt> sub, int line, int col)
        : Stmt(ASTKind::DefaultStmt, line, col), sub(std::move(sub)) {}


    std::unique_ptr<Stmt> sub;
};

} // namespace ast

#endif
