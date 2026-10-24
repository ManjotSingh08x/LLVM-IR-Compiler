#ifndef SEMANTIC_VISITOR_H
#define SEMANTIC_VISITOR_H

#include "ast/ast.h"

namespace ast {

//llvm StmtVisitor style visitor, shared by every pass over the tree: the
//semantic passes and the ast printer. visit() switches on the node's kind and calls
//the matching visitX of the derived class, so every pass picks its own return
//type: a printer returns void, a type checker could return a Type *.
//
//  class MyPass : public ASTVisitor<MyPass, int> {
//  public:
//      int visitIfStmt(IfStmt *node) { ... }
//  };
//
//a visitX the derived class does not define falls back to its family:
//ConstructorDecl and DestructorDecl to visitFunctionDecl, then every node to
//visitDecl, visitStmt or visitExpr, which return RetTy()
template <class Derived, class RetTy = void>
class ASTVisitor {
public:
    RetTy visit(ASTNode *node) {
        if (node == nullptr) return RetTy();

        switch (node->getKind()) {
        case ASTKind::Program:            return derived().visitProgram(static_cast<Program *>(node));
        case ASTKind::TypeSpec:           return derived().visitTypeSpec(static_cast<TypeSpec *>(node));

        case ASTKind::VarDecl:            return derived().visitVarDecl(static_cast<VarDecl *>(node));
        case ASTKind::ParamDecl:          return derived().visitParamDecl(static_cast<ParamDecl *>(node));
        case ASTKind::FunctionDecl:       return derived().visitFunctionDecl(static_cast<FunctionDecl *>(node));
        case ASTKind::ConstructorDecl:    return derived().visitConstructorDecl(static_cast<ConstructorDecl *>(node));
        case ASTKind::DestructorDecl:     return derived().visitDestructorDecl(static_cast<DestructorDecl *>(node));
        case ASTKind::RecordDecl:         return derived().visitRecordDecl(static_cast<RecordDecl *>(node));
        case ASTKind::AccessSpecDecl:     return derived().visitAccessSpecDecl(static_cast<AccessSpecDecl *>(node));
        case ASTKind::TypedefDecl:        return derived().visitTypedefDecl(static_cast<TypedefDecl *>(node));

        case ASTKind::CompoundStmt:       return derived().visitCompoundStmt(static_cast<CompoundStmt *>(node));
        case ASTKind::DeclStmt:           return derived().visitDeclStmt(static_cast<DeclStmt *>(node));
        case ASTKind::ExprStmt:           return derived().visitExprStmt(static_cast<ExprStmt *>(node));
        case ASTKind::IfStmt:             return derived().visitIfStmt(static_cast<IfStmt *>(node));
        case ASTKind::SwitchStmt:         return derived().visitSwitchStmt(static_cast<SwitchStmt *>(node));
        case ASTKind::WhileStmt:          return derived().visitWhileStmt(static_cast<WhileStmt *>(node));
        case ASTKind::DoStmt:             return derived().visitDoStmt(static_cast<DoStmt *>(node));
        case ASTKind::ForStmt:            return derived().visitForStmt(static_cast<ForStmt *>(node));
        case ASTKind::ReturnStmt:         return derived().visitReturnStmt(static_cast<ReturnStmt *>(node));
        case ASTKind::BreakStmt:          return derived().visitBreakStmt(static_cast<BreakStmt *>(node));
        case ASTKind::ContinueStmt:       return derived().visitContinueStmt(static_cast<ContinueStmt *>(node));
        case ASTKind::GotoStmt:           return derived().visitGotoStmt(static_cast<GotoStmt *>(node));
        case ASTKind::LabelStmt:          return derived().visitLabelStmt(static_cast<LabelStmt *>(node));
        case ASTKind::CaseStmt:           return derived().visitCaseStmt(static_cast<CaseStmt *>(node));
        case ASTKind::DefaultStmt:        return derived().visitDefaultStmt(static_cast<DefaultStmt *>(node));

        case ASTKind::IntLiteral:         return derived().visitIntLiteral(static_cast<IntLiteral *>(node));
        case ASTKind::FloatLiteral:       return derived().visitFloatLiteral(static_cast<FloatLiteral *>(node));
        case ASTKind::CharLiteral:        return derived().visitCharLiteral(static_cast<CharLiteral *>(node));
        case ASTKind::StringLiteral:      return derived().visitStringLiteral(static_cast<StringLiteral *>(node));
        case ASTKind::BoolLiteral:        return derived().visitBoolLiteral(static_cast<BoolLiteral *>(node));
        case ASTKind::NullLiteral:        return derived().visitNullLiteral(static_cast<NullLiteral *>(node));
        case ASTKind::ThisExpr:           return derived().visitThisExpr(static_cast<ThisExpr *>(node));
        case ASTKind::DeclRefExpr:        return derived().visitDeclRefExpr(static_cast<DeclRefExpr *>(node));
        case ASTKind::ParenExpr:          return derived().visitParenExpr(static_cast<ParenExpr *>(node));
        case ASTKind::UnaryExpr:          return derived().visitUnaryExpr(static_cast<UnaryExpr *>(node));
        case ASTKind::BinaryExpr:         return derived().visitBinaryExpr(static_cast<BinaryExpr *>(node));
        case ASTKind::AssignExpr:         return derived().visitAssignExpr(static_cast<AssignExpr *>(node));
        case ASTKind::ConditionalExpr:    return derived().visitConditionalExpr(static_cast<ConditionalExpr *>(node));
        case ASTKind::CallExpr:           return derived().visitCallExpr(static_cast<CallExpr *>(node));
        case ASTKind::ArraySubscriptExpr: return derived().visitArraySubscriptExpr(static_cast<ArraySubscriptExpr *>(node));
        case ASTKind::MemberExpr:         return derived().visitMemberExpr(static_cast<MemberExpr *>(node));
        case ASTKind::NewExpr:            return derived().visitNewExpr(static_cast<NewExpr *>(node));
        case ASTKind::DeleteExpr:         return derived().visitDeleteExpr(static_cast<DeleteExpr *>(node));
        case ASTKind::InitListExpr:       return derived().visitInitListExpr(static_cast<InitListExpr *>(node));
        case ASTKind::LambdaExpr:         return derived().visitLambdaExpr(static_cast<LambdaExpr *>(node));
        case ASTKind::ImplicitCastExpr:   return derived().visitImplicitCastExpr(static_cast<ImplicitCastExpr *>(node));
        }
        return RetTy();
    }

    //family fallbacks
    RetTy visitDecl(Decl *) { return RetTy(); }
    RetTy visitStmt(Stmt *) { return RetTy(); }
    RetTy visitExpr(Expr *) { return RetTy(); }

    RetTy visitProgram(Program *) { return RetTy(); }
    RetTy visitTypeSpec(TypeSpec *) { return RetTy(); }

    RetTy visitVarDecl(VarDecl *node) { return derived().visitDecl(node); }
    RetTy visitParamDecl(ParamDecl *node) { return derived().visitDecl(node); }
    RetTy visitFunctionDecl(FunctionDecl *node) { return derived().visitDecl(node); }
    RetTy visitConstructorDecl(ConstructorDecl *node) { return derived().visitFunctionDecl(node); }
    RetTy visitDestructorDecl(DestructorDecl *node) { return derived().visitFunctionDecl(node); }
    RetTy visitRecordDecl(RecordDecl *node) { return derived().visitDecl(node); }
    RetTy visitAccessSpecDecl(AccessSpecDecl *node) { return derived().visitDecl(node); }
    RetTy visitTypedefDecl(TypedefDecl *node) { return derived().visitDecl(node); }

    RetTy visitCompoundStmt(CompoundStmt *node) { return derived().visitStmt(node); }
    RetTy visitDeclStmt(DeclStmt *node) { return derived().visitStmt(node); }
    RetTy visitExprStmt(ExprStmt *node) { return derived().visitStmt(node); }
    RetTy visitIfStmt(IfStmt *node) { return derived().visitStmt(node); }
    RetTy visitSwitchStmt(SwitchStmt *node) { return derived().visitStmt(node); }
    RetTy visitWhileStmt(WhileStmt *node) { return derived().visitStmt(node); }
    RetTy visitDoStmt(DoStmt *node) { return derived().visitStmt(node); }
    RetTy visitForStmt(ForStmt *node) { return derived().visitStmt(node); }
    RetTy visitReturnStmt(ReturnStmt *node) { return derived().visitStmt(node); }
    RetTy visitBreakStmt(BreakStmt *node) { return derived().visitStmt(node); }
    RetTy visitContinueStmt(ContinueStmt *node) { return derived().visitStmt(node); }
    RetTy visitGotoStmt(GotoStmt *node) { return derived().visitStmt(node); }
    RetTy visitLabelStmt(LabelStmt *node) { return derived().visitStmt(node); }
    RetTy visitCaseStmt(CaseStmt *node) { return derived().visitStmt(node); }
    RetTy visitDefaultStmt(DefaultStmt *node) { return derived().visitStmt(node); }

    RetTy visitIntLiteral(IntLiteral *node) { return derived().visitExpr(node); }
    RetTy visitFloatLiteral(FloatLiteral *node) { return derived().visitExpr(node); }
    RetTy visitCharLiteral(CharLiteral *node) { return derived().visitExpr(node); }
    RetTy visitStringLiteral(StringLiteral *node) { return derived().visitExpr(node); }
    RetTy visitBoolLiteral(BoolLiteral *node) { return derived().visitExpr(node); }
    RetTy visitNullLiteral(NullLiteral *node) { return derived().visitExpr(node); }
    RetTy visitThisExpr(ThisExpr *node) { return derived().visitExpr(node); }
    RetTy visitDeclRefExpr(DeclRefExpr *node) { return derived().visitExpr(node); }
    RetTy visitParenExpr(ParenExpr *node) { return derived().visitExpr(node); }
    RetTy visitUnaryExpr(UnaryExpr *node) { return derived().visitExpr(node); }
    RetTy visitBinaryExpr(BinaryExpr *node) { return derived().visitExpr(node); }
    RetTy visitAssignExpr(AssignExpr *node) { return derived().visitExpr(node); }
    RetTy visitConditionalExpr(ConditionalExpr *node) { return derived().visitExpr(node); }
    RetTy visitCallExpr(CallExpr *node) { return derived().visitExpr(node); }
    RetTy visitArraySubscriptExpr(ArraySubscriptExpr *node) { return derived().visitExpr(node); }
    RetTy visitMemberExpr(MemberExpr *node) { return derived().visitExpr(node); }
    RetTy visitNewExpr(NewExpr *node) { return derived().visitExpr(node); }
    RetTy visitDeleteExpr(DeleteExpr *node) { return derived().visitExpr(node); }
    RetTy visitInitListExpr(InitListExpr *node) { return derived().visitExpr(node); }
    RetTy visitLambdaExpr(LambdaExpr *node) { return derived().visitExpr(node); }
    RetTy visitImplicitCastExpr(ImplicitCastExpr *node) { return derived().visitExpr(node); }

private:
    Derived &derived() { return *static_cast<Derived *>(this); }
};

} // namespace ast

#endif
