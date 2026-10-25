#ifndef AST_EXPRESSIONS_H
#define AST_EXPRESSIONS_H

#include "ast/declarations.h"

namespace ast {

enum class UnaryOp { PreInc, PreDec, PostInc, PostDec, Plus, Minus, LogicalNot, BitNot, Deref, AddrOf };

enum class BinaryOp {
    LogicalOr, LogicalAnd, BitOr, BitXor, BitAnd,
    Eq, Ne, Lt, Gt, Le, Ge,
    Shl, Shr, Add, Sub, Mul, Div, Rem
};

enum class AssignOp { Assign, AddAssign, SubAssign, MulAssign, DivAssign, RemAssign };

//literals keep their source spelling, the value is computed during semantic analysis
class IntLiteral : public Expr {
public:
    IntLiteral(std::string spelling, int line, int col)
        : Expr(ASTKind::IntLiteral, line, col), spelling(std::move(spelling)) {}


    std::string spelling;
};

class FloatLiteral : public Expr {
public:
    FloatLiteral(std::string spelling, int line, int col)
        : Expr(ASTKind::FloatLiteral, line, col), spelling(std::move(spelling)) {}


    std::string spelling;
};

class CharLiteral : public Expr {
public:
    CharLiteral(std::string spelling, int line, int col)
        : Expr(ASTKind::CharLiteral, line, col), spelling(std::move(spelling)) {}


    std::string spelling;
};

class StringLiteral : public Expr {
public:
    StringLiteral(std::string spelling, int line, int col)
        : Expr(ASTKind::StringLiteral, line, col), spelling(std::move(spelling)) {}


    std::string spelling;
};

class BoolLiteral : public Expr {
public:
    BoolLiteral(bool value, int line, int col)
        : Expr(ASTKind::BoolLiteral, line, col), value(value) {}


    bool value;
};

class NullLiteral : public Expr {
public:
    NullLiteral(int line, int col) : Expr(ASTKind::NullLiteral, line, col) {}

};

class ThisExpr : public Expr {
public:
    ThisExpr(int line, int col) : Expr(ASTKind::ThisExpr, line, col) {}

};

//a use of a name
class DeclRefExpr : public Expr {
public:
    DeclRefExpr(std::string name, int line, int col)
        : Expr(ASTKind::DeclRefExpr, line, col), name(std::move(name)) {}


    std::string   name;
    sema::Symbol *symbol = nullptr;  //what the name refers to, set by the type checker
};

class ParenExpr : public Expr {
public:
    ParenExpr(std::unique_ptr<Expr> sub, int line, int col)
        : Expr(ASTKind::ParenExpr, line, col), sub(std::move(sub)) {}


    std::unique_ptr<Expr> sub;
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(UnaryOp op, std::unique_ptr<Expr> operand, int line, int col)
        : Expr(ASTKind::UnaryExpr, line, col), op(op), operand(std::move(operand)) {}


    bool isPostfix() const { return op == UnaryOp::PostInc || op == UnaryOp::PostDec; }

    UnaryOp               op;
    std::unique_ptr<Expr> operand;
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(BinaryOp op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs, int line, int col)
        : Expr(ASTKind::BinaryExpr, line, col), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}


    BinaryOp              op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
};

class AssignExpr : public Expr {
public:
    AssignExpr(AssignOp op, std::unique_ptr<Expr> target, std::unique_ptr<Expr> value, int line, int col)
        : Expr(ASTKind::AssignExpr, line, col), op(op), target(std::move(target)),
          value(std::move(value)) {}


    AssignOp              op;
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> value;
};

//cond ? trueExpr : falseExpr
class ConditionalExpr : public Expr {
public:
    ConditionalExpr(std::unique_ptr<Expr> cond, std::unique_ptr<Expr> trueExpr,
                    std::unique_ptr<Expr> falseExpr, int line, int col)
        : Expr(ASTKind::ConditionalExpr, line, col), cond(std::move(cond)),
          trueExpr(std::move(trueExpr)), falseExpr(std::move(falseExpr)) {}


    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> trueExpr;
    std::unique_ptr<Expr> falseExpr;
};

class CallExpr : public Expr {
public:
    CallExpr(std::unique_ptr<Expr> callee, NodeList<Expr> args, int line, int col)
        : Expr(ASTKind::CallExpr, line, col), callee(std::move(callee)), args(std::move(args)) {}


    std::unique_ptr<Expr> callee;
    NodeList<Expr>        args;

    //the function called by name, null for function pointers and lambdas
    sema::FunctionSymbol *function = nullptr;
};
//a[i], where base may be any expression: f(x)[i]
class ArraySubscriptExpr : public Expr {
public:
    ArraySubscriptExpr(std::unique_ptr<Expr> base, std::unique_ptr<Expr> index, int line, int col)
        : Expr(ASTKind::ArraySubscriptExpr, line, col), base(std::move(base)), index(std::move(index)) {}


    std::unique_ptr<Expr> base;
    std::unique_ptr<Expr> index;
};

//base.member or base->member
class MemberExpr : public Expr {
public:
    MemberExpr(std::unique_ptr<Expr> base, std::string member, bool isArrow, int line, int col)
        : Expr(ASTKind::MemberExpr, line, col), base(std::move(base)), member(std::move(member)),
          isArrow(isArrow) {}


    std::unique_ptr<Expr> base;
    std::string           member;
    bool                  isArrow;
    sema::Symbol         *symbol = nullptr;  //the field or method, set by the type checker
};

//new T, new T(args), new T{...}, new T[n], new T[n]{...}
enum class NewForm { Plain, Args, InitList, Array, ArrayInit };

class NewExpr : public Expr {
public:
    NewExpr(NewForm form, int line, int col) : Expr(ASTKind::NewExpr, line, col), form(form) {}


    NewForm                   form;
    std::unique_ptr<TypeSpec> type;  //the element type for the array forms
    NodeList<Expr>            args;  //for Args
    NodeList<Expr>            dims;  //for Array and ArrayInit
    std::unique_ptr<Expr>     init;  //an InitListExpr, for InitList and ArrayInit
    sema::FunctionSymbol     *constructor = nullptr;  //the constructor that runs, if any
};

class DeleteExpr : public Expr {
public:
    DeleteExpr(bool isArray, std::unique_ptr<Expr> operand, int line, int col)
        : Expr(ASTKind::DeleteExpr, line, col), isArray(isArray), operand(std::move(operand)) {}


    bool                  isArray;
    std::unique_ptr<Expr> operand;
};

//{ a, b, { c } }
class InitListExpr : public Expr {
public:
    InitListExpr(NodeList<Expr> inits, int line, int col)
        : Expr(ASTKind::InitListExpr, line, col), inits(std::move(inits)) {}


    NodeList<Expr>        inits;
    sema::FunctionSymbol *constructor = nullptr;  //when the list is a class's constructor arguments
};

enum class CaptureDefault { None, ByCopy, ByRef };

struct LambdaCapture {
    enum class Kind { ByCopy, ByRef, This };

    Kind          kind = Kind::ByCopy;
    std::string   name;  //empty for this
    sema::Symbol *symbol = nullptr;  //the captured variable, set by the type checker
};

class LambdaExpr : public Expr {
public:
    LambdaExpr(CaptureDefault captureDefault, std::vector<LambdaCapture> captures,
               NodeList<ParamDecl> params, std::unique_ptr<CompoundStmt> body, int line, int col)
        : Expr(ASTKind::LambdaExpr, line, col), captureDefault(captureDefault),
          captures(std::move(captures)), params(std::move(params)), body(std::move(body)) {}


    CaptureDefault                captureDefault;
    std::vector<LambdaCapture>    captures;
    NodeList<ParamDecl>           params;
    std::unique_ptr<CompoundStmt> body;

    sema::Scope                  *scope = nullptr;  //holds the parameters
    //variables the body uses from the enclosing function, listed or captured by default
    std::vector<LambdaCapture>    usedCaptures;
};

//a conversion the source leaves implicit, inserted by the type checker
enum class CastKind {
    IntegralCast,       //between bool, char and int
    IntegralToFloating,
    FloatingToIntegral,
    FloatingCast,       //float and double
    ToBool,             //an arithmetic value or a pointer tested as bool
    ArrayToPointer,
    FunctionToPointer,
    NullToPointer,
    DerivedToBase,      //a derived class pointer or object used as its base
    PointerToVoid       //any object pointer to void *
};

class ImplicitCastExpr : public Expr {
public:
    ImplicitCastExpr(CastKind castKind, std::unique_ptr<Expr> sub, const sema::Type *toType)
        : Expr(ASTKind::ImplicitCastExpr, sub->line, sub->col), castKind(castKind),
          sub(std::move(sub)) {
        type = toType;
    }


    CastKind              castKind;
    std::unique_ptr<Expr> sub;
};

} // namespace ast

#endif
