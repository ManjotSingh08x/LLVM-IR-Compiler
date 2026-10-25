#include "ir/ir_visitor.h"

//the passes over the program, the shape of every function body, and globals

namespace ir {

IRVisitor::IRVisitor(Module &module, sema::SymbolTable &symTab, sema::Diagnostics &diags)
    : module(module), symTab(symTab), diags(diags), builder(module), folder(module, layout) {}

void IRVisitor::error(int line, int col, const std::string &message) {
    diags.error(line, col, "cannot generate code: " + message);
}

//pass 1: record types, static members and every function, so a use may come
//before the definition in the file
//pass 2: global variables, with constant initializers where possible and
//@__global_init for the rest
//pass 3: function bodies, then the constructors and destructors the compiler
//writes itself
void IRVisitor::generate(ast::Program *program) {
    collectScope(symTab.global());

    emitGlobalVariables(program);
    emitGlobalInitializers();

    for (const std::unique_ptr<ast::Decl> &decl : program->decls) emitDefinitions(decl.get());
    emitSynthesizedStructors();
}

// ---------------------------------------------------------------------------
// pass 1

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
            //a static member: storage of its own, zero until a definition such
            //as int Counter::total = 0; gives it a value in pass 2
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
    //static on a free function means this file only. on a member it means
    //"no this", and the method stays visible
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

// ---------------------------------------------------------------------------
// pass 2

void IRVisitor::emitGlobalVariables(ast::Program *program) {
    for (const std::unique_ptr<ast::Decl> &decl : program->decls) {
        if (decl->getKind() == ast::ASTKind::VarDecl) emitGlobalVariable(static_cast<ast::VarDecl *>(decl.get()));
    }
}

void IRVisitor::emitGlobalVariable(ast::VarDecl *decl) {
    sema::Symbol *symbol = decl->symbol;
    if (symbol == nullptr) return;
    const sema::Type *type = symbol->type;

    GlobalVariable *global = nullptr;
    if (symbol->parentRecord) {
        //int Counter::total = 0; defines the static member made in pass 1
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

//initializers that are not constants run in @__global_init, which
//@llvm.global_ctors runs before main, in the order they are written.
//objects with destructors are destroyed in the reverse order by @__global_fini.
//__global_init registers it with atexit, as clang does: @llvm.global_dtors is
//ignored by the macos loader, and atexit also runs it before stdout is flushed
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

// ---------------------------------------------------------------------------
// pass 3

//a function may be declared several times; only the declaration that is the
//definition gets a body. methods defined inside their class are found by
//walking into the record
void IRVisitor::emitDefinitions(ast::Decl *decl) {
    switch (decl->getKind()) {
    case ast::ASTKind::RecordDecl: {
        ast::RecordDecl *record = static_cast<ast::RecordDecl *>(decl);
        for (const std::unique_ptr<ast::Decl> &member : record->members) emitDefinitions(member.get());
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

//constructors and destructors that are declared but never defined get a body
//that only builds or destroys the bases and members (as "= default" would).
//records that declare none but still need one get an implicit one when it is
//first used, queued in pendingStructors
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

    //emitting one may ask for another, so the list can grow while it is read
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

// ---------------------------------------------------------------------------
// function bodies

void IRVisitor::startFunction(Function *fn, const sema::Type *returnType) {
    contexts.push_back(std::unique_ptr<FunctionContext>(new FunctionContext()));
    ctx().function = fn;
    ctx().returnType = returnType;
    //the entry block is named before anything else, so no parameter can take
    //the name "entry"
    BasicBlock *entry = fn->createBlock("entry");
    builder.setInsertPoint(fn, entry);
}

void IRVisitor::finishFunction() {
    ctx().function->finalizeCFG(module);
    contexts.pop_back();
}

//every body has the same shape:
//  entry: allocas for this and the parameters, stores of the incoming values
//  constructors: bases and members are built first
//  the statements of the body
//  destructors: members and bases are destroyed last, in reverse order
void IRVisitor::emitFunction(const FunctionInfo &info) {
    Function *fn = info.function;
    startFunction(fn, info.signature->returnType);

    if (info.hasThis) {
        //this arrives as the first argument and lives in a slot like any
        //parameter. it is loaded once, in entry, which dominates every use
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

//each parameter gets a stack slot, as clang -O0 does: %x.addr = alloca i32
//and store i32 %x, ptr %x.addr. the body then reads and writes the slot, so a
//parameter is used exactly like a local variable
void IRVisitor::bindParameters(const sema::FunctionType *signature, ast::NodeList<ast::ParamDecl> *params) {
    Function *fn = ctx().function;
    bool named = params != nullptr && params->size() == signature->params.size();
    for (size_t i = 0; i < signature->params.size(); i++) {
        const sema::Type *type = signature->params[i];
        ast::ParamDecl *decl = named ? (*params)[i].get() : nullptr;
        std::string name = (decl && !decl->name.empty()) ? decl->name : "arg" + std::to_string(i);

        Argument *arg = fn->addArgument(type, name);
        if (decl == nullptr || decl->symbol == nullptr) continue;

        AllocaInst *slot = builder.createEntryAlloca(type, name + ".addr");
        builder.createStore(type, arg, slot);
        VariableBinding binding;
        binding.slot = slot;
        binding.slotHoldsAddress = sema::asReference(type) != nullptr;
        ctx().locals[decl->symbol] = binding;
    }
}

} // namespace ir
