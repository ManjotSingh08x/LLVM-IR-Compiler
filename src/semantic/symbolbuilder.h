#ifndef SEMANTIC_SYMBOLBUILDER_H
#define SEMANTIC_SYMBOLBUILDER_H

#include <map>
#include <set>
#include <string>

#include "semantic/diagnostics.h"
#include "semantic/symbols.h"
#include "semantic/visitor.h"

namespace sema {

//first semantic pass: builds the scope tree, resolves types and mangles names
class SymbolBuilder : public ast::ASTVisitor<SymbolBuilder> {
public:
    SymbolBuilder(SymbolTable &table, Diagnostics &diags) : table(table), diags(diags) {}

    void build(ast::Program *program);

    //statements: open scopes and declare locals
    void visitCompoundStmt(ast::CompoundStmt *node);
    void visitDeclStmt(ast::DeclStmt *node);
    void visitExprStmt(ast::ExprStmt *node);
    void visitIfStmt(ast::IfStmt *node);
    void visitSwitchStmt(ast::SwitchStmt *node);
    void visitWhileStmt(ast::WhileStmt *node);
    void visitDoStmt(ast::DoStmt *node);
    void visitForStmt(ast::ForStmt *node);
    void visitReturnStmt(ast::ReturnStmt *node);
    void visitLabelStmt(ast::LabelStmt *node);
    void visitCaseStmt(ast::CaseStmt *node);
    void visitDefaultStmt(ast::DefaultStmt *node);

    //expressions: walked only for lambdas and the types in new
    void visitParenExpr(ast::ParenExpr *node);
    void visitUnaryExpr(ast::UnaryExpr *node);
    void visitBinaryExpr(ast::BinaryExpr *node);
    void visitAssignExpr(ast::AssignExpr *node);
    void visitConditionalExpr(ast::ConditionalExpr *node);
    void visitCallExpr(ast::CallExpr *node);
    void visitArraySubscriptExpr(ast::ArraySubscriptExpr *node);
    void visitMemberExpr(ast::MemberExpr *node);
    void visitNewExpr(ast::NewExpr *node);
    void visitDeleteExpr(ast::DeleteExpr *node);
    void visitInitListExpr(ast::InitListExpr *node);
    void visitLambdaExpr(ast::LambdaExpr *node);

private:
    //pass 1
    void declareBuiltins();
    void declareRecord(ast::RecordDecl *node, Scope *scope, RecordSymbol *outer);
    void declareMethod(ast::FunctionDecl *node, RecordSymbol *record, ast::AccessKind access);
    void declareFunction(ast::FunctionDecl *node);
    void defineMethod(ast::FunctionDecl *node);
    void declareGlobal(ast::VarDecl *node);
    Symbol *declareTypedef(ast::TypedefDecl *node, Scope *scope);
    void defineStaticMember(ast::VarDecl *node);
    Symbol *declareVariable(ast::VarDecl *node, Scope *scope, SymbolKind kind);

    //pass 2
    void buildBodies(ast::Decl *decl);
    void buildFunction(ast::FunctionDecl *node);
    void buildInitializer(ast::VarDecl *node, Scope *scope);
    void declareParams(const ast::NodeList<ast::ParamDecl> &params, const FunctionType *signature);

    //types
    const Type         *resolveType(ast::TypeSpec *spec, Scope *scope, bool allowUnsized);
    const Type         *parameterType(ast::ParamDecl *param, Scope *scope);
    const FunctionType *signatureOf(ast::FunctionDecl *node, Scope *scope);
    const Type         *sizeFromInitializer(const Type *type, ast::VarDecl *node);
    RecordSymbol       *findRecord(const std::string &name, Scope *scope);
    const Type         *findTypeName(const std::string &name, Scope *scope);
    RecordSymbol       *findRecordPath(const ast::QualifiedName &name, int line, int col);

    //names
    bool            isFree(Scope *scope, const std::string &name, int line, int col);
    FunctionSymbol *sameSignature(const std::vector<Symbol *> *candidates, const FunctionType *signature);
    std::string     paramCodes(const FunctionType *signature);
    void            setMangledName(Symbol *symbol, const std::string &name);

    SymbolTable &table;
    Diagnostics &diags;
    Scope       *current = nullptr;

    std::map<std::string, Symbol *> mangledNames;    //to catch two symbols given one name
    std::set<Symbol *>              definedStatics;  //static members defined outside their class
};

} // namespace sema

#endif
