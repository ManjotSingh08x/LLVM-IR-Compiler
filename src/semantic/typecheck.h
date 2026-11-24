#ifndef SEMANTIC_TYPECHECK_H
#define SEMANTIC_TYPECHECK_H

#include <set>
#include <string>
#include <vector>

#include "semantic/diagnostics.h"
#include "semantic/symbols.h"
#include "semantic/type_layout.h"
#include "semantic/visitor.h"

namespace sema {

class TypeChecker : public ast::ASTVisitor<TypeChecker, const Type *> {
public:
    TypeChecker(SymbolTable &table, Diagnostics &diags) : table(table), diags(diags), layout(table) {}

    void check(ast::Program *program);

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
    const Type *visitCommaExpr(ast::CommaExpr *node);
    const Type *visitCallExpr(ast::CallExpr *node);
    const Type *visitArraySubscriptExpr(ast::ArraySubscriptExpr *node);
    const Type *visitMemberExpr(ast::MemberExpr *node);
    const Type *visitNewExpr(ast::NewExpr *node);
    const Type *visitDeleteExpr(ast::DeleteExpr *node);
    const Type *visitInitListExpr(ast::InitListExpr *node);
    const Type *visitLambdaExpr(ast::LambdaExpr *node);
    const Type *visitCastExpr(ast::CastExpr *node);
    const Type *visitSizeofExpr(ast::SizeofExpr *node);
    const Type *visitImplicitCastExpr(ast::ImplicitCastExpr *node);

private:
    struct SwitchInfo {
        std::set<long long> values;
        bool                hasDefault = false;
        const Type         *condType = nullptr;
    };

    struct FunctionContext {
        FunctionSymbol             *function = nullptr;
        ast::LambdaExpr            *lambda = nullptr;
        const Type                 *returnType = nullptr;
        int                         loops = 0;
        int                         breakables = 0;
        std::vector<SwitchInfo *>   switches;
        std::vector<ast::GotoStmt *> gotos;
    };

    struct Lookup {
        std::vector<Symbol *>  symbols;
        RecordSymbol          *namingRecord = nullptr;
        std::vector<BaseClass> path;
        bool                   ambiguous = false;
        bool                   reported = false;
    };

    void checkTopLevel(ast::Decl *decl);
    void checkRecord(ast::RecordDecl *node);
    void checkFunction(ast::FunctionDecl *node);
    void checkVarDecl(ast::VarDecl *node);
    //matches goto statements with their labels
    void resolveGotos(Scope *functionScope);

    //converts a single value to the initialized type
    void initFromValue(std::unique_ptr<ast::Expr> &value, const Type *type);
    //checks brace initializers against the target type
    void initFromList(ast::InitListExpr *list, const Type *type);
    //picks the matching constructor for a call
    FunctionSymbol *construct(RecordSymbol *record, ast::NodeList<ast::Expr> &args, int line, int col);

    //finds a name through enclosing scopes
    Lookup lookupName(const std::string &name, int line, int col);
    Lookup lookupRef(ast::DeclRefExpr *node);
    const Type *sizeofOperand(std::unique_ptr<ast::Expr> &expr);
    bool   lookupMember(RecordSymbol *record, const std::string &name, Lookup &result);
    //records variables a lambda captures
    void   captureVariable(Symbol *symbol, const std::vector<Scope *> &lambdas, int line, int col);
    RecordSymbol *thisRecord(int line, int col);
    bool   memberOfThis(Symbol *member, int line, int col);
    //reports use of private or protected members
    void   checkAccess(Symbol *member, const Lookup &lookup, int line, int col);
    void   checkBaseConversion(const Type *from, const Type *to, int line, int col, const std::string &what);
    const Type *finishDeclRef(ast::DeclRefExpr *node, const Lookup &lookup);
    bool   findMember(ast::MemberExpr *node, Lookup &result);
    const Type *finishMember(ast::MemberExpr *node, const Lookup &lookup);

    //picks the best overload by argument conversions
    FunctionSymbol *chooseOverload(const std::vector<FunctionSymbol *> &candidates,
                                   ast::NodeList<ast::Expr> &args, const std::string &name, int line, int col);
    //converts call arguments to parameter types
    void applyArguments(const FunctionType *signature, ast::NodeList<ast::Expr> &args,
                        const std::string &name, int line, int col);
    const Type *callResult(ast::CallExpr *node, const FunctionType *signature);

    const Type *check(std::unique_ptr<ast::Expr> &expr);
    //turns arrays and functions into pointers
    const Type *decayValue(std::unique_ptr<ast::Expr> &expr);
    //finds the implicit conversion for an expression
    Conversion  conversionOf(ast::Expr *expr, const Type *to);
    //applies an implicit conversion or reports error
    bool        convert(std::unique_ptr<ast::Expr> &expr, const Type *to, const std::string &what);
    void        wrap(std::unique_ptr<ast::Expr> &expr, ast::CastKind kind, const Type *type);
    void        checkCondition(std::unique_ptr<ast::Expr> &cond, const std::string &what);
    bool        checkModifiable(ast::Expr *expr, const std::string &what);
    const Type *setType(ast::Expr *node, const Type *type, bool isLValue);
    RecordSymbol *contextRecord();

    SymbolTable    &table;
    Diagnostics    &diags;
    TypeLayout      layout;
    Scope          *current = nullptr;
    FunctionContext context;
};

}

#endif
