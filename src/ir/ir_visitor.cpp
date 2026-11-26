
#include "ir/ir_visitor.h"

namespace ir {

IRVisitor::IRVisitor(Module &module, sema::SymbolTable &symTab, sema::Diagnostics &diags)
    : module(module), symTab(symTab), diags(diags), builder(module), folder(module, layout) {}

void IRVisitor::error(int line, int col, const std::string &message) {
    diags.error(line, col, "cannot generate code: " + message);
}

void IRVisitor::generate(ast::Program *program) {
    collectScope(symTab.global());

    emitGlobalVariables(program);
    emitGlobalInitializers();

    for (const std::unique_ptr<ast::Decl> &decl : program->decls) emitDefinitions(decl.get());

    for (size_t i = 0; i < localRecords.size(); i++) emitDefinitions(localRecords[i]);
    emitSynthesizedStructors();
}

void IRVisitor::collectScope(sema::Scope *scope) {
    for (const std::unique_ptr<sema::Symbol> &symbol : scope->symbols) {
        switch (symbol->kind) {
        case sema::SymbolKind::Record: {
            sema::RecordSymbol *record = static_cast<sema::RecordSymbol *>(symbol.get());
            records.push_back(record);
            module.addRecord(record);
            break;
        }
        case sema::SymbolKind::Function:
            declareFunction(static_cast<sema::FunctionSymbol *>(symbol.get()));
            break;
        case sema::SymbolKind::Variable:

            if (scope->kind == sema::ScopeKind::Record) {
                GlobalVariable *global = module.createGlobal(symbol->mangledName, symbol->type,
                                                             module.getZero(symbol->type), Linkage::External, false);
                VariableBinding binding;
                binding.slot = global;
                binding.slotHoldsAddress = sema::asReference(symbol->type) != nullptr;
                globals[symbol.get()] = binding;
            }
            break;
        default:
            break;
        }
    }
    for (const std::unique_ptr<sema::Scope> &child : scope->children) collectScope(child.get());
}

void IRVisitor::declareFunction(sema::FunctionSymbol *fn) {
    if (module.findFunction(fn->mangledName)) return;

    Linkage linkage = (fn->isStatic && fn->parentRecord == nullptr) ? Linkage::Internal : Linkage::External;
    module.createFunction(fn->mangledName, fn->loweredType, linkage);
}

Function *IRVisitor::functionFor(const sema::FunctionSymbol *fn) {
    Function *function = module.findFunction(fn->mangledName);
    if (function == nullptr) {
        function = module.createFunction(fn->mangledName, fn->loweredType, Linkage::External);
    }
    return function;
}

void IRVisitor::emitGlobalVariables(ast::Program *program) { emitGlobalVariables(program->decls); }

void IRVisitor::emitGlobalVariables(const ast::NodeList<ast::Decl> &decls) {
    for (const std::unique_ptr<ast::Decl> &decl : decls) {
        if (decl->getKind() == ast::ASTKind::VarDecl) emitGlobalVariable(static_cast<ast::VarDecl *>(decl.get()));
        if (decl->getKind() == ast::ASTKind::RecordDecl) emitStaticMemberValues(static_cast<ast::RecordDecl *>(decl.get()));
        if (decl->getKind() == ast::ASTKind::NamespaceDecl) {
            emitGlobalVariables(static_cast<ast::NamespaceDecl *>(decl.get())->decls);
        }
    }
}

void IRVisitor::emitStaticMemberValues(ast::RecordDecl *record) {
    for (const std::unique_ptr<ast::Decl> &member : record->members) {
        if (member->getKind() == ast::ASTKind::RecordDecl) {
            emitStaticMemberValues(static_cast<ast::RecordDecl *>(member.get()));
            continue;
        }
        if (member->getKind() != ast::ASTKind::VarDecl) continue;
        ast::VarDecl *var = static_cast<ast::VarDecl *>(member.get());
        if (var->symbol && var->symbol->isStatic && var->initStyle != ast::InitStyle::None) emitGlobalVariable(var);
    }
}

void IRVisitor::emitGlobalVariable(ast::VarDecl *decl) {
    sema::Symbol *symbol = decl->symbol;
    if (symbol == nullptr) return;
    const sema::Type *type = symbol->type;

    GlobalVariable *global = nullptr;
    if (symbol->parentRecord) {

        if (decl->initStyle == ast::InitStyle::None && !sema::asRecord(type)) return;
        global = module.findGlobal(symbol->mangledName);
    } else {
        Linkage linkage = symbol->isStatic ? Linkage::Internal : Linkage::External;
        global = module.createGlobal(symbol->mangledName, type, module.getZero(type), linkage, false);
        VariableBinding binding;
        binding.slot = global;
        binding.slotHoldsAddress = sema::asReference(type) != nullptr;
        globals[symbol] = binding;
    }
    if (global == nullptr) return;

    Constant *initial = folder.foldVariable(decl, type);
    if (initial) {
        global->initializer = initial;
    } else {
        dynamicGlobals.push_back(decl);
    }
    if (layout.needsDestruction(type)) destroyedGlobals.push_back(decl);
}

void IRVisitor::emitGlobalInitializers() {
    if (dynamicGlobals.empty() && destroyedGlobals.empty()) return;
    const sema::FunctionType *voidFn = symTab.functionType(sema::voidType(), std::vector<const sema::Type *>(), false);

    Function *fini = nullptr;
    if (!destroyedGlobals.empty()) {
        fini = module.createFunction("__global_fini", voidFn, Linkage::Internal);
        startFunction(fini, sema::voidType());
        for (size_t i = destroyedGlobals.size(); i > 0; i--) {
            ast::VarDecl *decl = destroyedGlobals[i - 1];
            emitDestroy(globals[decl->symbol].slot, decl->symbol->type);
        }
        finishFunction();
    }

    module.globalInit = module.createFunction("__global_init", voidFn, Linkage::Internal);
    startFunction(module.globalInit, sema::voidType());
    for (ast::VarDecl *decl : dynamicGlobals) {
        emitObjectInit(globals[decl->symbol].slot, decl->symbol->type, decl);
    }
    if (fini) {
        Function *atexitFn = module.getRuntimeAtexit();
        std::vector<Value *> args;
        args.push_back(fini);
        builder.createCall(atexitFn, atexitFn->fnType, args);
    }
    finishFunction();
}

void IRVisitor::emitDefinitions(ast::Decl *decl) {
    switch (decl->getKind()) {
    case ast::ASTKind::RecordDecl: {
        ast::RecordDecl *record = static_cast<ast::RecordDecl *>(decl);
        for (const std::unique_ptr<ast::Decl> &member : record->members) emitDefinitions(member.get());
        break;
    }
    case ast::ASTKind::NamespaceDecl: {
        ast::NamespaceDecl *space = static_cast<ast::NamespaceDecl *>(decl);
        for (const std::unique_ptr<ast::Decl> &member : space->decls) emitDefinitions(member.get());
        break;
    }
    case ast::ASTKind::FunctionDecl:
    case ast::ASTKind::ConstructorDecl:
    case ast::ASTKind::DestructorDecl: {
        ast::FunctionDecl *node = static_cast<ast::FunctionDecl *>(decl);
        sema::FunctionSymbol *fn = node->symbol;
        if (fn == nullptr || fn->definition != node) return;

        FunctionInfo info;
        info.function = functionFor(fn);
        info.signature = fn->signature();
        info.record = fn->parentRecord;
        info.hasThis = fn->isMethod();
        info.isConstructor = fn->isConstructor;
        info.isDestructor = fn->isDestructor;
        info.definition = node;
        emitFunction(info);
        break;
    }
    default:
        break;
    }
}

void IRVisitor::emitSynthesizedStructors() {
    for (const sema::RecordSymbol *record : records) {
        std::vector<sema::FunctionSymbol *> structors = record->constructors;
        if (record->destructor) structors.push_back(record->destructor);
        for (sema::FunctionSymbol *fn : structors) {
            if (fn->definition != nullptr) continue;
            FunctionInfo info;
            info.function = functionFor(fn);
            info.signature = fn->signature();
            info.record = record;
            info.hasThis = true;
            info.isConstructor = fn->isConstructor;
            info.isDestructor = fn->isDestructor;
            emitFunction(info);
        }
    }

    for (size_t i = 0; i < pendingStructors.size(); i++) {
        PendingStructor pending = pendingStructors[i];
        FunctionInfo info;
        info.function = pending.function;
        info.signature = symTab.functionType(sema::voidType(), std::vector<const sema::Type *>(), false);
        info.record = pending.record;
        info.hasThis = true;
        info.isConstructor = pending.isConstructor;
        info.isDestructor = !pending.isConstructor;
        emitFunction(info);
    }
}

void IRVisitor::startFunction(Function *fn, const sema::Type *returnType) {
    contexts.push_back(std::unique_ptr<FunctionContext>(new FunctionContext()));
    ctx().function = fn;
    ctx().returnType = returnType;

    BasicBlock *entry = fn->createBlock("entry");
    builder.setInsertPoint(fn, entry);
    if (isPassedByAddress(returnType)) ctx().resultAddress = fn->addArgument(module.pointerType(), "agg.result");
}

void IRVisitor::finishFunction() {
    ctx().function->finalizeCFG(module);
    contexts.pop_back();
}

void IRVisitor::emitFunction(const FunctionInfo &info) {
    Function *fn = info.function;
    startFunction(fn, info.signature->returnType);

    if (info.hasThis) {

        const sema::Type *thisType = fn->fnType->params[0];
        Argument *thisArg = fn->addArgument(thisType, "this");
        AllocaInst *slot = builder.createEntryAlloca(thisType, "this.addr");
        builder.createStore(thisType, thisArg, slot);
        ctx().thisPtr = builder.createLoad(thisType, slot);
        ctx().thisRecord = info.record;
    }
    bindParameters(info.signature, info.definition ? &info.definition->params : nullptr);

    if (info.definition) {
        std::vector<CleanupScope> chain;
        collectLabels(info.definition->body.get(), chain);
    }

    if (info.isConstructor) emitMemberInitialization(info.record, ctx().thisPtr);
    if (info.isDestructor) ctx().returnBlock = builder.createBlock("dtor.end");

    if (info.definition) visit(info.definition->body.get());

    if (info.isDestructor) {
        builder.createBr(ctx().returnBlock);
        builder.setInsertBlock(ctx().returnBlock);
        emitMemberDestruction(info.record, ctx().thisPtr);
        builder.createRetVoid();
    }
    finishFunction();
}

void IRVisitor::bindParameters(const sema::FunctionType *signature, ast::NodeList<ast::ParamDecl> *params) {
    Function *fn = ctx().function;
    bool named = params != nullptr && params->size() == signature->params.size();
    for (size_t i = 0; i < signature->params.size(); i++) {
        const sema::Type *type = signature->params[i];
        ast::ParamDecl *decl = named ? (*params)[i].get() : nullptr;
        std::string name = (decl && !decl->name.empty()) ? decl->name : "arg" + std::to_string(i);

        bool byAddress = isPassedByAddress(type);
        Argument *arg = fn->addArgument(byAddress ? module.pointerType() : type, name);
        if (decl == nullptr || decl->symbol == nullptr) continue;

        VariableBinding binding;
        if (byAddress) {
            binding.slot = arg;
        } else {
            AllocaInst *slot = builder.createEntryAlloca(type, name + ".addr");
            builder.createStore(type, arg, slot);
            binding.slot = slot;
            binding.slotHoldsAddress = sema::asReference(type) != nullptr;
        }
        ctx().locals[decl->symbol] = binding;
    }
}

}
