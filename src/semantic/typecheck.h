#ifndef SEMANTIC_TYPECHECK_H
#define SEMANTIC_TYPECHECK_H

#include <set>
#include <string>
#include <vector>

#include "semantic/diagnostics.h"
#include "semantic/symbols.h"
#include "semantic/visitor.h"

namespace sema {

//second semantic pass: works out the type of every expression and checks
//that each use fits. it runs over the scope tree the symbol builder made and
//writes its results into the tree:
//  - Expr::type and Expr::isLValue on every expression
//  - the symbol every name, member and call resolves to
//  - an ImplicitCastExpr around every value the source converts silently
//  - the constructor a declaration or new runs, the target of every goto
//
//a name is found from the innermost scope outwards. in blocks, functions and
//the global scope only names declared before the use count, as in C++. a
//record scope sees all its members and those of its bases, so a method may
//use a member declared below it
//
//an expression that fails gets the error type, which every later check lets
//through quietly, so one mistake gives one message
class TypeChecker : public ast::ASTVisitor<TypeChecker, const Type *> {
public:
    TypeChecker(SymbolTable &table, Diagnostics &diags) : table(table), diags(diags) {}

    void check(ast::Program *program);

    //statements return null
    const Type *visitCompoundStmt(ast::CompoundStmt *node);
    const Type *visitDeclStmt(ast::DeclStmt *node);
    const Type *visitExprStmt(ast::ExprStmt *node);
    const Type *visitIfStmt(ast::IfStmt *node);
    const Type *visitSwitchStmt(ast::SwitchStmt *node);
    const Type *visitWhileStmt(ast::WhileStmt *node);
    const Type *visitDoStmt(ast::DoStmt *node);
    const Type *visitForStmt(ast::ForStmt *node);
    const Type *visitReturnStmt(ast::ReturnStmt *node);
    const Type *visitBreakStmt(ast::BreakStmt *node);
    const Type *visitContinueStmt(ast::ContinueStmt *node);
    const Type *visitGotoStmt(ast::GotoStmt *node);
    const Type *visitLabelStmt(ast::LabelStmt *node);
    const Type *visitCaseStmt(ast::CaseStmt *node);
    const Type *visitDefaultStmt(ast::DefaultStmt *node);

    //expressions return their type
    const Type *visitIntLiteral(ast::IntLiteral *node);
    const Type *visitFloatLiteral(ast::FloatLiteral *node);
    const Type *visitCharLiteral(ast::CharLiteral *node);
    const Type *visitStringLiteral(ast::StringLiteral *node);
    const Type *visitBoolLiteral(ast::BoolLiteral *node);
    const Type *visitNullLiteral(ast::NullLiteral *node);
    const Type *visitThisExpr(ast::ThisExpr *node);
    const Type *visitDeclRefExpr(ast::DeclRefExpr *node);
    const Type *visitParenExpr(ast::ParenExpr *node);
    const Type *visitUnaryExpr(ast::UnaryExpr *node);
    const Type *visitBinaryExpr(ast::BinaryExpr *node);
    const Type *visitAssignExpr(ast::AssignExpr *node);
    const Type *visitConditionalExpr(ast::ConditionalExpr *node);
    const Type *visitCallExpr(ast::CallExpr *node);
    const Type *visitArraySubscriptExpr(ast::ArraySubscriptExpr *node);
    const Type *visitMemberExpr(ast::MemberExpr *node);
    const Type *visitNewExpr(ast::NewExpr *node);
    const Type *visitDeleteExpr(ast::DeleteExpr *node);
    const Type *visitInitListExpr(ast::InitListExpr *node);
    const Type *visitLambdaExpr(ast::LambdaExpr *node);
    const Type *visitImplicitCastExpr(ast::ImplicitCastExpr *node);

private:
    struct SwitchInfo {
        std::set<long long> values;
        bool                hasDefault = false;
    };

    //what the statements of the function or lambda being checked may do
    struct FunctionContext {
        FunctionSymbol             *function = nullptr;  //for a lambda, the function it is written in
        ast::LambdaExpr            *lambda = nullptr;
        const Type                 *returnType = nullptr;  //for a lambda, null until its first return
        int                         loops = 0;
        int                         breakables = 0;  //loops and switches
        std::vector<SwitchInfo *>   switches;
        std::vector<ast::GotoStmt *> gotos;
    };

    //what a name was found as
    struct Lookup {
        std::vector<Symbol *>  symbols;               //empty when not found, more than one for overloads
        RecordSymbol          *namingRecord = nullptr;  //when found as a member: the record searched
        std::vector<BaseClass> path;                  //the bases walked from namingRecord to the member
        bool                   ambiguous = false;     //found in two different bases
    };

    //declarations
    void checkRecord(ast::RecordDecl *node);
    void checkFunction(ast::FunctionDecl *node);
    void checkVarDecl(ast::VarDecl *node);
    void resolveGotos(Scope *functionScope);

    //initialisation
    void initFromValue(std::unique_ptr<ast::Expr> &value, const Type *type);
    void initFromList(ast::InitListExpr *list, const Type *type);
    FunctionSymbol *construct(RecordSymbol *record, ast::NodeList<ast::Expr> &args, int line, int col);

    //names
    Lookup lookupName(const std::string &name, int line, int col);
    bool   lookupMember(RecordSymbol *record, const std::string &name, Lookup &result);
    void   captureVariable(Symbol *symbol, const std::vector<Scope *> &lambdas, int line, int col);
    RecordSymbol *thisRecord(int line, int col);
    bool   memberOfThis(Symbol *member, int line, int col);
    void   checkAccess(Symbol *member, const Lookup &lookup, int line, int col);
    void   checkBaseConversion(const Type *from, const Type *to, int line, int col, const std::string &what);
    const Type *finishDeclRef(ast::DeclRefExpr *node, const Lookup &lookup);
    bool   findMember(ast::MemberExpr *node, Lookup &result);
    const Type *finishMember(ast::MemberExpr *node, const Lookup &lookup);

    //calls
    FunctionSymbol *chooseOverload(const std::vector<FunctionSymbol *> &candidates,
                                   ast::NodeList<ast::Expr> &args, const std::string &name, int line, int col);
    void applyArguments(const FunctionType *signature, ast::NodeList<ast::Expr> &args,
                        const std::string &name, int line, int col);
    const Type *callResult(ast::CallExpr *node, const FunctionType *signature);

    //values
    const Type *check(std::unique_ptr<ast::Expr> &expr);
    const Type *decayValue(std::unique_ptr<ast::Expr> &expr);
    bool        convert(std::unique_ptr<ast::Expr> &expr, const Type *to, const std::string &what);
    void        wrap(std::unique_ptr<ast::Expr> &expr, ast::CastKind kind, const Type *type);
    void        checkCondition(std::unique_ptr<ast::Expr> &cond, const std::string &what);
    bool        checkModifiable(ast::Expr *expr, const std::string &what);
    const Type *setType(ast::Expr *node, const Type *type, bool isLValue);
    RecordSymbol *contextRecord();

    SymbolTable    &table;
    Diagnostics    &diags;
    Scope          *current = nullptr;
    FunctionContext context;
};

} // namespace sema

#endif
