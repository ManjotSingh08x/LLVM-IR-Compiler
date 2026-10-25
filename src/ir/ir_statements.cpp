#include "ir/ir_visitor.h"

//statements, local variables, scopes with destructors to run, and jumps

namespace ir {

// ---- local variables ----

void IRVisitor::emitLocalVariable(ast::VarDecl *decl) {
    sema::Symbol *symbol = decl->symbol;
    if (symbol == nullptr) return;
    if (symbol->isStatic) {
        emitStaticLocal(decl);
        return;
    }

    //the slot is made at the top of entry, the initialization happens here
    const sema::Type *type = symbol->type;
    AllocaInst *slot = builder.createEntryAlloca(type, symbol->name);
    VariableBinding binding;
    binding.slot = slot;
    binding.slotHoldsAddress = sema::asReference(type) != nullptr;
    ctx().locals[symbol] = binding;

    emitObjectInit(slot, type, decl);

    //a reference does not own its object, so it is never destroyed
    if (binding.slotHoldsAddress || ctx().cleanupScopes.empty()) return;
    CleanupScope &scope = ctx().cleanupScopes.back();
    if (layout.needsConstruction(type) || layout.needsDestruction(type)) scope.constructedCount++;
    if (layout.needsDestruction(type)) {
        LocalCleanup cleanup;
        cleanup.address = slot;
        cleanup.type = type;
        scope.objects.push_back(cleanup);
    }
}

//a static local lives in a global, initialised once behind a guard flag
void IRVisitor::emitStaticLocal(ast::VarDecl *decl) {
    sema::Symbol *symbol = decl->symbol;
    const sema::Type *type = symbol->type;
    std::string name = ctx().function->getName() + "." + symbol->name + "." + std::to_string(staticLocalCounter++);

    GlobalVariable *global = module.createGlobal(name, type, module.getZero(type), Linkage::Internal, false);
    VariableBinding binding;
    binding.slot = global;
    binding.slotHoldsAddress = sema::asReference(type) != nullptr;
    globals[symbol] = binding;

    Constant *initial = folder.foldVariable(decl, type);
    if (initial) {
        global->initializer = initial;
        return;
    }

    GlobalVariable *guard = module.createGlobal(name + ".guard", sema::boolType(), module.getBool(false),
                                                Linkage::Internal, false);
    BasicBlock *initBlock = builder.createBlock("static.init");
    BasicBlock *doneBlock = builder.createBlock("static.done");
    Value *done = builder.createLoad(sema::boolType(), guard);
    builder.createCondBr(done, doneBlock, initBlock);

    builder.setInsertBlock(initBlock);
    emitObjectInit(global, type, decl);
    builder.createStore(sema::boolType(), module.getBool(true), guard);
    builder.createBr(doneBlock);

    builder.setInsertBlock(doneBlock);
}

// ---- scopes and cleanups ----

void IRVisitor::pushScope(const ast::Stmt *owner) {
    CleanupScope scope;
    scope.owner = owner;
    ctx().cleanupScopes.push_back(scope);
}

//at the closing brace the objects of the scope are destroyed, newest first
void IRVisitor::popScope() {
    if (builder.isReachable()) emitScopeCleanups(ctx().cleanupScopes.back(), 0);
    ctx().cleanupScopes.pop_back();
}

//destroys the objects of one scope, newest first, keeping the first keep of them
void IRVisitor::emitScopeCleanups(const CleanupScope &scope, size_t keep) {
    for (size_t i = scope.objects.size(); i > keep; i--) {
        emitDestroy(scope.objects[i - 1].address, scope.objects[i - 1].type);
    }
}

//runs destructors for the scopes a jump leaves, without closing them
void IRVisitor::emitCleanupsDownTo(size_t depth) {
    std::vector<CleanupScope> &scopes = ctx().cleanupScopes;
    for (size_t i = scopes.size(); i > depth; i--) emitScopeCleanups(scopes[i - 1], 0);
}

bool IRVisitor::hasActiveCleanups() const {
    const std::vector<CleanupScope> &scopes = contexts.back()->cleanupScopes;
    for (const CleanupScope &scope : scopes) {
        if (!scope.objects.empty()) return true;
    }
    return false;
}

//records, for every label, the enclosing scopes and their object counts
void IRVisitor::collectLabels(ast::Stmt *stmt, std::vector<CleanupScope> &chain) {
    if (stmt == nullptr) return;

    switch (stmt->getKind()) {
    case ast::ASTKind::CompoundStmt: {
        CleanupScope scope;
        scope.owner = stmt;
        chain.push_back(scope);
        for (const std::unique_ptr<ast::Stmt> &child : static_cast<ast::CompoundStmt *>(stmt)->body) {
            collectLabels(child.get(), chain);
        }
        chain.pop_back();
        break;
    }
    case ast::ASTKind::DeclStmt:
        for (const std::unique_ptr<ast::Decl> &decl : static_cast<ast::DeclStmt *>(stmt)->decls) {
            if (decl->getKind() != ast::ASTKind::VarDecl || chain.empty()) continue;
            sema::Symbol *symbol = static_cast<ast::VarDecl *>(decl.get())->symbol;
            if (symbol == nullptr || symbol->isStatic || sema::asReference(symbol->type)) continue;
            if (layout.needsConstruction(symbol->type) || layout.needsDestruction(symbol->type)) {
                chain.back().constructedCount++;
            }
            if (layout.needsDestruction(symbol->type)) {
                LocalCleanup counted;
                counted.address = nullptr;
                counted.type = symbol->type;
                chain.back().objects.push_back(counted);
            }
        }
        break;
    case ast::ASTKind::LabelStmt: {
        ast::LabelStmt *label = static_cast<ast::LabelStmt *>(stmt);
        LabelInfo info;
        for (const CleanupScope &scope : chain) {
            info.owners.push_back(scope.owner);
            info.objectCounts.push_back(scope.objects.size());
            info.constructedCounts.push_back(scope.constructedCount);
        }
        ctx().labels[label] = info;
        collectLabels(label->sub.get(), chain);
        break;
    }
    case ast::ASTKind::IfStmt:
        collectLabels(static_cast<ast::IfStmt *>(stmt)->thenStmt.get(), chain);
        collectLabels(static_cast<ast::IfStmt *>(stmt)->elseStmt.get(), chain);
        break;
    case ast::ASTKind::WhileStmt:
        collectLabels(static_cast<ast::WhileStmt *>(stmt)->body.get(), chain);
        break;
    case ast::ASTKind::DoStmt:
        collectLabels(static_cast<ast::DoStmt *>(stmt)->body.get(), chain);
        break;
    case ast::ASTKind::ForStmt: {
        ast::ForStmt *loop = static_cast<ast::ForStmt *>(stmt);
        CleanupScope scope;
        scope.owner = stmt;
        chain.push_back(scope);
        collectLabels(loop->init.get(), chain);
        collectLabels(loop->body.get(), chain);
        chain.pop_back();
        break;
    }
    case ast::ASTKind::SwitchStmt:
        collectLabels(static_cast<ast::SwitchStmt *>(stmt)->body.get(), chain);
        break;
    case ast::ASTKind::CaseStmt:
        collectLabels(static_cast<ast::CaseStmt *>(stmt)->sub.get(), chain);
        break;
    case ast::ASTKind::DefaultStmt:
        collectLabels(static_cast<ast::DefaultStmt *>(stmt)->sub.get(), chain);
        break;
    default:
        break;
    }
}

BasicBlock *IRVisitor::labelBlock(const ast::LabelStmt *label) {
    std::map<const ast::LabelStmt *, BasicBlock *>::iterator it = ctx().labelBlocks.find(label);
    if (it != ctx().labelBlocks.end()) return it->second;
    BasicBlock *block = builder.createBlock(label->label);
    ctx().labelBlocks[label] = block;
    return block;
}

void IRVisitor::emitJump(const JumpTarget &target) {
    emitCleanupsDownTo(target.cleanupDepth);
    builder.createBr(target.block);
}

// ---- statements ----

Value *IRVisitor::visitCompoundStmt(ast::CompoundStmt *node) {
    pushScope(node);
    for (const std::unique_ptr<ast::Stmt> &stmt : node->body) visit(stmt.get());
    popScope();
    return nullptr;
}

Value *IRVisitor::visitDeclStmt(ast::DeclStmt *node) {
    for (const std::unique_ptr<ast::Decl> &decl : node->decls) {
        if (decl->getKind() == ast::ASTKind::VarDecl) emitLocalVariable(static_cast<ast::VarDecl *>(decl.get()));
    }
    return nullptr;
}

Value *IRVisitor::visitExprStmt(ast::ExprStmt *node) {
    if (node->expr) emitIgnored(node->expr.get());
    return nullptr;
}

Value *IRVisitor::visitIfStmt(ast::IfStmt *node) {
    Value *cond = emitCondition(node->cond.get());
    BasicBlock *thenBlock = builder.createBlock("if.then");
    BasicBlock *elseBlock = node->elseStmt ? builder.createBlock("if.else") : nullptr;
    BasicBlock *endBlock = builder.createBlock("if.end");
    builder.createCondBr(cond, thenBlock, elseBlock ? elseBlock : endBlock);

    builder.setInsertBlock(thenBlock);
    visit(node->thenStmt.get());
    builder.createBr(endBlock);

    if (elseBlock) {
        builder.setInsertBlock(elseBlock);
        visit(node->elseStmt.get());
        builder.createBr(endBlock);
    }
    builder.setInsertBlock(endBlock);
    return nullptr;
}

//continue goes to cond, break to end, until swaps the branch targets
Value *IRVisitor::visitWhileStmt(ast::WhileStmt *node) {
    std::string loop = node->isUntil ? "until" : "while";
    BasicBlock *condBlock = builder.createBlock(loop + ".cond");
    BasicBlock *bodyBlock = builder.createBlock(loop + ".body");
    BasicBlock *endBlock = builder.createBlock(loop + ".end");
    builder.createBr(condBlock);

    builder.setInsertBlock(condBlock);
    Value *cond = emitCondition(node->cond.get());
    if (node->isUntil) {
        builder.createCondBr(cond, endBlock, bodyBlock);
    } else {
        builder.createCondBr(cond, bodyBlock, endBlock);
    }

    builder.setInsertBlock(bodyBlock);
    JumpTarget breakTarget = { endBlock, ctx().cleanupScopes.size() };
    JumpTarget continueTarget = { condBlock, ctx().cleanupScopes.size() };
    ctx().breakTargets.push_back(breakTarget);
    ctx().continueTargets.push_back(continueTarget);
    visit(node->body.get());
    ctx().breakTargets.pop_back();
    ctx().continueTargets.pop_back();
    builder.createBr(condBlock);

    builder.setInsertBlock(endBlock);
    return nullptr;
}

//do-while and do-until: the body runs before the condition
Value *IRVisitor::visitDoStmt(ast::DoStmt *node) {
    BasicBlock *bodyBlock = builder.createBlock("do.body");
    BasicBlock *condBlock = builder.createBlock("do.cond");
    BasicBlock *endBlock = builder.createBlock("do.end");
    builder.createBr(bodyBlock);

    builder.setInsertBlock(bodyBlock);
    JumpTarget breakTarget = { endBlock, ctx().cleanupScopes.size() };
    JumpTarget continueTarget = { condBlock, ctx().cleanupScopes.size() };
    ctx().breakTargets.push_back(breakTarget);
    ctx().continueTargets.push_back(continueTarget);
    visit(node->body.get());
    ctx().breakTargets.pop_back();
    ctx().continueTargets.pop_back();
    builder.createBr(condBlock);

    builder.setInsertBlock(condBlock);
    Value *cond = emitCondition(node->cond.get());
    if (node->isUntil) {
        builder.createCondBr(cond, endBlock, bodyBlock);
    } else {
        builder.createCondBr(cond, bodyBlock, endBlock);
    }

    builder.setInsertBlock(endBlock);
    return nullptr;
}

//init declarations are scoped to the loop, continue goes to for.inc
Value *IRVisitor::visitForStmt(ast::ForStmt *node) {
    pushScope(node);
    visit(node->init.get());

    BasicBlock *condBlock = builder.createBlock("for.cond");
    BasicBlock *bodyBlock = builder.createBlock("for.body");
    BasicBlock *incBlock = builder.createBlock("for.inc");
    BasicBlock *endBlock = builder.createBlock("for.end");
    builder.createBr(condBlock);

    builder.setInsertBlock(condBlock);
    if (node->cond) {
        builder.createCondBr(emitCondition(node->cond.get()), bodyBlock, endBlock);
    } else {
        builder.createBr(bodyBlock);
    }

    builder.setInsertBlock(bodyBlock);
    JumpTarget breakTarget = { endBlock, ctx().cleanupScopes.size() };
    JumpTarget continueTarget = { incBlock, ctx().cleanupScopes.size() };
    ctx().breakTargets.push_back(breakTarget);
    ctx().continueTargets.push_back(continueTarget);
    visit(node->body.get());
    ctx().breakTargets.pop_back();
    ctx().continueTargets.pop_back();
    builder.createBr(incBlock);

    builder.setInsertBlock(incBlock);
    if (node->inc) emitIgnored(node->inc.get());
    builder.createBr(condBlock);

    builder.setInsertBlock(endBlock);
    popScope();
    return nullptr;
}

//the return value is computed before destructors run, via %retval when needed
Value *IRVisitor::visitReturnStmt(ast::ReturnStmt *node) {
    const sema::Type *returnType = ctx().returnType;
    Value *value = nullptr;
    if (node->value) {
        if (returnType->is(sema::TypeKind::Void)) {
            emitIgnored(node->value.get());  //return f(); in a void function
        } else {
            value = emitRValue(node->value.get());
        }
    }

    if (hasActiveCleanups()) {
        if (value) {
            if (ctx().retvalSlot == nullptr) ctx().retvalSlot = builder.createEntryAlloca(returnType, "retval");
            builder.createStore(returnType, value, ctx().retvalSlot);
        }
        emitCleanupsDownTo(0);
        if (value) value = builder.createLoad(returnType, ctx().retvalSlot);
    }

    if (ctx().returnBlock) {
        builder.createBr(ctx().returnBlock);  //a destructor still destroys its members
    } else if (value) {
        builder.createRet(returnType, value);
    } else {
        builder.createRetVoid();
    }
    return nullptr;
}

Value *IRVisitor::visitBreakStmt(ast::BreakStmt *) {
    if (!ctx().breakTargets.empty()) emitJump(ctx().breakTargets.back());
    return nullptr;
}

Value *IRVisitor::visitContinueStmt(ast::ContinueStmt *) {
    if (!ctx().continueTargets.empty()) emitJump(ctx().continueTargets.back());
    return nullptr;
}

//destroys the objects a goto leaves, jumping past a construction is an error
Value *IRVisitor::visitGotoStmt(ast::GotoStmt *node) {
    if (node->target == nullptr) return nullptr;
    BasicBlock *target = labelBlock(node->target);
    const LabelInfo &info = ctx().labels[node->target];
    std::vector<CleanupScope> &scopes = ctx().cleanupScopes;

    size_t common = 0;
    while (common < scopes.size() && common < info.owners.size() && scopes[common].owner == info.owners[common]) {
        common++;
    }

    for (size_t i = common; i < info.owners.size(); i++) {
        if (info.constructedCounts[i] > 0) {
            error(node->line, node->col, "goto '" + node->label + "' jumps into a scope past the construction of an object");
            return nullptr;
        }
    }
    for (size_t i = 0; i < common; i++) {
        if (info.constructedCounts[i] > scopes[i].constructedCount) {
            error(node->line, node->col, "goto '" + node->label + "' jumps forward past the construction of an object");
            return nullptr;
        }
    }

    for (size_t i = scopes.size(); i > 0; i--) {
        size_t keep = (i - 1 < common) ? info.objectCounts[i - 1] : 0;
        emitScopeCleanups(scopes[i - 1], keep);
    }
    builder.createBr(target);
    return nullptr;
}

//a label starts a new block, because a goto may jump to it from anywhere
Value *IRVisitor::visitLabelStmt(ast::LabelStmt *node) {
    BasicBlock *block = labelBlock(node);
    builder.createBr(block);
    builder.setInsertBlock(block);
    visit(node->sub.get());
    return nullptr;
}

//switch: each case label starts a block, falling through with a plain br
Value *IRVisitor::visitSwitchStmt(ast::SwitchStmt *node) {
    Value *cond = emitRValue(node->cond.get());
    BasicBlock *endBlock = builder.createBlock("sw.end");
    SwitchInst *inst = builder.createSwitch(cond, endBlock);

    SwitchContext context;
    context.inst = inst;
    ctx().switches.push_back(context);
    JumpTarget breakTarget = { endBlock, ctx().cleanupScopes.size() };
    ctx().breakTargets.push_back(breakTarget);

    //statements before the first case are unreachable and dropped
    visit(node->body.get());

    ctx().breakTargets.pop_back();
    ctx().switches.pop_back();
    builder.createBr(endBlock);
    builder.setInsertBlock(endBlock);
    return nullptr;
}

Value *IRVisitor::visitCaseStmt(ast::CaseStmt *node) {
    BasicBlock *block = builder.createBlock("sw.case");
    builder.createBr(block);  //fall through from the case above
    builder.setInsertBlock(block);
    if (!ctx().switches.empty()) {
        //the condition was promoted to int, so every case value is an i32
        builder.addSwitchCase(ctx().switches.back().inst, normalizeInt(node->constant, sema::intType()), block);
    }
    visit(node->sub.get());
    return nullptr;
}

Value *IRVisitor::visitDefaultStmt(ast::DefaultStmt *node) {
    BasicBlock *block = builder.createBlock("sw.default");
    builder.createBr(block);
    builder.setInsertBlock(block);
    if (!ctx().switches.empty()) builder.setSwitchDefault(ctx().switches.back().inst, block);
    visit(node->sub.get());
    return nullptr;
}

} // namespace ir
