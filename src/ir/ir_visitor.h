#ifndef IR_IR_VISITOR_H
#define IR_IR_VISITOR_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "ir/cfg.h"
#include "ir/constant_folder.h"
#include "ir/ir_builder.h"
#include "ir/record_layout.h"
#include "semantic/diagnostics.h"
#include "semantic/symbols.h"
#include "semantic/visitor.h"

namespace ir {

struct VariableBinding {
    Value *slot = nullptr;
    bool   slotHoldsAddress = false;
};

struct LocalCleanup {
    Value            *address;
    const sema::Type *type;
};

struct CleanupScope {
    const ast::Stmt          *owner = nullptr;
    std::vector<LocalCleanup> objects;
    size_t                    constructedCount = 0;
};

struct JumpTarget {
    BasicBlock *block;
    size_t      cleanupDepth;
};

struct SwitchContext {
    SwitchInst *inst;
};

struct LabelInfo {
    std::vector<const ast::Stmt *> owners;
    std::vector<size_t>            objectCounts;
    std::vector<size_t>            constructedCounts;
};

struct FunctionContext {
    Function                                       *function = nullptr;
    const sema::Type                               *returnType = nullptr;
    Value                                          *thisPtr = nullptr;
    const sema::RecordSymbol                       *thisRecord = nullptr;
    Value                                          *retvalSlot = nullptr;
    Value                                          *resultAddress = nullptr;
    BasicBlock                                     *returnBlock = nullptr;
    std::map<const sema::Symbol *, VariableBinding> locals;
    std::map<const ast::LabelStmt *, BasicBlock *>  labelBlocks;
    std::map<const ast::LabelStmt *, LabelInfo>     labels;
    std::vector<JumpTarget>                         breakTargets;
    std::vector<JumpTarget>                         continueTargets;
    std::vector<SwitchContext>                      switches;
    std::vector<CleanupScope>                       cleanupScopes;
};

struct FunctionInfo {
    Function                 *function = nullptr;
    const sema::FunctionType *signature = nullptr;
    const sema::RecordSymbol *record = nullptr;
    bool                      hasThis = false;
    bool                      isConstructor = false;
    bool                      isDestructor = false;
    ast::FunctionDecl        *definition = nullptr;
};

struct PendingStructor {
    Function                 *function;
    const sema::RecordSymbol *record;
    bool                      isConstructor;
};

enum class ElementAction { DefaultConstruct, ZeroFill, Destroy };

struct IncDecResult {
    Value *address = nullptr;
    Value *oldValue = nullptr;
    Value *newValue = nullptr;
};

class IRVisitor : public ast::ASTVisitor<IRVisitor, Value *> {
public:
    IRVisitor(Module &module, sema::SymbolTable &symTab, sema::Diagnostics &diags);

    void generate(ast::Program *program);

    Value *visitCompoundStmt(ast::CompoundStmt *node);
    Value *visitDeclStmt(ast::DeclStmt *node);
    Value *visitExprStmt(ast::ExprStmt *node);
    Value *visitIfStmt(ast::IfStmt *node);
    Value *visitSwitchStmt(ast::SwitchStmt *node);
    Value *visitWhileStmt(ast::WhileStmt *node);
    Value *visitDoStmt(ast::DoStmt *node);
    Value *visitForStmt(ast::ForStmt *node);
    Value *visitReturnStmt(ast::ReturnStmt *node);
    Value *visitBreakStmt(ast::BreakStmt *node);
    Value *visitContinueStmt(ast::ContinueStmt *node);
    Value *visitGotoStmt(ast::GotoStmt *node);
    Value *visitLabelStmt(ast::LabelStmt *node);
    Value *visitCaseStmt(ast::CaseStmt *node);
    Value *visitDefaultStmt(ast::DefaultStmt *node);

private:
    void collectScope(sema::Scope *scope);
    void declareFunction(sema::FunctionSymbol *fn);
    void emitGlobalVariables(ast::Program *program);
    void emitGlobalVariables(const ast::NodeList<ast::Decl> &decls);
    void emitGlobalVariable(ast::VarDecl *decl);
    void emitStaticMemberValues(ast::RecordDecl *record);
    void emitGlobalInitializers();
    void emitDefinitions(ast::Decl *decl);
    void emitSynthesizedStructors();
    void emitFunction(const FunctionInfo &info);
    void bindParameters(const sema::FunctionType *signature, ast::NodeList<ast::ParamDecl> *params);
    void startFunction(Function *fn, const sema::Type *returnType);
    void finishFunction();
    FunctionContext &ctx() { return *contexts.back(); }
    Function *functionFor(const sema::FunctionSymbol *fn);
    void error(int line, int col, const std::string &message);

    void emitLocalVariable(ast::VarDecl *decl);
    //declares static locals with one-time initialization
    void emitStaticLocal(ast::VarDecl *decl);
    void pushScope(const ast::Stmt *owner);
    void popScope();
    //runs pending destructors down to given depth
    void emitCleanupsDownTo(size_t depth);
    void emitScopeCleanups(const CleanupScope &scope, size_t keep);
    bool hasActiveCleanups() const;
    //pre-scans labels so gotos can find cleanup scopes
    void collectLabels(ast::Stmt *stmt, std::vector<CleanupScope> &chain);
    BasicBlock *labelBlock(const ast::LabelStmt *label);
    void emitJump(const JumpTarget &target);

    //computes the address of an lvalue
    Value *emitLValue(ast::Expr *expr);
    //computes the value of an expression
    Value *emitRValue(ast::Expr *expr);
    //branches straight to targets for && and || conditions
    void   emitBranch(ast::Expr *expr, BasicBlock *ifTrue, BasicBlock *ifFalse);
    void   emitIgnored(ast::Expr *expr);
    Value *emitAddressOf(ast::Expr *expr);
    Value *materialize(Value *value, const sema::Type *type);
    Value *emitDeclRefAddress(ast::DeclRefExpr *node);
    Value *variableAddress(const sema::Symbol *symbol);
    Value *emitMemberAddress(ast::MemberExpr *node);
    Value *emitSubscriptAddress(ast::ArraySubscriptExpr *node);
    Value *emitUnary(ast::UnaryExpr *node);
    //applies ++ or -- giving old and new values
    IncDecResult emitIncDec(ast::UnaryExpr *node);
    Value *emitBinary(ast::BinaryExpr *node);
    Value *emitPointerArithmetic(ast::BinaryExpr *node, Value *lhs, Value *rhs);
    Value *emitPointerDifference(ast::BinaryExpr *node, Value *lhs, Value *rhs);
    Value *emitCompare(ast::BinaryOp op, const sema::Type *operandType, Value *lhs, Value *rhs);
    Value *emitArithmetic(ast::BinaryOp op, const sema::Type *type, Value *lhs, Value *rhs);
    //evaluates && and || with short-circuit control flow
    Value *emitShortCircuit(ast::BinaryExpr *node);
    Value *emitAssign(ast::AssignExpr *node);
    //evaluates ?: using branches or a phi
    Value *emitConditional(ast::ConditionalExpr *node, bool wantAddress);
    //converts values for implicit and explicit casts
    Value *emitCast(ast::ImplicitCastExpr *node);
    Value *emitArithmeticConversion(Value *value, const sema::Type *from, const sema::Type *to);
    Value *sizeOf(const sema::Type *type, Value *count);

    Value *emitCall(ast::CallExpr *node);
    Value *emitDirectCall(ast::CallExpr *node, sema::FunctionSymbol *fn);
    std::vector<Value *> emitArguments(const sema::FunctionType *signature, ast::NodeList<ast::Expr> &args);
    const sema::FunctionType *lambdaSignature(const ast::LambdaExpr *lambda);
    Function *lambdaFunction(ast::LambdaExpr *lambda);
    void      emitLambdaBody(ast::LambdaExpr *lambda);
    void      emitLambdaInto(Value *address, ast::LambdaExpr *lambda);

    //initializes an object from its declaration
    void   emitObjectInit(Value *address, const sema::Type *type, ast::VarDecl *decl);
    void   emitInitValue(Value *address, const sema::Type *type, ast::Expr *expr);
    //initializes arrays and records from braced lists
    void   emitInitList(Value *address, const sema::Type *type, ast::InitListExpr *list);
    void   emitFieldDefaultInit(Value *object, const sema::RecordSymbol *record, const sema::Symbol *field);
    void   emitDefaultConstruct(Value *address, const sema::Type *type, const sema::FunctionSymbol *chosen);
    void   emitConstructorCall(const sema::FunctionSymbol *ctor, Value *address, ast::NodeList<ast::Expr> &args);
    void   emitMemberInitialization(const sema::RecordSymbol *record, Value *thisPtr);
    void   emitMemberDestruction(const sema::RecordSymbol *record, Value *thisPtr);
    void   emitDestroy(Value *address, const sema::Type *type);
    void   emitElementLoop(Value *first, const sema::Type *elementType, Value *begin, Value *end,
                           ElementAction action, const sema::FunctionSymbol *ctor);
    void   emitElementAction(Value *element, const sema::Type *elementType, ElementAction action,
                             const sema::FunctionSymbol *ctor);
    //builds compiler-generated constructor or destructor
    Function *implicitStructor(const sema::RecordSymbol *record, bool isConstructor);
    Function *destructorFunction(const sema::RecordSymbol *record);
    Value *fieldAddress(Value *object, const sema::RecordSymbol *objectRecord, const sema::Symbol *field);
    Value *adjustToBase(Value *address, const sema::RecordSymbol *from, const sema::RecordSymbol *to,
                        bool keepNull);
    //allocates and constructs object for new
    Value *emitNew(ast::NewExpr *node);
    //destroys and frees object for delete
    Value *emitDelete(ast::DeleteExpr *node);
    Value *callMalloc(Value *bytes);
    void   callFree(Value *pointer);

    Module            &module;
    sema::SymbolTable &symTab;
    sema::Diagnostics &diags;
    IRBuilder          builder;
    RecordLayout       layout;
    ConstantFolder     folder;

    std::vector<std::unique_ptr<FunctionContext>>    contexts;
    std::map<const sema::Symbol *, VariableBinding>  globals;
    std::vector<const sema::RecordSymbol *>          records;
    std::vector<ast::VarDecl *>                      dynamicGlobals;
    std::vector<ast::VarDecl *>                      destroyedGlobals;
    std::vector<PendingStructor>                     pendingStructors;
    std::set<const ast::LambdaExpr *>                emittedLambdas;
    std::vector<ast::RecordDecl *>                   localRecords;
    unsigned                                         staticLocalCounter = 0;
};

}

#endif
