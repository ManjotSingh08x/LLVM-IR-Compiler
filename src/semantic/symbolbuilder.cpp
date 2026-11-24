#include "semantic/symbolbuilder.h"

#include "semantic/constants.h"

namespace sema {

namespace {

std::string lengthPrefixed(const std::string &name) { return std::to_string(name.size()) + name; }

//Outer::Inner written for a mangled name: 5Outer5Inner, with its namespaces first: 1N5Outer
//a record declared in a function body is named by its unique mangled name, s.3
std::string recordPath(const RecordSymbol *record) {
    if (record->outer) return recordPath(record->outer) + lengthPrefixed(record->name);
    if (record->scope->kind == ScopeKind::Namespace) return namespacePath(record->scope) + lengthPrefixed(record->name);
    return lengthPrefixed(record->mangledName);
}

//the parts of a qualified name before the last: A::B::f gives A, B
std::vector<std::string> qualifierOf(const ast::QualifiedName &name) {
    return std::vector<std::string>(name.parts.begin(), name.parts.end() - 1);
}

bool isVoidParamList(const ast::NodeList<ast::ParamDecl> &params) {
    if (params.size() != 1 || !params[0]->name.empty()) return false;
    const ast::TypeSpec *type = params[0]->type.get();
    return type->specKind == ast::TypeSpecKind::Builtin && type->name == "void";
}

//a parameter written as an array or a function is a pointer
void bindConstantNames(ast::Expr *expr, Scope *scope);

const Type *adjustParameter(const Type *type, SymbolTable &table) {
    if (type->is(TypeKind::Array) || type->is(TypeKind::Function)) return decay(type, table);
    return type;
}

} // namespace

void SymbolBuilder::build(ast::Program *program) {
    current = table.global();
    program->scope = current;
    declareBuiltins();

    for (const std::unique_ptr<ast::Decl> &decl : program->decls) declareTopLevel(decl.get());
    for (const std::unique_ptr<ast::Decl> &decl : program->decls) buildBodies(decl.get());
}

//a declaration at file scope or directly in a namespace, which current is
void SymbolBuilder::declareTopLevel(ast::Decl *decl) {
    switch (decl->getKind()) {
    case ast::ASTKind::RecordDecl:
        declareRecord(static_cast<ast::RecordDecl *>(decl), current, nullptr);
        break;
    case ast::ASTKind::FunctionDecl:
    case ast::ASTKind::ConstructorDecl:
    case ast::ASTKind::DestructorDecl:
        declareFunction(static_cast<ast::FunctionDecl *>(decl));
        break;
    case ast::ASTKind::VarDecl:
        declareGlobal(static_cast<ast::VarDecl *>(decl));
        break;
    case ast::ASTKind::TypedefDecl:
        declareTypedef(static_cast<ast::TypedefDecl *>(decl), current);
        break;
    case ast::ASTKind::NamespaceDecl:
        declareNamespace(static_cast<ast::NamespaceDecl *>(decl));
        break;
    default:
        break;
    }
}

//namespace N { ... } opened again adds to the same scope
void SymbolBuilder::declareNamespace(ast::NamespaceDecl *node) {
    NamespaceSymbol *space = nullptr;
    if (const std::vector<Symbol *> *found = current->find(node->name)) {
        if ((*found)[0]->kind != SymbolKind::Namespace) {
            diags.error(node->line, node->col, "'" + node->name + "' is already declared as something other than a namespace");
            return;
        }
        space = static_cast<NamespaceSymbol *>((*found)[0]);
    } else {
        space = new NamespaceSymbol(node->name, node->line, node->col);
        space->members = current->addChild(ScopeKind::Namespace, node->line, node->col);
        space->members->space = space;
        current->add(std::unique_ptr<Symbol>(space));
    }
    node->scope = space->members;

    Scope *saved = current;
    current = space->members;
    for (const std::unique_ptr<ast::Decl> &decl : node->decls) declareTopLevel(decl.get());
    current = saved;
}

//pass 1: declarations

void SymbolBuilder::declareBuiltins() {
    const char *names[] = { "printf", "scanf" };
    for (const char *name : names) {
        const FunctionType *signature = table.functionType(intType(), { table.pointerTo(table.constOf(charType())) }, true);
        FunctionSymbol *fn = new FunctionSymbol(name, signature, 0, 0);
        fn->isBuiltin = true;
        fn->loweredType = signature;
        table.global()->add(std::unique_ptr<Symbol>(fn));
        setMangledName(fn, name);
    }
}

void SymbolBuilder::declareRecord(ast::RecordDecl *node, Scope *scope, RecordSymbol *outer) {
    //struct s; declares s once per scope, and the definition in that scope completes the same record
    RecordSymbol *record = nullptr;
    if (const std::vector<Symbol *> *found = scope->find(node->name)) {
        if ((*found)[0]->kind == SymbolKind::Record) {
            RecordSymbol *earlier = static_cast<RecordSymbol *>((*found)[0]);
            if (!node->hasBody) {
                node->symbol = earlier;
                return;
            }
            if (earlier->members == nullptr) record = earlier;
        }
    }
    if (record == nullptr) {
        if (!isFree(scope, node->name, node->line, node->col)) return;
        record = new RecordSymbol(node->name, node->isClass, node->line, node->col);
        record->decl = node;
        record->outer = outer;
        record->type = table.recordType(record);
        //a record in a function body gets a name no other function's record can have
        bool isLocal = scope->kind == ScopeKind::Block || scope->kind == ScopeKind::Function;
        std::string spaces = namespacePrefix(scope);
        for (size_t at = spaces.find("::"); at != std::string::npos; at = spaces.find("::", at)) spaces.replace(at, 2, ".");
        record->mangledName = outer   ? outer->mangledName + "." + node->name
                            : isLocal ? node->name + "." + std::to_string(++localRecordCount)
                                      : spaces + node->name;
        scope->add(std::unique_ptr<Symbol>(record));
    }
    node->symbol = record;
    if (!node->hasBody) return;

    record->decl = node;
    record->members = scope->addChild(ScopeKind::Record, node->line, node->col);
    record->members->record = record;
    node->scope = record->members;

    //a class inherits privately and a struct publicly when no access is written
    for (const ast::BaseSpecifier &spec : node->bases) {
        std::string error;
        const Type *named = spec.qualifier.empty() ? findTypeName(spec.name, scope, node->line, node->col)
                                                   : findQualifiedType(spec.qualifier, spec.name, scope, node->line, node->col, error);
        RecordSymbol *base = named ? asRecord(named) : nullptr;
        if (base == nullptr) {
            diags.error(node->line, node->col, error.empty() ? "base '" + ast::qualifiedSpelling(spec.qualifier, spec.name)
                                                                   + "' is not a declared class or struct"
                                                             : error);
            continue;
        }
        if (base == record || !base->isComplete) {
            diags.error(node->line, node->col, "base '" + spec.name + "' is incomplete");
            continue;
        }
        bool repeated = false;
        for (const BaseClass &earlier : record->bases) repeated = repeated || earlier.record == base;
        if (repeated) {
            diags.error(node->line, node->col, "base '" + spec.name + "' is listed twice");
            continue;
        }
        ast::AccessKind access = spec.access;
        if (access == ast::AccessKind::None) access = node->isClass ? ast::AccessKind::Private : ast::AccessKind::Public;
        record->bases.push_back({ base, access });
    }

    ast::AccessKind access = node->isClass ? ast::AccessKind::Private : ast::AccessKind::Public;
    for (const std::unique_ptr<ast::Decl> &member : node->members) {
        switch (member->getKind()) {
        case ast::ASTKind::AccessSpecDecl:
            access = static_cast<ast::AccessSpecDecl *>(member.get())->access;
            break;
        case ast::ASTKind::RecordDecl: {
            ast::RecordDecl *nested = static_cast<ast::RecordDecl *>(member.get());
            declareRecord(nested, record->members, record);
            if (nested->symbol) nested->symbol->access = access;
            break;
        }
        case ast::ASTKind::TypedefDecl:
            if (Symbol *alias = declareTypedef(static_cast<ast::TypedefDecl *>(member.get()), record->members)) {
                alias->parentRecord = record;
                alias->access = access;
            }
            break;
        case ast::ASTKind::VarDecl: {
            ast::VarDecl *var = static_cast<ast::VarDecl *>(member.get());
            SymbolKind kind = var->isStatic ? SymbolKind::Variable : SymbolKind::Field;
            Symbol *symbol = declareVariable(var, record->members, kind);
            if (symbol == nullptr) break;
            symbol->parentRecord = record;
            symbol->access = access;
            if (asReference(symbol->type)) {
                diags.error(var->line, var->col, "reference member '" + symbol->name + "' is not supported");
            }
            if (var->isStatic) {
                setMangledName(symbol, "class_" + recordPath(record) + "_" + lengthPrefixed(symbol->name));
            } else {
                record->fields.push_back(symbol);
            }
            break;
        }
        case ast::ASTKind::FunctionDecl:
        case ast::ASTKind::ConstructorDecl:
        case ast::ASTKind::DestructorDecl:
            declareMethod(static_cast<ast::FunctionDecl *>(member.get()), record, access);
            break;
        default:
            break;
        }
    }
    record->isComplete = true;
}

void SymbolBuilder::declareMethod(ast::FunctionDecl *node, RecordSymbol *record, ast::AccessKind access) {
    bool isConstructor = node->getKind() == ast::ASTKind::ConstructorDecl;
    bool isDestructor = node->getKind() == ast::ASTKind::DestructorDecl;

    if (node->name.parts.size() > 1) {
        diags.error(node->line, node->col, "a qualified name is not allowed inside a class");
        return;
    }
    std::string name = node->name.parts.back();
    if (isConstructor && name != record->name) {
        diags.error(node->line, node->col, "constructor '" + name + "' must be named after its class '"
                                               + record->name + "'");
        return;
    }
    if (isDestructor && name != "~" + record->name) {
        diags.error(node->line, node->col, "destructor '" + name + "' must be named after its class '"
                                               + record->name + "'");
        return;
    }

    const FunctionType *signature = signatureOf(node, record->members);
    if ((isConstructor || isDestructor) && node->isStatic) {
        diags.error(node->line, node->col, "a constructor or destructor cannot be static");
    }
    if (isDestructor && !signature->params.empty()) {
        diags.error(node->line, node->col, "a destructor takes no parameters");
    }

    //overloads share a list, constructors are compared against the record's
    std::vector<Symbol *> existing;
    if (isConstructor) {
        existing.assign(record->constructors.begin(), record->constructors.end());
    } else if (isDestructor) {
        if (record->destructor) existing.push_back(record->destructor);
    } else if (const std::vector<Symbol *> *found = record->members->find(name)) {
        existing = *found;
        if (!existing.empty() && existing[0]->kind != SymbolKind::Function) {
            diags.error(node->line, node->col, "'" + name + "' is already declared in '"
                                                   + record->qualifiedName() + "'");
            return;
        }
    }
    if (sameSignature(&existing, signature)) {
        diags.error(node->line, node->col, "member function '" + name + "' is already declared with these parameters");
        return;
    }

    FunctionSymbol *fn = new FunctionSymbol(name, signature, node->line, node->col);
    fn->decl = node;
    fn->parentRecord = record;
    fn->access = access;
    fn->isStatic = node->isStatic;
    fn->isConstructor = isConstructor;
    fn->isDestructor = isDestructor;
    if (node->isDefinition()) fn->definition = node;

    //the method becomes a global function that takes the object as this
    if (fn->isMethod()) {
        std::vector<const Type *> params;
        params.push_back(table.pointerTo(record->type));
        params.insert(params.end(), signature->params.begin(), signature->params.end());
        fn->loweredType = table.functionType(signature->returnType, params, false);
    } else {
        fn->loweredType = signature;
    }

    record->members->add(std::unique_ptr<Symbol>(fn), !isConstructor && !isDestructor);
    if (isConstructor) record->constructors.push_back(fn);
    if (isDestructor) record->destructor = fn;
    node->symbol = fn;

    std::string part = isConstructor ? "C" : isDestructor ? "D" : lengthPrefixed(name);
    setMangledName(fn, "class_" + recordPath(record) + "_" + part + "_" + paramCodes(signature));
}

void SymbolBuilder::declareFunction(ast::FunctionDecl *node) {
    if (node->name.parts.size() > 1) {
        QualifierTarget target = resolveQualifier(current, qualifierOf(node->name), node->line, node->col);
        if (!target.error.empty()) {
            diags.error(node->line, node->col, target.error);
        } else if (target.record) {
            defineMethod(node, target.record);
        } else {
            defineNamespaceFunction(node, target);
        }
        return;
    }

    std::string name = node->name.parts[0];
    Scope *scope = current;
    const FunctionType *signature = signatureOf(node, scope);

    const std::vector<Symbol *> *existing = scope->find(name);
    if (existing && (*existing)[0]->kind != SymbolKind::Function) {
        diags.error(node->line, node->col, "'" + name + "' is already declared as something other than a function");
        return;
    }

    //a later declaration or the definition of a function already declared
    if (FunctionSymbol *earlier = sameSignature(existing, signature)) {
        if (earlier->isBuiltin) {
            diags.error(node->line, node->col, "'" + name + "' is a builtin and cannot be redeclared");
        } else if (!isSameType(earlier->signature()->returnType, signature->returnType)) {
            diags.error(node->line, node->col, "'" + name + "' is redeclared with a different return type");
        } else if (node->isDefinition() && earlier->definition) {
            diags.error(node->line, node->col, "redefinition of function '" + name + "'");
        } else {
            if (node->isDefinition()) earlier->definition = node;
            earlier->isStatic = earlier->isStatic || node->isStatic;
            node->symbol = earlier;
        }
        return;
    }

    FunctionSymbol *fn = new FunctionSymbol(name, signature, node->line, node->col);
    fn->decl = node;
    fn->isStatic = node->isStatic;
    fn->loweredType = signature;
    if (node->isDefinition()) fn->definition = node;
    scope->add(std::unique_ptr<Symbol>(fn));
    node->symbol = fn;

    if (name == "main" && scope == table.global()) {
        if (!signature->returnType->is(TypeKind::Int)) diags.error(node->line, node->col, "'main' must return int");
        setMangledName(fn, "main");
    } else {
        setMangledName(fn, "global_" + namespacePath(scope) + lengthPrefixed(name) + "_" + paramCodes(signature));
    }
}

//N::f written outside N: the definition of a function declared in N
void SymbolBuilder::defineNamespaceFunction(ast::FunctionDecl *node, const QualifierTarget &target) {
    std::string name = node->name.parts.back();
    std::string qualified = node->name.str();
    if (node->getKind() != ast::ASTKind::FunctionDecl) {
        diags.error(node->line, node->col, "'" + qualified + "' needs a return type");
        return;
    }
    const FunctionType *signature = signatureOf(node, target.scope);
    FunctionSymbol *declared = sameSignature(target.scope->find(name), signature);
    if (declared == nullptr) {
        diags.error(node->line, node->col, "no function '" + name + "' with these parameters is declared in "
                                               + target.spelling());
        return;
    }
    if (!isSameType(declared->signature()->returnType, signature->returnType)) {
        diags.error(node->line, node->col, "the return type of '" + qualified + "' does not match its declaration");
        return;
    }
    if (declared->definition) {
        diags.error(node->line, node->col, "redefinition of '" + qualified + "'");
        return;
    }
    declared->definition = node;
    node->symbol = declared;
}

//Class::method written outside the class: the definition of a declared member
void SymbolBuilder::defineMethod(ast::FunctionDecl *node, RecordSymbol *record) {
    std::string name = node->name.parts.back();
    std::string qualified = node->name.str();

    bool isConstructor = node->getKind() == ast::ASTKind::ConstructorDecl;
    bool isDestructor = node->getKind() == ast::ASTKind::DestructorDecl;
    if (isConstructor && name != record->name) {
        diags.error(node->line, node->col, "'" + qualified + "' is not a constructor of '" + record->name + "'");
        return;
    }
    if (isDestructor && name != "~" + record->name) {
        diags.error(node->line, node->col, "'" + qualified + "' is not the destructor of '" + record->name + "'");
        return;
    }
    if (node->isStatic) {
        diags.error(node->line, node->col, "'static' is written only on the declaration inside the class");
    }

    const FunctionType *signature = signatureOf(node, record->members);
    std::vector<Symbol *> candidates;
    if (isConstructor) {
        candidates.assign(record->constructors.begin(), record->constructors.end());
    } else if (isDestructor) {
        if (record->destructor) candidates.push_back(record->destructor);
    } else if (const std::vector<Symbol *> *found = record->members->find(name)) {
        candidates = *found;
    }

    FunctionSymbol *declared = sameSignature(&candidates, signature);
    if (declared == nullptr) {
        diags.error(node->line, node->col, "no member function '" + name + "' with these parameters is declared in '"
                                               + record->qualifiedName() + "'");
        return;
    }
    if (!isSameType(declared->signature()->returnType, signature->returnType)) {
        diags.error(node->line, node->col, "the return type of '" + qualified + "' does not match its declaration");
        return;
    }
    if (declared->definition) {
        diags.error(node->line, node->col, "redefinition of '" + qualified + "'");
        return;
    }
    declared->definition = node;
    node->symbol = declared;
}

void SymbolBuilder::declareGlobal(ast::VarDecl *node) {
    if (node->name.parts.size() > 1) {
        QualifierTarget target = resolveQualifier(current, qualifierOf(node->name), node->line, node->col);
        if (!target.error.empty()) {
            diags.error(node->line, node->col, target.error);
        } else if (target.record) {
            defineStaticMember(node, target.record);
        } else {
            diags.error(node->line, node->col, "'" + node->name.str() + "' can only be defined inside "
                                                   + target.spelling());
        }
        return;
    }
    Symbol *symbol = declareVariable(node, current, SymbolKind::Variable);
    if (symbol) setMangledName(symbol, "global_" + namespacePath(current) + lengthPrefixed(symbol->name));
}

//int Counter::total = 0; defines a static member declared in its class
void SymbolBuilder::defineStaticMember(ast::VarDecl *node, RecordSymbol *record) {
    std::string name = node->name.parts.back();
    std::string qualified = node->name.str();

    const std::vector<Symbol *> *found = record->members->find(name);
    Symbol *member = found ? (*found)[0] : nullptr;
    if (member == nullptr) {
        diags.error(node->line, node->col, "no member named '" + name + "' in '" + record->qualifiedName() + "'");
        return;
    }
    if (member->kind == SymbolKind::Field) {
        diags.error(node->line, node->col, "'" + qualified + "' is not static, only a static member can be defined outside its class");
        return;
    }
    if (member->kind != SymbolKind::Variable) {
        diags.error(node->line, node->col, "'" + qualified + "' is not a static data member");
        return;
    }

    const Type *type = resolveType(node->type.get(), record->members, true);
    type = sizeFromInitializer(type, node);
    if (!isError(type) && !isSameType(type, member->type)) {
        diags.error(node->line, node->col, "'" + qualified + "' is defined with type '" + typeToString(type)
                                               + "' but declared with type '" + typeToString(member->type) + "'");
        return;
    }
    if (definedStatics.count(member)) {
        diags.error(node->line, node->col, "redefinition of '" + qualified + "'");
        return;
    }
    definedStatics.insert(member);
    node->symbol = member;
}

Symbol *SymbolBuilder::declareVariable(ast::VarDecl *node, Scope *scope, SymbolKind kind) {
    if (node->name.parts.size() > 1) {
        diags.error(node->line, node->col, "a qualified name is not allowed here");
        return nullptr;
    }
    std::string name = node->name.parts[0];

    const Type *type = resolveType(node->type.get(), scope, true);
    type = sizeFromInitializer(type, node);

    if (type->is(TypeKind::Auto) && node->initStyle == ast::InitStyle::None) {
        diags.error(node->line, node->col, "'" + name + "' is declared auto but has no initializer");
        type = errorType();
    }
    if (kind == SymbolKind::Field && type->is(TypeKind::Auto)) {
        diags.error(node->line, node->col, "a member cannot be declared auto");
        type = errorType();
    }
    if (const ArrayType *array = asArray(type)) {
        if (array->size == 0) {
            diags.error(node->line, node->col, "array '" + name + "' needs a size or an initializer");
            type = errorType();
        }
    }
    if (type->is(TypeKind::Void)) {
        diags.error(node->line, node->col, "'" + name + "' cannot have type void");
        type = errorType();
    }
    if (RecordSymbol *record = asRecord(type)) {
        if (!record->isComplete) {
            diags.error(node->line, node->col, "'" + name + "' has the incomplete type '" + record->qualifiedName() + "'");
            type = errorType();
        }
    }
    if (asReference(type) && kind != SymbolKind::Field && node->initStyle == ast::InitStyle::None) {
        diags.error(node->line, node->col, "reference '" + name + "' must be initialised");
    }

    if (!isFree(scope, name, node->line, node->col)) return nullptr;

    //const int M = N * 2; can size an array before the type checker runs, so M's names are bound now
    if (type->isConst() && isIntegral(type)) {
        bindConstantNames(node->init.get(), scope);
        for (const std::unique_ptr<ast::Expr> &arg : node->ctorArgs) bindConstantNames(arg.get(), scope);
    }

    Symbol *symbol = new Symbol(kind, name, type, node->line, node->col);
    symbol->decl = node;
    symbol->isStatic = node->isStatic;
    scope->add(std::unique_ptr<Symbol>(symbol));
    node->symbol = symbol;
    return symbol;
}

//the aliased type is resolved once here, later passes never see the typedef
Symbol *SymbolBuilder::declareTypedef(ast::TypedefDecl *node, Scope *scope) {
    const Type *type = resolveType(node->type.get(), scope, true);
    if (type->is(TypeKind::Auto)) {
        diags.error(node->line, node->col, "'auto' cannot be used in a typedef");
        type = errorType();
    }
    if (!isFree(scope, node->name, node->line, node->col)) return nullptr;

    Symbol *symbol = new Symbol(SymbolKind::Typedef, node->name, type, node->line, node->col);
    symbol->decl = node;
    scope->add(std::unique_ptr<Symbol>(symbol));
    node->symbol = symbol;
    return symbol;
}

//pass 2: bodies

void SymbolBuilder::buildBodies(ast::Decl *decl) {
    switch (decl->getKind()) {
    case ast::ASTKind::RecordDecl: {
        ast::RecordDecl *record = static_cast<ast::RecordDecl *>(decl);
        if (record->symbol == nullptr) return;
        for (const std::unique_ptr<ast::Decl> &member : record->members) {
            if (member->getKind() == ast::ASTKind::VarDecl) {
                buildInitializer(static_cast<ast::VarDecl *>(member.get()), record->scope);
            } else {
                buildBodies(member.get());
            }
        }
        break;
    }
    case ast::ASTKind::FunctionDecl:
    case ast::ASTKind::ConstructorDecl:
    case ast::ASTKind::DestructorDecl:
        buildFunction(static_cast<ast::FunctionDecl *>(decl));
        break;
    case ast::ASTKind::VarDecl:
        buildInitializer(static_cast<ast::VarDecl *>(decl), current);
        break;
    case ast::ASTKind::NamespaceDecl: {
        ast::NamespaceDecl *space = static_cast<ast::NamespaceDecl *>(decl);
        if (space->scope == nullptr) return;
        Scope *saved = current;
        current = space->scope;
        for (const std::unique_ptr<ast::Decl> &member : space->decls) buildBodies(member.get());
        current = saved;
        break;
    }
    default:
        break;
    }
}

void SymbolBuilder::buildFunction(ast::FunctionDecl *node) {
    FunctionSymbol *fn = node->symbol;
    if (fn == nullptr || !node->isDefinition() || fn->definition != node) return;

    //a body sees its class, or the namespace (or file) its function is declared in
    Scope *parent = fn->parentRecord ? fn->parentRecord->members : fn->scope;
    Scope *scope = parent->addChild(ScopeKind::Function, node->line, node->col);
    scope->function = fn;
    node->scope = scope;

    Scope *saved = current;
    current = scope;
    declareParams(node->params, fn->signature());

    //parameters and the outer block share one scope
    node->body->scope = scope;
    visit(node->body.get());
    current = saved;
}

void SymbolBuilder::declareParams(const ast::NodeList<ast::ParamDecl> &params, const FunctionType *signature) {
    if (signature->params.size() != params.size()) return;  //f(void)
    for (size_t i = 0; i < params.size(); i++) {
        ast::ParamDecl *param = params[i].get();
        if (param->name.empty()) continue;
        if (!isFree(current, param->name, param->line, param->col)) continue;
        //the parameter itself stays const inside the body
        const Type *type = signature->params[i];
        if (param->type->resolved && param->type->resolved->isConst()) type = table.constOf(type);
        Symbol *symbol = new Symbol(SymbolKind::Parameter, param->name, type, param->line, param->col);
        symbol->decl = param;
        current->add(std::unique_ptr<Symbol>(symbol));
        param->symbol = symbol;
    }
}

void SymbolBuilder::buildInitializer(ast::VarDecl *node, Scope *scope) {
    Scope *saved = current;
    current = scope;
    visit(node->init.get());
    for (const std::unique_ptr<ast::Expr> &arg : node->ctorArgs) visit(arg.get());
    current = saved;
}

//statements

void SymbolBuilder::visitCompoundStmt(ast::CompoundStmt *node) {
    if (node->scope == nullptr) node->scope = current->addChild(ScopeKind::Block, node->line, node->col);
    Scope *saved = current;
    current = node->scope;
    for (const std::unique_ptr<ast::Stmt> &stmt : node->body) visit(stmt.get());
    current = saved;
}

void SymbolBuilder::visitDeclStmt(ast::DeclStmt *node) {
    for (const std::unique_ptr<ast::Decl> &decl : node->decls) {
        if (decl->getKind() == ast::ASTKind::TypedefDecl) {
            declareTypedef(static_cast<ast::TypedefDecl *>(decl.get()), current);
            continue;
        }
        if (decl->getKind() == ast::ASTKind::RecordDecl) {
            declareRecord(static_cast<ast::RecordDecl *>(decl.get()), current, nullptr);
            buildBodies(decl.get());
            continue;
        }
        if (decl->getKind() != ast::ASTKind::VarDecl) continue;
        ast::VarDecl *var = static_cast<ast::VarDecl *>(decl.get());
        //the name is in scope from its declarator on
        declareVariable(var, current, SymbolKind::Variable);
        buildInitializer(var, current);
    }
}

void SymbolBuilder::visitExprStmt(ast::ExprStmt *node) { visit(node->expr.get()); }

void SymbolBuilder::visitIfStmt(ast::IfStmt *node) {
    visit(node->cond.get());
    visit(node->thenStmt.get());
    visit(node->elseStmt.get());
}

void SymbolBuilder::visitSwitchStmt(ast::SwitchStmt *node) {
    visit(node->cond.get());
    visit(node->body.get());
}

void SymbolBuilder::visitWhileStmt(ast::WhileStmt *node) {
    visit(node->cond.get());
    visit(node->body.get());
}

void SymbolBuilder::visitDoStmt(ast::DoStmt *node) {
    visit(node->body.get());
    visit(node->cond.get());
}

//the init declarations get a scope of their own around the loop
void SymbolBuilder::visitForStmt(ast::ForStmt *node) {
    node->scope = current->addChild(ScopeKind::Block, node->line, node->col);
    Scope *saved = current;
    current = node->scope;
    visit(node->init.get());
    visit(node->cond.get());
    visit(node->inc.get());
    visit(node->body.get());
    current = saved;
}

void SymbolBuilder::visitReturnStmt(ast::ReturnStmt *node) { visit(node->value.get()); }

//labels belong to the whole function, so a goto may jump forward to one
void SymbolBuilder::visitLabelStmt(ast::LabelStmt *node) {
    Scope *scope = current->functionScope();
    if (scope && scope->labels.count(node->label)) {
        diags.error(node->line, node->col, "label '" + node->label + "' is already defined in this function");
    } else if (scope) {
        scope->labels[node->label] = node;
    }
    visit(node->sub.get());
}

void SymbolBuilder::visitCaseStmt(ast::CaseStmt *node) {
    visit(node->value.get());
    visit(node->sub.get());
}

void SymbolBuilder::visitDefaultStmt(ast::DefaultStmt *node) { visit(node->sub.get()); }

//expressions

void SymbolBuilder::visitParenExpr(ast::ParenExpr *node) { visit(node->sub.get()); }

void SymbolBuilder::visitUnaryExpr(ast::UnaryExpr *node) { visit(node->operand.get()); }

void SymbolBuilder::visitBinaryExpr(ast::BinaryExpr *node) {
    visit(node->lhs.get());
    visit(node->rhs.get());
}

void SymbolBuilder::visitAssignExpr(ast::AssignExpr *node) {
    visit(node->target.get());
    visit(node->value.get());
}

void SymbolBuilder::visitConditionalExpr(ast::ConditionalExpr *node) {
    visit(node->cond.get());
    visit(node->trueExpr.get());
    visit(node->falseExpr.get());
}

void SymbolBuilder::visitCallExpr(ast::CallExpr *node) {
    visit(node->callee.get());
    for (const std::unique_ptr<ast::Expr> &arg : node->args) visit(arg.get());
}

void SymbolBuilder::visitArraySubscriptExpr(ast::ArraySubscriptExpr *node) {
    visit(node->base.get());
    visit(node->index.get());
}

void SymbolBuilder::visitMemberExpr(ast::MemberExpr *node) { visit(node->base.get()); }

void SymbolBuilder::visitNewExpr(ast::NewExpr *node) {
    resolveType(node->type.get(), current, false);
    for (const std::unique_ptr<ast::Expr> &dim : node->dims) visit(dim.get());
    for (const std::unique_ptr<ast::Expr> &arg : node->args) visit(arg.get());
    visit(node->init.get());
}

void SymbolBuilder::visitCastExpr(ast::CastExpr *node) {
    resolveType(node->target.get(), current, false);
    visit(node->sub.get());
}

void SymbolBuilder::visitSizeofExpr(ast::SizeofExpr *node) {
    if (node->typeOperand) resolveType(node->typeOperand.get(), current, false);
    visit(node->exprOperand.get());
}

void SymbolBuilder::visitDeleteExpr(ast::DeleteExpr *node) { visit(node->operand.get()); }

void SymbolBuilder::visitInitListExpr(ast::InitListExpr *node) {
    for (const std::unique_ptr<ast::Expr> &init : node->inits) visit(init.get());
}

void SymbolBuilder::visitLambdaExpr(ast::LambdaExpr *node) {
    Scope *scope = current->addChild(ScopeKind::Lambda, node->line, node->col);
    scope->lambda = node;
    node->scope = scope;

    Scope *saved = current;
    current = scope;
    for (const std::unique_ptr<ast::ParamDecl> &param : node->params) {
        const Type *type = parameterType(param.get(), saved);
        if (param->name.empty() || !isFree(scope, param->name, param->line, param->col)) continue;
        Symbol *symbol = new Symbol(SymbolKind::Parameter, param->name, type, param->line, param->col);
        symbol->decl = param.get();
        scope->add(std::unique_ptr<Symbol>(symbol));
        param->symbol = symbol;
    }
    node->body->scope = scope;
    visit(node->body.get());
    current = saved;
}

//types

const Type *SymbolBuilder::resolveType(ast::TypeSpec *spec, Scope *scope, bool allowUnsized) {
    if (spec == nullptr) return errorType();
    const Type *result = errorType();

    //the part a wrapping type is built from, checked for what may not be in it
    const Type *inner = nullptr;
    if (!spec->isName()) {
        inner = resolveType(spec->inner.get(), scope, false);
        if (inner->is(TypeKind::Auto)) {
            diags.error(spec->line, spec->col, "'auto' must be the whole type of a variable");
            inner = errorType();
        }
    }

    switch (spec->specKind) {
    case ast::TypeSpecKind::Builtin:
        if (spec->name == "int") result = intType();
        else if (spec->name == "char") result = charType();
        else if (spec->name == "signed char") result = scharType();
        else if (spec->name == "unsigned char") result = ucharType();
        else if (spec->name == "short") result = shortType();
        else if (spec->name == "unsigned short") result = ushortType();
        else if (spec->name == "unsigned int") result = uintType();
        else if (spec->name == "long") result = longType();
        else if (spec->name == "unsigned long") result = ulongType();
        else if (spec->name == "float") result = floatType();
        else if (spec->name == "double") result = doubleType();
        else if (spec->name == "bool") result = boolType();
        else if (spec->name == "void") result = voidType();
        else if (spec->name == "auto") result = autoType();
        break;

    case ast::TypeSpecKind::Named: {
        std::string error;
        const Type *named = spec->qualifier.empty()
                                ? findTypeName(spec->name, scope, spec->line, spec->col)
                                : findQualifiedType(spec->qualifier, spec->name, scope, spec->line, spec->col, error);
        if (named) {
            result = named;
        } else {
            diags.error(spec->line, spec->col, error.empty() ? "unknown type name '" + spec->name + "'" : error);
        }
        break;
    }

    //struct X names only a record, never a typedef
    case ast::TypeSpecKind::Struct:
        if (RecordSymbol *record = findRecord(spec->name, scope, spec->line, spec->col)) {
            result = record->type;
        } else {
            diags.error(spec->line, spec->col, "unknown type name '" + spec->name + "'");
        }
        break;

    case ast::TypeSpecKind::Pointer:
        if (asReference(inner)) {
            diags.error(spec->line, spec->col, "a pointer to a reference is not allowed");
        } else if (!isError(inner)) {
            result = table.pointerTo(inner);
        }
        break;

    case ast::TypeSpecKind::Reference:
        if (asReference(inner) || inner->is(TypeKind::Void)) {
            diags.error(spec->line, spec->col, "a reference to '" + typeToString(inner) + "' is not allowed");
        } else if (!isError(inner)) {
            result = table.referenceTo(inner);
        }
        break;

    case ast::TypeSpecKind::Array: {
        if (inner->is(TypeKind::Void) || asReference(inner) || asFunction(inner)) {
            diags.error(spec->line, spec->col, "an array of '" + typeToString(inner) + "' is not allowed");
            break;
        }
        if (asRecord(inner) && !asRecord(inner)->isComplete) {
            diags.error(spec->line, spec->col, "an array of the incomplete type '" + typeToString(inner) + "'");
            break;
        }
        long long size = 0;
        if (spec->size) {
            bindConstantNames(spec->size.get(), scope);
            if (!evaluateConstant(spec->size.get(), size)) {
                diags.error(spec->size->line, spec->size->col, "an array size must be an integer constant");
                break;
            }
            if (size <= 0) {
                diags.error(spec->size->line, spec->size->col, "an array size must be positive");
                break;
            }
        } else if (!allowUnsized) {
            diags.error(spec->line, spec->col, "only the first array dimension may be left empty");
            break;
        }
        if (!isError(inner)) result = table.arrayOf(inner, (uint64_t)size);
        break;
    }

    case ast::TypeSpecKind::Function: {
        if (asArray(inner) || asFunction(inner)) {
            diags.error(spec->line, spec->col, "a function cannot return '" + typeToString(inner) + "'");
            break;
        }
        std::vector<const Type *> params;
        if (!isVoidParamList(spec->params)) {
            for (const std::unique_ptr<ast::ParamDecl> &param : spec->params) {
                params.push_back(unqualified(parameterType(param.get(), scope)));
            }
        }
        result = table.functionType(unqualified(inner), params, false);
        break;
    }
    }

    if (spec->isConst && !isError(result)) result = table.constOf(result);
    spec->resolved = result;
    return result;
}

const Type *SymbolBuilder::parameterType(ast::ParamDecl *param, Scope *scope) {
    const Type *type = resolveType(param->type.get(), scope, true);
    if (type->is(TypeKind::Void) || type->is(TypeKind::Auto)) {
        diags.error(param->line, param->col, "a parameter cannot have type '" + typeToString(type) + "'");
        return errorType();
    }
    return adjustParameter(type, table);
}

const FunctionType *SymbolBuilder::signatureOf(ast::FunctionDecl *node, Scope *scope) {
    const Type *returnType = voidType();
    if (node->returnType) {
        returnType = resolveType(node->returnType.get(), scope, false);
        if (returnType->is(TypeKind::Auto)) {
            diags.error(node->line, node->col, "a function cannot return 'auto'");
            returnType = errorType();
        } else if (asArray(returnType) || asFunction(returnType)) {
            diags.error(node->line, node->col, "a function cannot return '" + typeToString(returnType) + "'");
            returnType = errorType();
        }
    }

    std::vector<const Type *> params;
    if (!isVoidParamList(node->params)) {
        for (const std::unique_ptr<ast::ParamDecl> &param : node->params) {
            params.push_back(unqualified(parameterType(param.get(), scope)));
        }
    }
    //const on a parameter or a returned value is not part of the function's type
    return table.functionType(unqualified(returnType), params, false);
}

//int a[] = {1, 2, 3} and char s[] = "hi" take their size from the initializer
const Type *SymbolBuilder::sizeFromInitializer(const Type *type, ast::VarDecl *node) {
    const ArrayType *array = asArray(type);
    if (array == nullptr || array->size != 0 || node->init == nullptr) return type;

    uint64_t size = 0;
    if (node->init->getKind() == ast::ASTKind::InitListExpr) {
        size = static_cast<ast::InitListExpr *>(node->init.get())->inits.size();
    } else if (node->init->getKind() == ast::ASTKind::StringLiteral && array->element->is(TypeKind::Char)) {
        size = stringLiteralLength(static_cast<ast::StringLiteral *>(node->init.get())->spelling) + 1;
    }
    if (size == 0) return type;

    const Type *sized = table.arrayOf(array->element, size);
    node->type->resolved = sized;
    return sized;
}

namespace {

//bodies are built after every declaration, so outside a class a type name must come before its use
Symbol *visibleTypeSymbol(const std::string &name, Scope *scope, int line, int col, bool recordsOnly) {
    for (; scope != nullptr; scope = scope->parent) {
        const std::vector<Symbol *> *found = scope->find(name);
        if (found == nullptr) continue;
        Symbol *symbol = (*found)[0];
        bool before = symbol->line < line || (symbol->line == line && symbol->col <= col);
        if (scope->kind != ScopeKind::Record && symbol->line != 0 && !before) continue;
        if (symbol->kind == SymbolKind::Record) return symbol;
        if (symbol->kind == SymbolKind::Typedef && !recordsOnly) return symbol;
    }
    return nullptr;
}

//the names in an array size, which is evaluated before the type checker resolves any name: a const
//variable declared before the size can give its value
void bindConstantNames(ast::Expr *expr, Scope *scope) {
    if (expr == nullptr) return;
    switch (expr->getKind()) {
    case ast::ASTKind::DeclRefExpr: {
        ast::DeclRefExpr *ref = static_cast<ast::DeclRefExpr *>(expr);
        if (!ref->qualifier.empty()) {
            //N::SIZE or Box::SIZE: looked up only where the qualifier points
            QualifierTarget target = resolveQualifier(scope, ref->qualifier, ref->line, ref->col);
            const std::vector<Symbol *> *found = target.scope ? target.scope->find(ref->name) : nullptr;
            if (found && (*found)[0]->kind == SymbolKind::Variable) ref->symbol = (*found)[0];
            return;
        }
        for (; scope != nullptr && ref->symbol == nullptr; scope = scope->parent) {
            const std::vector<Symbol *> *found = scope->find(ref->name);
            if (found == nullptr) continue;
            Symbol *symbol = (*found)[0];
            bool before = symbol->line < ref->line || (symbol->line == ref->line && symbol->col < ref->col);
            if (symbol->kind == SymbolKind::Variable && before) ref->symbol = symbol;
            return;
        }
        return;
    }
    case ast::ASTKind::ParenExpr:
        return bindConstantNames(static_cast<ast::ParenExpr *>(expr)->sub.get(), scope);
    case ast::ASTKind::CastExpr:
        return bindConstantNames(static_cast<ast::CastExpr *>(expr)->sub.get(), scope);
    case ast::ASTKind::UnaryExpr:
        return bindConstantNames(static_cast<ast::UnaryExpr *>(expr)->operand.get(), scope);
    case ast::ASTKind::BinaryExpr:
        bindConstantNames(static_cast<ast::BinaryExpr *>(expr)->lhs.get(), scope);
        return bindConstantNames(static_cast<ast::BinaryExpr *>(expr)->rhs.get(), scope);
    case ast::ASTKind::InitListExpr:
        for (const std::unique_ptr<ast::Expr> &item : static_cast<ast::InitListExpr *>(expr)->inits) {
            bindConstantNames(item.get(), scope);
        }
        return;
    default:
        return;
    }
}

} // namespace

RecordSymbol *SymbolBuilder::findRecord(const std::string &name, Scope *scope, int line, int col) {
    return static_cast<RecordSymbol *>(visibleTypeSymbol(name, scope, line, col, true));
}

//the type a plain name stands for: a record, or the type a typedef names
const Type *SymbolBuilder::findTypeName(const std::string &name, Scope *scope, int line, int col) {
    Symbol *symbol = visibleTypeSymbol(name, scope, line, col, false);
    return symbol ? symbol->type : nullptr;
}

//N::Dog: a record or typedef inside the namespace or class the qualifier names, declared before the use
//unless it is a class member
const Type *SymbolBuilder::findQualifiedType(const std::vector<std::string> &qualifier, const std::string &name,
                                             Scope *scope, int line, int col, std::string &error) {
    QualifierTarget target = resolveQualifier(scope, qualifier, line, col);
    if (!target.error.empty()) {
        error = target.error;
        return nullptr;
    }
    if (const std::vector<Symbol *> *found = target.scope->find(name)) {
        Symbol *symbol = (*found)[0];
        bool before = symbol->line < line || (symbol->line == line && symbol->col <= col);
        bool visible = target.record || before;
        if (visible && (symbol->kind == SymbolKind::Record || symbol->kind == SymbolKind::Typedef)) return symbol->type;
    }
    error = "no type named '" + name + "' in " + target.spelling();
    return nullptr;
}

//names

bool SymbolBuilder::isFree(Scope *scope, const std::string &name, int line, int col) {
    if (scope->find(name) == nullptr) return true;
    diags.error(line, col, "redefinition of '" + name + "'");
    return false;
}

FunctionSymbol *SymbolBuilder::sameSignature(const std::vector<Symbol *> *candidates,
                                             const FunctionType *signature) {
    if (candidates == nullptr) return nullptr;
    for (Symbol *candidate : *candidates) {
        if (candidate->kind != SymbolKind::Function) continue;
        const FunctionType *other = static_cast<FunctionSymbol *>(candidate)->signature();
        if (other->params.size() != signature->params.size()) continue;
        bool same = true;
        for (size_t i = 0; i < other->params.size(); i++) {
            same = same && isSameType(other->params[i], signature->params[i]);
        }
        if (same) return static_cast<FunctionSymbol *>(candidate);
    }
    return nullptr;
}

std::string SymbolBuilder::paramCodes(const FunctionType *signature) {
    if (signature->params.empty()) return "v";
    std::string codes;
    for (const Type *param : signature->params) codes += mangleType(param);
    return codes;
}

void SymbolBuilder::setMangledName(Symbol *symbol, const std::string &name) {
    symbol->mangledName = name;
    std::map<std::string, Symbol *>::iterator it = mangledNames.find(name);
    if (it != mangledNames.end() && it->second != symbol) {
        diags.error(symbol->line, symbol->col, "'" + symbol->name + "' has the same mangled name as the symbol at line "
                                                   + std::to_string(it->second->line));
        return;
    }
    mangledNames[name] = symbol;
}

} // namespace sema
