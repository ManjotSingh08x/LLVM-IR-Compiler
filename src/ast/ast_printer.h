#ifndef AST_AST_PRINTER_H
#define AST_AST_PRINTER_H

#include <ostream>
#include <string>

#include "semantic/visitor.h"

namespace ast {

//debugging visitor: one line per node, indented by depth, as role: Kind attrs <line:col>
class ASTPrinter : public ASTVisitor<ASTPrinter> {
public:
    explicit ASTPrinter(std::ostream &out) : out(out) {}

    void print(ASTNode *root) { visit(root); }

    void visitProgram(Program *node);
    void visitTypeSpec(TypeSpec *node);

    void visitVarDecl(VarDecl *node);
    void visitParamDecl(ParamDecl *node);
    void visitFunctionDecl(FunctionDecl *node);
    void visitConstructorDecl(ConstructorDecl *node);
    void visitDestructorDecl(DestructorDecl *node);
    void visitRecordDecl(RecordDecl *node);
    void visitAccessSpecDecl(AccessSpecDecl *node);
    void visitTypedefDecl(TypedefDecl *node);

    void visitCompoundStmt(CompoundStmt *node);
    void visitDeclStmt(DeclStmt *node);
    void visitExprStmt(ExprStmt *node);
    void visitIfStmt(IfStmt *node);
    void visitSwitchStmt(SwitchStmt *node);
    void visitWhileStmt(WhileStmt *node);
    void visitDoStmt(DoStmt *node);
    void visitForStmt(ForStmt *node);
    void visitReturnStmt(ReturnStmt *node);
    void visitBreakStmt(BreakStmt *node);
    void visitContinueStmt(ContinueStmt *node);
    void visitGotoStmt(GotoStmt *node);
    void visitLabelStmt(LabelStmt *node);
    void visitCaseStmt(CaseStmt *node);
    void visitDefaultStmt(DefaultStmt *node);

    void visitIntLiteral(IntLiteral *node);
    void visitFloatLiteral(FloatLiteral *node);
    void visitCharLiteral(CharLiteral *node);
    void visitStringLiteral(StringLiteral *node);
    void visitBoolLiteral(BoolLiteral *node);
    void visitNullLiteral(NullLiteral *node);
    void visitThisExpr(ThisExpr *node);
    void visitDeclRefExpr(DeclRefExpr *node);
    void visitParenExpr(ParenExpr *node);
    void visitUnaryExpr(UnaryExpr *node);
    void visitBinaryExpr(BinaryExpr *node);
    void visitAssignExpr(AssignExpr *node);
    void visitConditionalExpr(ConditionalExpr *node);
    void visitCallExpr(CallExpr *node);
    void visitArraySubscriptExpr(ArraySubscriptExpr *node);
    void visitMemberExpr(MemberExpr *node);
    void visitNewExpr(NewExpr *node);
    void visitDeleteExpr(DeleteExpr *node);
    void visitInitListExpr(InitListExpr *node);
    void visitLambdaExpr(LambdaExpr *node);
    void visitImplicitCastExpr(ImplicitCastExpr *node);

private:
    //writes the line for a node, attrs may be empty
    void line(const ASTNode &node, const std::string &attrs);

    //visits a child one level deeper, labelled with its role
    void child(const char *role, ASTNode *node);

    template <class T>
    void children(const char *role, const NodeList<T> &list) {
        for (const auto &item : list) child(role, item.get());
    }

    //the array sizes and function pointer parameters inside a written type
    void typeChildren(const TypeSpec *type);
    void initChildren(VarDecl *node);
    void functionBody(FunctionDecl *node);

    std::ostream &out;
    int           depth = 0;
    const char   *role = nullptr;
};

} // namespace ast

#endif
