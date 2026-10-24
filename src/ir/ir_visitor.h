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

//where a variable's storage is. slot is an alloca, a global, or a field of a
//lambda's closure. for a reference (and a by-reference capture) the slot does
//not hold the object, it holds the object's address, so it is loaded first
struct VariableBinding {
    Value *slot = nullptr;
    bool   slotHoldsAddress = false;
};

//an object whose destructor must run when its scope ends
struct LocalCleanup {
    Value            *address;
    const sema::Type *type;   //a record, or an array of records
};

//one { } block (or the init part of a for) with the objects declared in it so far
struct CleanupScope {
    const ast::Stmt          *owner = nullptr;
    std::vector<LocalCleanup> objects;
    size_t                    constructedCount = 0;  //objects with a constructor or destructor
};

//where break or continue jumps, and how many cleanup scopes are still open there
struct JumpTarget {
    BasicBlock *block;
    size_t      cleanupDepth;
};

struct SwitchContext {
    SwitchInst *inst;      //null when the switch itself is unreachable
};

//the scopes around a label, worked out before a function is lowered, so that a
//goto (even one above the label) knows which destructors to run
struct LabelInfo {
    std::vector<const ast::Stmt *> owners;
    std::vector<size_t>            objectCounts;       //objects to destroy, per scope, before the label
    std::vector<size_t>            constructedCounts;  //objects constructed, per scope, before the label
};

//everything that belongs to the one function being lowered. a lambda body is
//lowered while its enclosing function is half done, so these are kept on a stack
struct FunctionContext {
    Function                                       *function = nullptr;
    const sema::Type                               *returnType = nullptr;
    Value                                          *thisPtr = nullptr;
    const sema::RecordSymbol                       *thisRecord = nullptr;
    Value                                          *retvalSlot = nullptr;   //see visitReturnStmt
    BasicBlock                                     *returnBlock = nullptr;  //destructors: where return goes
    std::map<const sema::Symbol *, VariableBinding> locals;
    std::map<const ast::LabelStmt *, BasicBlock *>  labelBlocks;
    std::map<const ast::LabelStmt *, LabelInfo>     labels;
    std::vector<JumpTarget>                         breakTargets;     //loops and switches
    std::vector<JumpTarget>                         continueTargets;  //loops only
    std::vector<SwitchContext>                      switches;
    std::vector<CleanupScope>                       cleanupScopes;
};

//what is needed to lower one function body
struct FunctionInfo {
    Function                 *function = nullptr;
    const sema::FunctionType *signature = nullptr;  //the parameters callers see, without this
    const sema::RecordSymbol *record = nullptr;     //for methods, constructors and destructors
    bool                      hasThis = false;
    bool                      isConstructor = false;
    bool                      isDestructor = false;
    ast::FunctionDecl        *definition = nullptr; //null for a body the compiler makes up
};

//a constructor or destructor the compiler writes because the source has none
struct PendingStructor {
    Function                 *function;
    const sema::RecordSymbol *record;
    bool                      isConstructor;
};

//repeat what an object needs for every element of an array
enum class ElementAction { DefaultConstruct, ZeroFill, Destroy };

//what ++x, x++, --x and x-- computed
struct IncDecResult {
    Value *address = nullptr;
    Value *oldValue = nullptr;
    Value *newValue = nullptr;
};

//lowers the checked tree to the ir in a Module. statements are lowered through
//the visitor (visit(stmt)), expressions through emitLValue and emitRValue.
//the work is split by topic over several files:
//  ir_visitor.cpp     the passes, functions, globals
//  ir_statements.cpp  statements, local variables, scopes and cleanups, goto
//  ir_expressions.cpp lvalues, rvalues, operators, casts
//  ir_calls.cpp       calls, methods, lambdas
//  ir_objects.cpp     initialization, construction, destruction, new, delete
class IRVisitor : public ast::ASTVisitor<IRVisitor, Value *> {
public:
    IRVisitor(Module &module, sema::SymbolTable &symTab, sema::Diagnostics &diags);

    void generate(ast::Program *program);

    //statements. they return null
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
    // ---- ir_visitor.cpp: passes, functions and globals ----
    void collectScope(sema::Scope *scope);
    void declareFunction(sema::FunctionSymbol *fn);
    void emitGlobalVariables(ast::Program *program);
    void emitGlobalVariable(ast::VarDecl *decl);
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

    // ---- ir_statements.cpp: statements, locals, scopes ----
    void emitLocalVariable(ast::VarDecl *decl);
    void emitStaticLocal(ast::VarDecl *decl);
    void pushScope(const ast::Stmt *owner);
    void popScope();
    void emitCleanupsDownTo(size_t depth);
    void emitScopeCleanups(const CleanupScope &scope, size_t keep);
    bool hasActiveCleanups() const;
    void collectLabels(ast::Stmt *stmt, std::vector<CleanupScope> &chain);
    BasicBlock *labelBlock(const ast::LabelStmt *label);
    void emitJump(const JumpTarget &target);

    // ---- ir_expressions.cpp: expressions ----
    Value *emitLValue(ast::Expr *expr);
    Value *emitRValue(ast::Expr *expr);
    Value *emitCondition(ast::Expr *expr);
    void   emitIgnored(ast::Expr *expr);
    Value *emitAddressOf(ast::Expr *expr);
    Value *materialize(Value *value, const sema::Type *type);
    Value *emitDeclRefAddress(ast::DeclRefExpr *node);
    Value *variableAddress(const sema::Symbol *symbol);
    Value *emitMemberAddress(ast::MemberExpr *node);
    Value *emitSubscriptAddress(ast::ArraySubscriptExpr *node);
    Value *emitUnary(ast::UnaryExpr *node);
    IncDecResult emitIncDec(ast::UnaryExpr *node);
    Value *emitBinary(ast::BinaryExpr *node);
    Value *emitPointerArithmetic(ast::BinaryExpr *node, Value *lhs, Value *rhs);
    Value *emitPointerDifference(ast::BinaryExpr *node, Value *lhs, Value *rhs);
    Value *emitCompare(ast::BinaryOp op, const sema::Type *operandType, Value *lhs, Value *rhs);
    Value *emitArithmetic(ast::BinaryOp op, const sema::Type *type, Value *lhs, Value *rhs);
    Value *emitShortCircuit(ast::BinaryExpr *node);
    Value *emitAssign(ast::AssignExpr *node);
    Value *emitConditional(ast::ConditionalExpr *node, bool wantAddress);
    Value *emitCast(ast::ImplicitCastExpr *node);
    Value *emitArithmeticConversion(Value *value, const sema::Type *from, const sema::Type *to);
    Value *sizeOf(const sema::Type *type, Value *count);

    // ---- ir_calls.cpp: calls and lambdas ----
    Value *emitCall(ast::CallExpr *node);
    Value *emitDirectCall(ast::CallExpr *node, sema::FunctionSymbol *fn);
    std::vector<Value *> emitArguments(const sema::FunctionType *signature, ast::NodeList<ast::Expr> &args);
    Function *lambdaFunction(ast::LambdaExpr *lambda);
    void      emitLambdaBody(ast::LambdaExpr *lambda);
    void      emitLambdaInto(Value *address, ast::LambdaExpr *lambda);

    // ---- ir_objects.cpp: objects ----
    void   emitObjectInit(Value *address, const sema::Type *type, ast::VarDecl *decl);
    void   emitInitValue(Value *address, const sema::Type *type, ast::Expr *expr);
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
    Function *implicitStructor(const sema::RecordSymbol *record, bool isConstructor);
    Function *destructorFunction(const sema::RecordSymbol *record);
    Value *fieldAddress(Value *object, const sema::RecordSymbol *objectRecord, const sema::Symbol *field);
    Value *adjustToBase(Value *address, const sema::RecordSymbol *from, const sema::RecordSymbol *to,
                        bool keepNull);
    Value *emitNew(ast::NewExpr *node);
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
    std::map<const sema::Symbol *, VariableBinding>  globals;  //globals, static members, static locals
    std::vector<const sema::RecordSymbol *>          records;
    std::vector<ast::VarDecl *>                      dynamicGlobals;   //initialised by @__global_init
    std::vector<ast::VarDecl *>                      destroyedGlobals; //destroyed by @__global_fini
    std::vector<PendingStructor>                     pendingStructors;
    std::set<const ast::LambdaExpr *>                emittedLambdas;
    unsigned                                         staticLocalCounter = 0;
};

} // namespace ir

#endif
