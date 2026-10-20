#include "semantic/typecheck.h"

#include "semantic/constants.h"

namespace sema {

namespace {

const char *opSpelling(ast::BinaryOp op) {
    switch (op) {
    case ast::BinaryOp::LogicalOr:  return "||";
    case ast::BinaryOp::LogicalAnd: return "&&";
    case ast::BinaryOp::BitOr:      return "|";
    case ast::BinaryOp::BitXor:     return "^";
    case ast::BinaryOp::BitAnd:     return "&";
    case ast::BinaryOp::Eq:         return "==";
    case ast::BinaryOp::Ne:         return "!=";
    case ast::BinaryOp::Lt:         return "<";
    case ast::BinaryOp::Gt:         return ">";
    case ast::BinaryOp::Le:         return "<=";
    case ast::BinaryOp::Ge:         return ">=";
    case ast::BinaryOp::Shl:        return "<<";
    case ast::BinaryOp::Shr:        return ">>";
    case ast::BinaryOp::Add:        return "+";
    case ast::BinaryOp::Sub:        return "-";
    case ast::BinaryOp::Mul:        return "*";
    case ast::BinaryOp::Div:        return "/";
    case ast::BinaryOp::Rem:        return "%";
    }
    return "?";
}

const char *accessWord(ast::AccessKind access) {
    return access == ast::AccessKind::Private ? "private" : "protected";
}

//a local variable or parameter, which a lambda must capture to use
bool isLocal(const Symbol *symbol) {
    if (symbol->kind == SymbolKind::Parameter) return true;
    if (symbol->kind != SymbolKind::Variable || symbol->isStatic) return false;
    ScopeKind kind = symbol->scope->kind;
    return kind == ScopeKind::Function || kind == ScopeKind::Block || kind == ScopeKind::Lambda;
}

//inside the record or a record nested in it, which may use its private members
bool isWithin(const RecordSymbol *context, const RecordSymbol *record) {
    for (; context != nullptr; context = context->outer) {
        if (context == record) return true;
    }
    return false;
}

bool hasCapture(const ast::LambdaExpr *lambda, ast::LambdaCapture::Kind kind, const std::string &name) {
    for (const ast::LambdaCapture &capture : lambda->usedCaptures) {
        if (capture.kind == kind && capture.name == name) return true;
        if (kind != ast::LambdaCapture::Kind::This && capture.kind != ast::LambdaCapture::Kind::This
            && capture.name == name) {
            return true;
        }
    }
    return false;
}

ast::Expr *withoutParens(ast::Expr *expr) {
    while (expr->getKind() == ast::ASTKind::ParenExpr) expr = static_cast<ast::ParenExpr *>(expr)->sub.get();
    return expr;
}

std::string argumentTypes(const ast::NodeList<ast::Expr> &args) {
    std::string out = "(";
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) out += ", ";
        out += typeToString(args[i]->type);
    }
    return out + ")";
}

} // namespace

void TypeChecker::check(ast::Program *program) {
    current = program->scope;
    for (const std::unique_ptr<ast::Decl> &decl : program->decls) {
        switch (decl->getKind()) {
        case ast::ASTKind::RecordDecl:
            checkRecord(static_cast<ast::RecordDecl *>(decl.get()));
            break;
        case ast::ASTKind::FunctionDecl:
        case ast::ASTKind::ConstructorDecl:
        case ast::ASTKind::DestructorDecl:
            checkFunction(static_cast<ast::FunctionDecl *>(decl.get()));
            break;
        case ast::ASTKind::VarDecl:
            checkVarDecl(static_cast<ast::VarDecl *>(decl.get()));
            break;
        default:
            break;
        }
    }
}

//declarations

void TypeChecker::checkRecord(ast::RecordDecl *node) {
    if (node->scope == nullptr) return;
    for (const std::unique_ptr<ast::Decl> &member : node->members) {
        switch (member->getKind()) {
        case ast::ASTKind::RecordDecl:
            checkRecord(static_cast<ast::RecordDecl *>(member.get()));
            break;
        case ast::ASTKind::FunctionDecl:
        case ast::ASTKind::ConstructorDecl:
        case ast::ASTKind::DestructorDecl:
            checkFunction(static_cast<ast::FunctionDecl *>(member.get()));
            break;
        case ast::ASTKind::VarDecl: {
            //a member initializer, int start{0}, is checked in the record's scope
            Scope *saved = current;
            current = node->scope;
            checkVarDecl(static_cast<ast::VarDecl *>(member.get()));
            current = saved;
            break;
        }
        default:
            break;
        }
    }
}

void TypeChecker::checkFunction(ast::FunctionDecl *node) {
    FunctionSymbol *fn = node->symbol;
    if (fn == nullptr || fn->definition != node || node->scope == nullptr) return;

    FunctionContext saved = context;
    Scope *savedScope = current;
    context = FunctionContext();
    context.function = fn;
    context.returnType = fn->signature()->returnType;
    current = node->scope;

    visit(node->body.get());
    resolveGotos(node->scope);

    context = saved;
    current = savedScope;
}

void TypeChecker::resolveGotos(Scope *functionScope) {
    for (ast::GotoStmt *jump : context.gotos) {
        auto it = functionScope->labels.find(jump->label);
        if (it == functionScope->labels.end()) {
            diags.error(jump->line, jump->col, "use of undeclared label '" + jump->label + "'");
        } else {
            jump->target = it->second;
        }
    }
}

void TypeChecker::checkVarDecl(ast::VarDecl *node) {
    Symbol *symbol = node->symbol;
    if (symbol == nullptr) return;

    //auto takes the type of its initializer, after arrays and functions turn
    //into pointers
    if (symbol->type->is(TypeKind::Auto)) {
        std::unique_ptr<ast::Expr> *value = &node->init;
        if (node->initStyle == ast::InitStyle::Brace) {
            ast::InitListExpr *list = static_cast<ast::InitListExpr *>(node->init.get());
            if (list->inits.size() != 1) {
                diags.error(node->line, node->col, "'auto' needs exactly one value to deduce from");
                symbol->type = errorType();
                return;
            }
            value = &list->inits[0];
        }
        if (node->initStyle == ast::InitStyle::CtorArgs || (*value)->getKind() == ast::ASTKind::InitListExpr) {
            diags.error(node->line, node->col, "'auto' cannot be deduced from a brace list");
            symbol->type = errorType();
            return;
        }
        check(*value);
        const Type *deduced = decayValue(*value);
        if (deduced->is(TypeKind::Void) || deduced->is(TypeKind::Null)) {
            diags.error(node->line, node->col, "'auto' cannot be deduced from a value of type '"
                                                   + typeToString(deduced) + "'");
            deduced = errorType();
        }
        if (node->initStyle == ast::InitStyle::Brace) node->init->type = deduced;
        symbol->type = deduced;
        node->type->resolved = deduced;
        return;
    }

    const Type *type = symbol->type;
    RecordSymbol *record = asRecord(type);
    switch (node->initStyle) {
    case ast::InitStyle::None:
        if (record) {
            node->constructor = construct(record, node->ctorArgs, node->line, node->col);
        } else if (const ArrayType *array = asArray(type)) {
            //every element of an array of objects is default constructed
            while (asArray(array->element)) array = asArray(array->element);
            if (RecordSymbol *element = asRecord(array->element)) {
                node->constructor = construct(element, node->ctorArgs, node->line, node->col);
            }
        }
        break;

    case ast::InitStyle::CtorArgs:
        for (std::unique_ptr<ast::Expr> &arg : node->ctorArgs) check(arg);
        if (record) {
            node->constructor = construct(record, node->ctorArgs, node->line, node->col);
        } else if (node->ctorArgs.size() == 1) {
            convert(node->ctorArgs[0], type, "initialisation");
        } else {
            diags.error(node->line, node->col, "'" + typeToString(type) + "' takes one value to initialise");
        }
        break;

    case ast::InitStyle::Copy:
    case ast::InitStyle::Brace:
        initFromValue(node->init, type);
        if (node->init->getKind() == ast::ASTKind::InitListExpr) {
            node->constructor = static_cast<ast::InitListExpr *>(node->init.get())->constructor;
        }
        break;
    }
}

//initialisation

void TypeChecker::initFromValue(std::unique_ptr<ast::Expr> &value, const Type *type) {
    if (value->getKind() == ast::ASTKind::InitListExpr) {
        initFromList(static_cast<ast::InitListExpr *>(value.get()), type);
        return;
    }

    //a char array may be filled from a string literal that fits, \0 included
    if (const ArrayType *array = asArray(type)) {
        if (value->getKind() == ast::ASTKind::StringLiteral && array->element->is(TypeKind::Char)) {
            check(value);
            const ArrayType *literal = asArray(value->type);
            if (literal->size > array->size) {
                diags.error(value->line, value->col, "the string needs " + std::to_string(literal->size)
                                                         + " characters but the array holds "
                                                         + std::to_string(array->size));
            }
            return;
        }
        check(value);
        if (!isError(value->type)) {
            diags.error(value->line, value->col, "an array must be initialised with a brace list");
        }
        return;
    }

    check(value);
    convert(value, type, "initialisation");
}

//{ ... } for an array, a record or a single value
void TypeChecker::initFromList(ast::InitListExpr *list, const Type *type) {
    list->type = type;
    std::string what = "'" + typeToString(type) + "'";

    if (const ArrayType *array = asArray(type)) {
        if (list->inits.size() > array->size) {
            diags.error(list->line, list->col, "too many initializers for " + what);
        }
        for (std::unique_ptr<ast::Expr> &item : list->inits) initFromValue(item, array->element);
        return;
    }

    if (RecordSymbol *record = asRecord(type)) {
        //a class with constructors takes the list as constructor arguments
        if (!record->constructors.empty()) {
            for (std::unique_ptr<ast::Expr> &item : list->inits) check(item);
            list->constructor = construct(record, list->inits, list->line, list->col);
            return;
        }
        //otherwise the list fills the fields in order
        if (!record->bases.empty()) {
            diags.error(list->line, list->col, "brace initialisation of " + what + ", which has base classes, is not supported");
            return;
        }
        if (list->inits.size() > record->fields.size()) {
            diags.error(list->line, list->col, "too many initializers for " + what);
            return;
        }
        for (size_t i = 0; i < list->inits.size(); i++) initFromValue(list->inits[i], record->fields[i]->type);
        return;
    }

    //a single value: int a{5}, int b{}, Point *p{&q}
    if (list->inits.size() > 1) {
        diags.error(list->line, list->col, "too many initializers for " + what);
        return;
    }
    if (asReference(type) && list->inits.empty()) {
        diags.error(list->line, list->col, "a reference must be bound to an object");
        return;
    }
    if (list->inits.size() == 1) {
        if (list->inits[0]->getKind() == ast::ASTKind::InitListExpr) {
            diags.error(list->inits[0]->line, list->inits[0]->col, "too many braces around the value for " + what);
            return;
        }
        initFromValue(list->inits[0], type);
    }
}

//the constructor that builds a record from these already checked arguments.
//a record without constructors has an implicit default and copy constructor,
//which have no symbol, so null is returned for them
FunctionSymbol *TypeChecker::construct(RecordSymbol *record, ast::NodeList<ast::Expr> &args, int line, int col) {
    if (record->constructors.empty()) {
        if (args.empty()) return nullptr;
        if (args.size() == 1) {
            convert(args[0], record->type, "initialisation");
            return nullptr;
        }
        diags.error(line, col, "'" + record->qualifiedName() + "' has no constructor taking "
                                   + std::to_string(args.size()) + " arguments");
        return nullptr;
    }

    for (std::unique_ptr<ast::Expr> &arg : args) {
        if (arg->getKind() == ast::ASTKind::InitListExpr) {
            diags.error(arg->line, arg->col, "a brace list cannot be passed as a constructor argument");
            return nullptr;
        }
    }

    FunctionSymbol *ctor = chooseOverload(record->constructors, args, record->qualifiedName(), line, col);
    if (ctor == nullptr) return nullptr;
    Lookup lookup;
    lookup.namingRecord = record;
    checkAccess(ctor, lookup, line, col);
    applyArguments(ctor->signature(), args, record->qualifiedName(), line, col);
    return ctor;
}

//names

TypeChecker::Lookup TypeChecker::lookupName(const std::string &name, int line, int col) {
    Lookup result;
    std::vector<Scope *> lambdas;  //the lambdas left on the way out

    for (Scope *scope = current; scope != nullptr; scope = scope->parent) {
        if (scope->kind == ScopeKind::Record) {
            if (lookupMember(scope->record, name, result)) {
                result.namingRecord = scope->record;
                return result;
            }
            continue;
        }

        if (const std::vector<Symbol *> *found = scope->find(name)) {
            //only what is declared before the use, builtins have no position
            for (Symbol *symbol : *found) {
                bool before = symbol->line < line || (symbol->line == line && symbol->col < col);
                if (symbol->line == 0 || before) result.symbols.push_back(symbol);
            }
            if (!result.symbols.empty()) {
                if (isLocal(result.symbols[0]) && !lambdas.empty()) {
                    captureVariable(result.symbols[0], lambdas, line, col);
                }
                return result;
            }
        }
        if (scope->kind == ScopeKind::Lambda) lambdas.push_back(scope);
    }
    return result;
}

//the record's own members first, then each base. a name found through two
//different bases is ambiguous
bool TypeChecker::lookupMember(RecordSymbol *record, const std::string &name, Lookup &result) {
    if (const std::vector<Symbol *> *found = record->members->find(name)) {
        result.symbols = *found;
        result.path.clear();
        return true;
    }

    bool found = false;
    for (const BaseClass &base : record->bases) {
        Lookup inBase;
        if (!lookupMember(base.record, name, inBase)) continue;
        if (found && inBase.symbols[0]->parentRecord != result.symbols[0]->parentRecord) {
            result.ambiguous = true;
            continue;
        }
        if (found) continue;
        found = true;
        result.symbols = inBase.symbols;
        result.ambiguous = inBase.ambiguous;
        result.path.clear();
        result.path.push_back(base);
        result.path.insert(result.path.end(), inBase.path.begin(), inBase.path.end());
    }
    return found;
}

//a local used inside a lambda must be captured by it and by every lambda in
//between, by its capture list or by a [=] or [&] default
void TypeChecker::captureVariable(Symbol *symbol, const std::vector<Scope *> &lambdas, int line, int col) {
    for (Scope *scope : lambdas) {
        ast::LambdaExpr *lambda = scope->lambda;
        if (hasCapture(lambda, ast::LambdaCapture::Kind::ByCopy, symbol->name)) continue;
        if (lambda->captureDefault == ast::CaptureDefault::None) {
            diags.error(line, col, "'" + symbol->name + "' is not captured by the lambda at line "
                                       + std::to_string(lambda->line));
            return;
        }
        ast::LambdaCapture capture;
        capture.kind = lambda->captureDefault == ast::CaptureDefault::ByRef ? ast::LambdaCapture::Kind::ByRef
                                                                            : ast::LambdaCapture::Kind::ByCopy;
        capture.name = symbol->name;
        capture.symbol = symbol;
        lambda->usedCaptures.push_back(capture);
    }
}

//the record this points at here, or null with an error when there is no this:
//outside a method, in a static method, or in a lambda that does not capture it
RecordSymbol *TypeChecker::thisRecord(int line, int col) {
    for (Scope *scope = current; scope != nullptr; scope = scope->parent) {
        if (scope->kind == ScopeKind::Lambda) {
            ast::LambdaExpr *lambda = scope->lambda;
            if (hasCapture(lambda, ast::LambdaCapture::Kind::This, "")) continue;
            if (lambda->captureDefault == ast::CaptureDefault::None) {
                diags.error(line, col, "'this' is not captured by the lambda at line " + std::to_string(lambda->line));
                return nullptr;
            }
            ast::LambdaCapture capture;
            capture.kind = ast::LambdaCapture::Kind::This;
            lambda->usedCaptures.push_back(capture);
            continue;
        }
        if (scope->kind == ScopeKind::Function) {
            if (scope->function && scope->function->isMethod()) return scope->function->parentRecord;
            break;
        }
        if (scope->kind == ScopeKind::Record || scope->kind == ScopeKind::Global) break;
    }
    diags.error(line, col, "'this' can only be used inside a non-static member function");
    return nullptr;
}

//a field or method named without an object belongs to this, which must exist
//and be of the member's record or a record derived from it
bool TypeChecker::memberOfThis(Symbol *member, int line, int col) {
    RecordSymbol *record = thisRecord(line, col);
    if (record == nullptr) return false;
    if (record != member->parentRecord && !isDerivedFrom(record, member->parentRecord)) {
        diags.error(line, col, "'" + member->name + "' belongs to '" + member->parentRecord->qualifiedName()
                                   + "' and needs an object of that class");
        return false;
    }
    return true;
}

//the record whose members the code being checked may see, for access rules
RecordSymbol *TypeChecker::contextRecord() {
    for (Scope *scope = current; scope != nullptr; scope = scope->parent) {
        if (scope->kind == ScopeKind::Record) return scope->record;
    }
    return nullptr;
}

//a private member is usable inside its record, a protected one also inside
//records derived from it. a base inherited as private or protected makes its
//members private or protected in the derived record
void TypeChecker::checkAccess(Symbol *member, const Lookup &lookup, int line, int col) {
    RecordSymbol *owner = member->parentRecord;
    if (owner == nullptr) return;
    RecordSymbol *here = contextRecord();

    bool allowed = true;
    if (member->access == ast::AccessKind::Private) {
        allowed = isWithin(here, owner);
    } else if (member->access == ast::AccessKind::Protected) {
        allowed = isWithin(here, owner) || (here && isDerivedFrom(here, owner));
    }
    if (!allowed) {
        diags.error(line, col, "'" + member->name + "' is a " + accessWord(member->access) + " member of '"
                                   + owner->qualifiedName() + "'");
        return;
    }

    RecordSymbol *derived = lookup.namingRecord;
    for (const BaseClass &base : lookup.path) {
        bool inherited = true;
        if (base.access == ast::AccessKind::Private) {
            inherited = isWithin(here, derived);
        } else if (base.access == ast::AccessKind::Protected) {
            inherited = isWithin(here, derived) || (here && isDerivedFrom(here, derived));
        }
        if (!inherited) {
            diags.error(line, col, "'" + member->name + "' cannot be used here because '" + derived->qualifiedName()
                                       + "' inherits '" + base.record->qualifiedName() + "' as "
                                       + accessWord(base.access));
            return;
        }
        derived = base.record;
    }
}

const Type *TypeChecker::finishDeclRef(ast::DeclRefExpr *node, const Lookup &lookup) {
    if (lookup.symbols.empty()) {
        diags.error(node->line, node->col, "use of undeclared identifier '" + node->name + "'");
        return setType(node, errorType(), false);
    }
    if (lookup.ambiguous) {
        diags.error(node->line, node->col, "'" + node->name + "' is found in more than one base class");
        return setType(node, errorType(), false);
    }

    Symbol *symbol = lookup.symbols[0];
    node->symbol = symbol;
    switch (symbol->kind) {
    case SymbolKind::Record:
        diags.error(node->line, node->col, "'" + node->name + "' is a type, not a value");
        return setType(node, errorType(), false);

    case SymbolKind::Function: {
        FunctionSymbol *fn = static_cast<FunctionSymbol *>(symbol);
        if (lookup.symbols.size() > 1) {
            diags.error(node->line, node->col, "'" + node->name + "' is overloaded, so it can only be called here");
            return setType(node, errorType(), false);
        }
        if (fn->isMethod()) {
            diags.error(node->line, node->col, "the member function '" + node->name + "' must be called");
            return setType(node, errorType(), false);
        }
        checkAccess(fn, lookup, node->line, node->col);
        return setType(node, fn->signature(), true);
    }

    case SymbolKind::Field:
        if (!memberOfThis(symbol, node->line, node->col)) return setType(node, errorType(), false);
        checkAccess(symbol, lookup, node->line, node->col);
        return setType(node, symbol->type, true);

    case SymbolKind::Variable:
    case SymbolKind::Parameter:
        if (symbol->type->is(TypeKind::Auto)) {
            diags.error(node->line, node->col, "'" + node->name + "' is used before its type is deduced");
            return setType(node, errorType(), false);
        }
        checkAccess(symbol, lookup, node->line, node->col);
        return setType(node, stripReference(symbol->type), true);
    }
    return setType(node, errorType(), false);
}

//checks the object of a member access and finds the member in its record
bool TypeChecker::findMember(ast::MemberExpr *node, Lookup &result) {
    const Type *base = check(node->base);
    if (isError(base)) return false;

    const Type *object = base;
    if (node->isArrow) {
        object = decayValue(node->base);
        const PointerType *pointer = asPointer(object);
        object = pointer ? pointer->pointee : nullptr;
    }
    RecordSymbol *record = asRecord(object);
    if (record == nullptr) {
        diags.error(node->line, node->col, std::string("'") + (node->isArrow ? "->" : ".") + node->member
                                               + "' needs " + (node->isArrow ? "a pointer to " : "")
                                               + "a class or struct, not '" + typeToString(base) + "'");
        return false;
    }
    if (!lookupMember(record, node->member, result)) {
        diags.error(node->line, node->col, "no member named '" + node->member + "' in '"
                                               + record->qualifiedName() + "'");
        return false;
    }
    if (result.ambiguous) {
        diags.error(node->line, node->col, "'" + node->member + "' is found in more than one base of '"
                                               + record->qualifiedName() + "'");
        return false;
    }
    result.namingRecord = record;
    return true;
}

const Type *TypeChecker::finishMember(ast::MemberExpr *node, const Lookup &lookup) {
    Symbol *symbol = lookup.symbols[0];
    node->symbol = symbol;
    if (symbol->kind == SymbolKind::Function) {
        diags.error(node->line, node->col, "the member function '" + node->member + "' must be called");
        return setType(node, errorType(), false);
    }
    if (symbol->kind == SymbolKind::Record) {
        diags.error(node->line, node->col, "'" + node->member + "' is a type, not a member value");
        return setType(node, errorType(), false);
    }
    checkAccess(symbol, lookup, node->line, node->col);
    //through a pointer the object is always in memory; with . the member is an
    //lvalue when the object is, and a static member always is
    bool isLValue = node->isArrow || node->base->isLValue || symbol->kind == SymbolKind::Variable;
    return setType(node, symbol->type, isLValue);
}

//calls

//the best candidate for these arguments. a candidate is better than another
//when none of its arguments converts worse and at least one converts better
FunctionSymbol *TypeChecker::chooseOverload(const std::vector<FunctionSymbol *> &candidates,
                                            ast::NodeList<ast::Expr> &args, const std::string &name,
                                            int line, int col) {
    for (const std::unique_ptr<ast::Expr> &arg : args) {
        if (isError(arg->type)) return nullptr;
    }

    std::vector<FunctionSymbol *> viable;
    std::vector<std::vector<ConversionRank>> ranks;
    for (FunctionSymbol *candidate : candidates) {
        const FunctionType *signature = candidate->signature();
        if (args.size() < signature->params.size()) continue;
        if (args.size() > signature->params.size() && !signature->isVariadic) continue;
        std::vector<ConversionRank> rank;
        bool fits = true;
        for (size_t i = 0; i < signature->params.size() && fits; i++) {
            Conversion conversion = findConversion(args[i]->type, args[i]->isLValue, signature->params[i], table);
            fits = conversion.rank != ConversionRank::Impossible;
            rank.push_back(conversion.rank);
        }
        if (!fits) continue;
        viable.push_back(candidate);
        ranks.push_back(rank);
    }

    if (viable.empty()) {
        if (candidates.size() == 1) {
            const FunctionType *signature = candidates[0]->signature();
            if (args.size() != signature->params.size() && !signature->isVariadic) {
                diags.error(line, col, "'" + name + "' takes " + std::to_string(signature->params.size())
                                           + " argument(s) but " + std::to_string(args.size()) + " were given");
                return nullptr;
            }
        }
        diags.error(line, col, "no '" + name + "' matches the arguments " + argumentTypes(args));
        return nullptr;
    }

    for (size_t best = 0; best < viable.size(); best++) {
        bool beatsAll = true;
        for (size_t other = 0; other < viable.size() && beatsAll; other++) {
            if (other == best) continue;
            bool better = false;
            for (size_t i = 0; i < ranks[best].size(); i++) {
                if (ranks[best][i] > ranks[other][i]) beatsAll = false;
                if (ranks[best][i] < ranks[other][i]) better = true;
            }
            if (!better) beatsAll = false;
        }
        if (beatsAll) return viable[best];
    }
    diags.error(line, col, "the call to '" + name + "' with arguments " + argumentTypes(args) + " is ambiguous");
    return nullptr;
}

//converts each argument to its parameter. arguments past the parameters of a
//variadic function get the C promotions instead
void TypeChecker::applyArguments(const FunctionType *signature, ast::NodeList<ast::Expr> &args,
                                 const std::string &name, int line, int col) {
    if (args.size() < signature->params.size() || (args.size() > signature->params.size() && !signature->isVariadic)) {
        diags.error(line, col, "'" + name + "' takes " + std::to_string(signature->params.size())
                                   + " argument(s) but " + std::to_string(args.size()) + " were given");
        return;
    }
    for (size_t i = 0; i < args.size(); i++) {
        if (i < signature->params.size()) {
            convert(args[i], signature->params[i], "argument " + std::to_string(i + 1) + " of '" + name + "'");
            continue;
        }
        const Type *value = decayValue(args[i]);
        if (asRecord(value) || value->is(TypeKind::Void) || asClosure(value)) {
            diags.error(args[i]->line, args[i]->col, "a value of type '" + typeToString(value)
                                                         + "' cannot be passed to '" + name + "'");
        } else if (value->is(TypeKind::Float)) {
            convert(args[i], doubleType(), "argument");
        } else if (isIntegral(value)) {
            convert(args[i], promote(value), "argument");
        }
    }
}

const Type *TypeChecker::callResult(ast::CallExpr *node, const FunctionType *signature) {
    const Type *result = signature->returnType;
    return setType(node, stripReference(result), asReference(result) != nullptr);
}

//values

const Type *TypeChecker::check(std::unique_ptr<ast::Expr> &expr) {
    if (!expr) return errorType();
    const Type *type = visit(expr.get());
    return type ? type : errorType();
}

//uses an array or a function as a pointer, as every operator but & and
//sizeof does
const Type *TypeChecker::decayValue(std::unique_ptr<ast::Expr> &expr) {
    if (expr->type->is(TypeKind::Array)) {
        wrap(expr, ast::CastKind::ArrayToPointer, decay(expr->type, table));
    } else if (expr->type->is(TypeKind::Function)) {
        wrap(expr, ast::CastKind::FunctionToPointer, decay(expr->type, table));
    }
    return expr->type;
}

bool TypeChecker::convert(std::unique_ptr<ast::Expr> &expr, const Type *to, const std::string &what) {
    Conversion conversion = findConversion(expr->type, expr->isLValue, to, table);
    if (conversion.rank == ConversionRank::Impossible) {
        if (asReference(to) && !expr->isLValue && !isError(expr->type)) {
            diags.error(expr->line, expr->col, "a reference in " + what + " must be bound to an object, not a value");
        } else {
            diags.error(expr->line, expr->col, "cannot convert '" + typeToString(expr->type) + "' to '"
                                                   + typeToString(to) + "' in " + what);
        }
        return false;
    }
    bool bindsReference = asReference(to) != nullptr;
    for (const ConversionStep &step : conversion.steps) {
        wrap(expr, step.kind, step.result);
        //a reference binds to the base part of the same object
        if (bindsReference) expr->isLValue = true;
    }
    return true;
}

void TypeChecker::wrap(std::unique_ptr<ast::Expr> &expr, ast::CastKind kind, const Type *type) {
    expr.reset(new ast::ImplicitCastExpr(kind, std::move(expr), type));
}

void TypeChecker::checkCondition(std::unique_ptr<ast::Expr> &cond, const std::string &what) {
    const Type *type = check(cond);
    if (isError(type)) return;
    const Type *value = decayValue(cond);
    if (!isScalar(value)) {
        diags.error(cond->line, cond->col, "a value of type '" + typeToString(value) + "' cannot be used as "
                                               + what);
        return;
    }
    convert(cond, boolType(), what);
}

//something that may be assigned to or incremented
bool TypeChecker::checkModifiable(ast::Expr *expr, const std::string &what) {
    if (isError(expr->type)) return false;
    if (!expr->isLValue || asArray(expr->type) || asFunction(expr->type) || asClosure(expr->type)) {
        diags.error(expr->line, expr->col, "the operand of " + what + " must be a variable that can be changed");
        return false;
    }
    return true;
}

const Type *TypeChecker::setType(ast::Expr *node, const Type *type, bool isLValue) {
    node->type = type;
    node->isLValue = isLValue;
    return type;
}

//statements

const Type *TypeChecker::visitCompoundStmt(ast::CompoundStmt *node) {
    Scope *saved = current;
    if (node->scope) current = node->scope;
    for (const std::unique_ptr<ast::Stmt> &stmt : node->body) visit(stmt.get());
    current = saved;
    return nullptr;
}

const Type *TypeChecker::visitDeclStmt(ast::DeclStmt *node) {
    for (const std::unique_ptr<ast::Decl> &decl : node->decls) {
        if (decl->getKind() == ast::ASTKind::VarDecl) checkVarDecl(static_cast<ast::VarDecl *>(decl.get()));
    }
    return nullptr;
}

const Type *TypeChecker::visitExprStmt(ast::ExprStmt *node) {
    if (node->expr) check(node->expr);
    return nullptr;
}

const Type *TypeChecker::visitIfStmt(ast::IfStmt *node) {
    checkCondition(node->cond, "an if condition");
    visit(node->thenStmt.get());
    visit(node->elseStmt.get());
    return nullptr;
}

const Type *TypeChecker::visitSwitchStmt(ast::SwitchStmt *node) {
    const Type *type = check(node->cond);
    if (!isError(type)) {
        const Type *value = decayValue(node->cond);
        if (isIntegral(value)) {
            convert(node->cond, promote(value), "a switch condition");
        } else {
            diags.error(node->cond->line, node->cond->col, "a switch condition must be an integer, not '"
                                                               + typeToString(value) + "'");
        }
    }

    SwitchInfo info;
    context.switches.push_back(&info);
    context.breakables++;
    visit(node->body.get());
    context.breakables--;
    context.switches.pop_back();
    return nullptr;
}

const Type *TypeChecker::visitWhileStmt(ast::WhileStmt *node) {
    checkCondition(node->cond, "a while condition");
    context.loops++;
    context.breakables++;
    visit(node->body.get());
    context.loops--;
    context.breakables--;
    return nullptr;
}

const Type *TypeChecker::visitDoStmt(ast::DoStmt *node) {
    context.loops++;
    context.breakables++;
    visit(node->body.get());
    context.loops--;
    context.breakables--;
    checkCondition(node->cond, "a do-while condition");
    return nullptr;
}

const Type *TypeChecker::visitForStmt(ast::ForStmt *node) {
    Scope *saved = current;
    if (node->scope) current = node->scope;
    visit(node->init.get());
    if (node->cond) checkCondition(node->cond, "a for condition");
    if (node->inc) check(node->inc);
    context.loops++;
    context.breakables++;
    visit(node->body.get());
    context.loops--;
    context.breakables--;
    current = saved;
    return nullptr;
}

//a lambda takes its return type from its first return, every other return
//must give the same type
const Type *TypeChecker::visitReturnStmt(ast::ReturnStmt *node) {
    if (context.lambda) {
        const Type *type = voidType();
        if (node->value) {
            check(node->value);
            type = decayValue(node->value);
        }
        if (isError(type)) return nullptr;
        if (context.returnType == nullptr) {
            context.returnType = type;
        } else if (!isSameType(context.returnType, type)) {
            diags.error(node->line, node->col, "the lambda returns '" + typeToString(type) + "' here but '"
                                                   + typeToString(context.returnType) + "' before");
        }
        return nullptr;
    }

    std::string name = context.function ? context.function->name : "";
    const Type *expected = context.returnType ? context.returnType : voidType();
    if (node->value == nullptr) {
        if (!expected->is(TypeKind::Void) && !isError(expected)) {
            diags.error(node->line, node->col, "'" + name + "' must return a value of type '"
                                                   + typeToString(expected) + "'");
        }
        return nullptr;
    }

    const Type *type = check(node->value);
    if (expected->is(TypeKind::Void)) {
        if (!type->is(TypeKind::Void) && !isError(type)) {
            diags.error(node->line, node->col, "'" + name + "' returns void and cannot return a value");
        }
        return nullptr;
    }
    convert(node->value, expected, "the return of '" + name + "'");
    return nullptr;
}

const Type *TypeChecker::visitBreakStmt(ast::BreakStmt *node) {
    if (context.breakables == 0) diags.error(node->line, node->col, "'break' outside a loop or switch");
    return nullptr;
}

const Type *TypeChecker::visitContinueStmt(ast::ContinueStmt *node) {
    if (context.loops == 0) diags.error(node->line, node->col, "'continue' outside a loop");
    return nullptr;
}

//a goto may name a label further down, so gotos are resolved once the whole
//function has been seen
const Type *TypeChecker::visitGotoStmt(ast::GotoStmt *node) {
    context.gotos.push_back(node);
    return nullptr;
}

const Type *TypeChecker::visitLabelStmt(ast::LabelStmt *node) {
    visit(node->sub.get());
    return nullptr;
}

const Type *TypeChecker::visitCaseStmt(ast::CaseStmt *node) {
    const Type *type = check(node->value);
    long long value = 0;
    if (context.switches.empty()) {
        diags.error(node->line, node->col, "'case' outside a switch");
    } else if (!isError(type) && (!isIntegral(type) || !evaluateConstant(node->value.get(), value))) {
        diags.error(node->value->line, node->value->col, "a case value must be an integer constant");
    } else if (!isError(type)) {
        node->constant = value;
        SwitchInfo *info = context.switches.back();
        if (info->values.count(value)) {
            diags.error(node->line, node->col, "duplicate case value " + std::to_string(value));
        }
        info->values.insert(value);
    }
    visit(node->sub.get());
    return nullptr;
}

const Type *TypeChecker::visitDefaultStmt(ast::DefaultStmt *node) {
    if (context.switches.empty()) {
        diags.error(node->line, node->col, "'default' outside a switch");
    } else if (context.switches.back()->hasDefault) {
        diags.error(node->line, node->col, "a switch can only have one 'default'");
    } else {
        context.switches.back()->hasDefault = true;
    }
    visit(node->sub.get());
    return nullptr;
}

//expressions

const Type *TypeChecker::visitIntLiteral(ast::IntLiteral *node) {
    long long value;
    if (!intLiteralValue(node->spelling, value)) {
        diags.error(node->line, node->col, "integer literal " + node->spelling + " is too large for int");
    }
    return setType(node, intType(), false);
}

const Type *TypeChecker::visitFloatLiteral(ast::FloatLiteral *node) { return setType(node, doubleType(), false); }

const Type *TypeChecker::visitCharLiteral(ast::CharLiteral *node) { return setType(node, charType(), false); }

//a string literal is an array of its characters and the closing \0
const Type *TypeChecker::visitStringLiteral(ast::StringLiteral *node) {
    uint64_t size = stringLiteralLength(node->spelling) + 1;
    return setType(node, table.arrayOf(charType(), size), true);
}

const Type *TypeChecker::visitBoolLiteral(ast::BoolLiteral *node) { return setType(node, boolType(), false); }

const Type *TypeChecker::visitNullLiteral(ast::NullLiteral *node) { return setType(node, nullType(), false); }

const Type *TypeChecker::visitThisExpr(ast::ThisExpr *node) {
    RecordSymbol *record = thisRecord(node->line, node->col);
    if (record == nullptr) return setType(node, errorType(), false);
    return setType(node, table.pointerTo(record->type), false);
}

const Type *TypeChecker::visitDeclRefExpr(ast::DeclRefExpr *node) {
    return finishDeclRef(node, lookupName(node->name, node->line, node->col));
}

const Type *TypeChecker::visitParenExpr(ast::ParenExpr *node) {
    const Type *type = check(node->sub);
    return setType(node, type, node->sub->isLValue);
}

const Type *TypeChecker::visitUnaryExpr(ast::UnaryExpr *node) {
    const Type *type = check(node->operand);
    if (isError(type)) return setType(node, errorType(), false);

    switch (node->op) {
    case ast::UnaryOp::PreInc:
    case ast::UnaryOp::PreDec:
    case ast::UnaryOp::PostInc:
    case ast::UnaryOp::PostDec: {
        std::string what = (node->op == ast::UnaryOp::PreInc || node->op == ast::UnaryOp::PostInc) ? "'++'" : "'--'";
        if (!checkModifiable(node->operand.get(), what)) return setType(node, errorType(), false);
        if (type->is(TypeKind::Bool) || !(isArithmetic(type) || isObjectPointer(type))) {
            diags.error(node->line, node->col, "cannot apply " + what + " to '" + typeToString(type) + "'");
            return setType(node, errorType(), false);
        }
        return setType(node, type, !node->isPostfix());
    }

    case ast::UnaryOp::Plus:
    case ast::UnaryOp::Minus: {
        if (!isArithmetic(type)) {
            diags.error(node->line, node->col, "unary '" + std::string(node->op == ast::UnaryOp::Plus ? "+" : "-")
                                                   + "' needs a number, not '" + typeToString(type) + "'");
            return setType(node, errorType(), false);
        }
        const Type *result = promote(type);
        convert(node->operand, result, "a unary operator");
        return setType(node, result, false);
    }

    case ast::UnaryOp::LogicalNot:
        checkCondition(node->operand, "the operand of '!'");
        return setType(node, boolType(), false);

    case ast::UnaryOp::BitNot:
        if (!isIntegral(type)) {
            diags.error(node->line, node->col, "'~' needs an integer, not '" + typeToString(type) + "'");
            return setType(node, errorType(), false);
        }
        convert(node->operand, promote(type), "'~'");
        return setType(node, promote(type), false);

    case ast::UnaryOp::Deref: {
        const Type *value = decayValue(node->operand);
        const PointerType *pointer = asPointer(value);
        if (pointer == nullptr || pointer->pointee->is(TypeKind::Void)) {
            diags.error(node->line, node->col, "cannot dereference a value of type '" + typeToString(value) + "'");
            return setType(node, errorType(), false);
        }
        return setType(node, pointer->pointee, true);
    }

    case ast::UnaryOp::AddrOf:
        if (!node->operand->isLValue) {
            diags.error(node->line, node->col, "cannot take the address of a value that is not stored in a variable");
            return setType(node, errorType(), false);
        }
        return setType(node, table.pointerTo(type), false);
    }
    return setType(node, errorType(), false);
}

const Type *TypeChecker::visitBinaryExpr(ast::BinaryExpr *node) {
    const Type *left = check(node->lhs);
    const Type *right = check(node->rhs);
    if (isError(left) || isError(right)) return setType(node, errorType(), false);

    if (node->op == ast::BinaryOp::LogicalAnd || node->op == ast::BinaryOp::LogicalOr) {
        std::string what = std::string("an operand of '") + opSpelling(node->op) + "'";
        const Type *l = decayValue(node->lhs);
        const Type *r = decayValue(node->rhs);
        if (!isScalar(l) || !isScalar(r)) {
            diags.error(node->line, node->col, "invalid operands to '" + std::string(opSpelling(node->op)) + "' ('"
                                                   + typeToString(l) + "' and '" + typeToString(r) + "')");
            return setType(node, errorType(), false);
        }
        convert(node->lhs, boolType(), what);
        convert(node->rhs, boolType(), what);
        return setType(node, boolType(), false);
    }

    left = decayValue(node->lhs);
    right = decayValue(node->rhs);
    std::string invalid = "invalid operands to '" + std::string(opSpelling(node->op)) + "' ('"
                        + typeToString(left) + "' and '" + typeToString(right) + "')";

    //both numbers: convert to the common type
    if (isArithmetic(left) && isArithmetic(right)) {
        const Type *common = commonArithmeticType(promote(left), promote(right));
        switch (node->op) {
        case ast::BinaryOp::Rem:
        case ast::BinaryOp::BitAnd:
        case ast::BinaryOp::BitOr:
        case ast::BinaryOp::BitXor:
        case ast::BinaryOp::Shl:
        case ast::BinaryOp::Shr:
            if (!isIntegral(left) || !isIntegral(right)) {
                diags.error(node->line, node->col, invalid);
                return setType(node, errorType(), false);
            }
            break;
        default:
            break;
        }
        convert(node->lhs, common, "an operand");
        convert(node->rhs, common, "an operand");
        switch (node->op) {
        case ast::BinaryOp::Eq:
        case ast::BinaryOp::Ne:
        case ast::BinaryOp::Lt:
        case ast::BinaryOp::Gt:
        case ast::BinaryOp::Le:
        case ast::BinaryOp::Ge:
            return setType(node, boolType(), false);
        default:
            return setType(node, common, false);
        }
    }

    switch (node->op) {
    //pointer arithmetic: pointer + int, int + pointer, pointer - int and the
    //distance between two pointers
    case ast::BinaryOp::Add:
        if (isObjectPointer(left) && isIntegral(right)) {
            convert(node->rhs, intType(), "an operand");
            return setType(node, left, false);
        }
        if (isIntegral(left) && isObjectPointer(right)) {
            convert(node->lhs, intType(), "an operand");
            return setType(node, right, false);
        }
        break;
    case ast::BinaryOp::Sub:
        if (isObjectPointer(left) && isIntegral(right)) {
            convert(node->rhs, intType(), "an operand");
            return setType(node, left, false);
        }
        if (isObjectPointer(left) && isObjectPointer(right) && isSameType(left, right)) {
            return setType(node, intType(), false);
        }
        break;

    //pointers compare with pointers to the same type, to a base, to void or
    //to null. one side is converted to the other
    case ast::BinaryOp::Eq:
    case ast::BinaryOp::Ne:
    case ast::BinaryOp::Lt:
    case ast::BinaryOp::Gt:
    case ast::BinaryOp::Le:
    case ast::BinaryOp::Ge: {
        bool pointers = (asPointer(left) || left->is(TypeKind::Null)) && (asPointer(right) || right->is(TypeKind::Null));
        if (!pointers) break;
        if (isSameType(left, right)) return setType(node, boolType(), false);
        if (findConversion(right, false, left, table).rank != ConversionRank::Impossible) {
            convert(node->rhs, left, "a comparison");
            return setType(node, boolType(), false);
        }
        if (findConversion(left, false, right, table).rank != ConversionRank::Impossible) {
            convert(node->lhs, right, "a comparison");
            return setType(node, boolType(), false);
        }
        break;
    }
    default:
        break;
    }

    diags.error(node->line, node->col, invalid);
    return setType(node, errorType(), false);
}

const Type *TypeChecker::visitAssignExpr(ast::AssignExpr *node) {
    const Type *target = check(node->target);
    const Type *value = check(node->value);
    if (isError(target) || isError(value)) return setType(node, errorType(), false);
    if (!checkModifiable(node->target.get(), "an assignment")) return setType(node, errorType(), false);

    switch (node->op) {
    case ast::AssignOp::Assign:
        convert(node->value, target, "an assignment");
        break;

    case ast::AssignOp::AddAssign:
    case ast::AssignOp::SubAssign:
        value = decayValue(node->value);
        if (isObjectPointer(target) && isIntegral(value)) {
            convert(node->value, intType(), "an assignment");
            break;
        }
        //fall through, numbers on both sides
    case ast::AssignOp::MulAssign:
    case ast::AssignOp::DivAssign:
    case ast::AssignOp::RemAssign: {
        value = decayValue(node->value);
        bool integers = node->op == ast::AssignOp::RemAssign;
        bool fits = integers ? isIntegral(target) && isIntegral(value) : isArithmetic(target) && isArithmetic(value);
        if (!fits) {
            diags.error(node->line, node->col, "invalid operands to a compound assignment ('" + typeToString(target)
                                                   + "' and '" + typeToString(value) + "')");
            return setType(node, errorType(), false);
        }
        convert(node->value, target, "an assignment");
        break;
    }
    }
    return setType(node, target, true);
}

const Type *TypeChecker::visitConditionalExpr(ast::ConditionalExpr *node) {
    checkCondition(node->cond, "a condition");
    const Type *t = check(node->trueExpr);
    const Type *f = check(node->falseExpr);
    if (isError(t) || isError(f)) return setType(node, errorType(), false);

    //both the same object: the result names it
    if (isSameType(t, f) && node->trueExpr->isLValue && node->falseExpr->isLValue && !asArray(t)) {
        return setType(node, t, true);
    }

    t = decayValue(node->trueExpr);
    f = decayValue(node->falseExpr);
    if (isArithmetic(t) && isArithmetic(f)) {
        const Type *common = commonArithmeticType(promote(t), promote(f));
        if (t->is(TypeKind::Bool) && f->is(TypeKind::Bool)) common = boolType();
        convert(node->trueExpr, common, "a ?: branch");
        convert(node->falseExpr, common, "a ?: branch");
        return setType(node, common, false);
    }
    if (isSameType(t, f)) return setType(node, t, false);
    if (findConversion(f, false, t, table).rank != ConversionRank::Impossible && !isArithmetic(t)) {
        convert(node->falseExpr, t, "a ?: branch");
        return setType(node, t, false);
    }
    if (findConversion(t, false, f, table).rank != ConversionRank::Impossible && !isArithmetic(f)) {
        convert(node->trueExpr, f, "a ?: branch");
        return setType(node, f, false);
    }
    diags.error(node->line, node->col, "the branches of ?: have incompatible types '" + typeToString(t)
                                           + "' and '" + typeToString(f) + "'");
    return setType(node, errorType(), false);
}

//a call by name or through a member is resolved against the overloads with the
//arguments. any other callee is a value: a function pointer or a lambda
const Type *TypeChecker::visitCallExpr(ast::CallExpr *node) {
    for (std::unique_ptr<ast::Expr> &arg : node->args) check(arg);

    ast::Expr *callee = withoutParens(node->callee.get());
    std::vector<FunctionSymbol *> candidates;
    Lookup lookup;
    std::string name;
    bool viaMember = false;

    if (callee->getKind() == ast::ASTKind::DeclRefExpr) {
        ast::DeclRefExpr *ref = static_cast<ast::DeclRefExpr *>(callee);
        lookup = lookupName(ref->name, ref->line, ref->col);
        name = ref->name;
        if (lookup.symbols.empty() || lookup.symbols[0]->kind != SymbolKind::Function || lookup.ambiguous) {
            finishDeclRef(ref, lookup);
        }
    } else if (callee->getKind() == ast::ASTKind::MemberExpr) {
        ast::MemberExpr *member = static_cast<ast::MemberExpr *>(callee);
        if (!findMember(member, lookup)) {
            setType(member, errorType(), false);
            return setType(node, errorType(), false);
        }
        name = member->member;
        viaMember = true;
        if (lookup.symbols[0]->kind != SymbolKind::Function) finishMember(member, lookup);
    }

    if (!lookup.symbols.empty() && lookup.symbols[0]->kind == SymbolKind::Function && !lookup.ambiguous) {
        for (Symbol *symbol : lookup.symbols) candidates.push_back(static_cast<FunctionSymbol *>(symbol));
        FunctionSymbol *fn = chooseOverload(candidates, node->args, name, node->line, node->col);
        if (fn == nullptr) {
            setType(callee, errorType(), false);
            return setType(node, errorType(), false);
        }
        //a method named without an object is called on this
        if (!viaMember && fn->isMethod() && !memberOfThis(fn, node->line, node->col)) {
            setType(callee, errorType(), false);
            return setType(node, errorType(), false);
        }
        checkAccess(fn, lookup, node->line, node->col);

        if (viaMember) {
            static_cast<ast::MemberExpr *>(callee)->symbol = fn;
        } else {
            static_cast<ast::DeclRefExpr *>(callee)->symbol = fn;
        }
        setType(callee, fn->signature(), true);
        if (node->callee.get() != callee) setType(node->callee.get(), fn->signature(), true);
        node->function = fn;
        applyArguments(fn->signature(), node->args, name, node->line, node->col);
        return callResult(node, fn->signature());
    }

    const Type *type = callee->type ? callee->type : check(node->callee);
    if (node->callee.get() != callee) setType(node->callee.get(), type, callee->isLValue);
    if (isError(type)) return setType(node, errorType(), false);

    const FunctionType *signature = nullptr;
    if (const PointerType *pointer = asPointer(type)) signature = asFunction(pointer->pointee);
    if (asFunction(type)) signature = asFunction(type);
    if (const ClosureType *closure = asClosure(type)) signature = closure->signature;
    if (signature == nullptr) {
        diags.error(node->line, node->col, "a value of type '" + typeToString(type) + "' cannot be called");
        return setType(node, errorType(), false);
    }
    for (const std::unique_ptr<ast::Expr> &arg : node->args) {
        if (isError(arg->type)) return setType(node, errorType(), false);
    }
    applyArguments(signature, node->args, name.empty() ? "the callee" : name, node->line, node->col);
    return callResult(node, signature);
}

const Type *TypeChecker::visitArraySubscriptExpr(ast::ArraySubscriptExpr *node) {
    const Type *base = check(node->base);
    const Type *index = check(node->index);
    if (isError(base) || isError(index)) return setType(node, errorType(), false);

    base = decayValue(node->base);
    if (!isObjectPointer(base)) {
        diags.error(node->line, node->col, "a value of type '" + typeToString(base) + "' cannot be indexed");
        return setType(node, errorType(), false);
    }
    index = decayValue(node->index);
    if (!isIntegral(index)) {
        diags.error(node->index->line, node->index->col, "an index must be an integer, not '"
                                                             + typeToString(index) + "'");
        return setType(node, errorType(), false);
    }
    convert(node->index, intType(), "an index");
    return setType(node, asPointer(base)->pointee, true);
}

const Type *TypeChecker::visitMemberExpr(ast::MemberExpr *node) {
    Lookup lookup;
    if (!findMember(node, lookup)) return setType(node, errorType(), false);
    return finishMember(node, lookup);
}

const Type *TypeChecker::visitNewExpr(ast::NewExpr *node) {
    const Type *type = node->type->resolved;
    if (isError(type)) return setType(node, errorType(), false);
    if (type->is(TypeKind::Void) || type->is(TypeKind::Auto) || asReference(type) || asFunction(type)) {
        diags.error(node->line, node->col, "cannot allocate a value of type '" + typeToString(type) + "'");
        return setType(node, errorType(), false);
    }
    RecordSymbol *record = asRecord(type);

    switch (node->form) {
    case ast::NewForm::Plain:
        if (record) node->constructor = construct(record, node->args, node->line, node->col);
        return setType(node, table.pointerTo(type), false);

    case ast::NewForm::Args:
        for (std::unique_ptr<ast::Expr> &arg : node->args) check(arg);
        if (record) {
            node->constructor = construct(record, node->args, node->line, node->col);
        } else if (node->args.size() == 1) {
            convert(node->args[0], type, "new");
        } else if (!node->args.empty()) {
            diags.error(node->line, node->col, "'" + typeToString(type) + "' takes one value to initialise");
        }
        return setType(node, table.pointerTo(type), false);

    case ast::NewForm::InitList: {
        ast::InitListExpr *list = static_cast<ast::InitListExpr *>(node->init.get());
        initFromList(list, type);
        node->constructor = list->constructor;
        return setType(node, table.pointerTo(type), false);
    }

    case ast::NewForm::Array:
    case ast::NewForm::ArrayInit: {
        //new T[n][3][4]: n may be any integer, the inner sizes are part of the
        //element type and must be constants. the result points at the first
        //element, an int (*)[3][4]
        const Type *count = check(node->dims[0]);
        if (!isError(count)) {
            count = decayValue(node->dims[0]);
            if (isIntegral(count)) {
                convert(node->dims[0], intType(), "an array size");
            } else {
                diags.error(node->dims[0]->line, node->dims[0]->col, "an array size must be an integer");
            }
        }
        const Type *element = type;
        for (size_t i = node->dims.size(); i-- > 1;) {
            long long size;
            if (!evaluateConstant(node->dims[i].get(), size) || size <= 0) {
                diags.error(node->dims[i]->line, node->dims[i]->col,
                            "only the first size in new may vary, the others must be positive constants");
                return setType(node, errorType(), false);
            }
            check(node->dims[i]);
            element = table.arrayOf(element, (uint64_t)size);
        }

        if (node->form == ast::NewForm::ArrayInit) {
            ast::InitListExpr *list = static_cast<ast::InitListExpr *>(node->init.get());
            long long size;
            if (!evaluateConstant(node->dims[0].get(), size)) size = (long long)list->inits.size();
            initFromList(list, table.arrayOf(element, (uint64_t)size));
        } else if (record) {
            node->constructor = construct(record, node->args, node->line, node->col);
        }
        return setType(node, table.pointerTo(element), false);
    }
    }
    return setType(node, errorType(), false);
}

const Type *TypeChecker::visitDeleteExpr(ast::DeleteExpr *node) {
    const Type *type = check(node->operand);
    if (isError(type)) return setType(node, voidType(), false);
    type = decayValue(node->operand);
    if (!isObjectPointer(type)) {
        diags.error(node->line, node->col, "cannot delete a value of type '" + typeToString(type) + "'");
    }
    return setType(node, voidType(), false);
}

//a brace list only has a type where something is initialised from it, which
//initFromList handles
const Type *TypeChecker::visitInitListExpr(ast::InitListExpr *node) {
    if (node->type) return node->type;
    diags.error(node->line, node->col, "a brace list can only initialise a variable");
    return setType(node, errorType(), false);
}

const Type *TypeChecker::visitLambdaExpr(ast::LambdaExpr *node) {
    ClosureType *closure = table.closureType(node);

    //the capture list is resolved where the lambda is written, outside its body
    for (ast::LambdaCapture &capture : node->captures) {
        if (capture.kind == ast::LambdaCapture::Kind::This) {
            if (hasCapture(node, ast::LambdaCapture::Kind::This, "")) {
                diags.error(node->line, node->col, "'this' is captured twice");
            } else if (thisRecord(node->line, node->col)) {
                node->usedCaptures.push_back(capture);
            }
            continue;
        }
        Lookup lookup = lookupName(capture.name, node->line, node->col);
        if (lookup.symbols.empty()) {
            diags.error(node->line, node->col, "use of undeclared identifier '" + capture.name + "' in a capture");
            continue;
        }
        if (!isLocal(lookup.symbols[0])) {
            diags.error(node->line, node->col, "'" + capture.name + "' cannot be captured, only local variables can");
            continue;
        }
        if (hasCapture(node, capture.kind, capture.name)) {
            diags.error(node->line, node->col, "'" + capture.name + "' is captured twice");
            continue;
        }
        capture.symbol = lookup.symbols[0];
        node->usedCaptures.push_back(capture);
    }

    FunctionContext saved = context;
    Scope *savedScope = current;
    context = FunctionContext();
    context.function = saved.function;
    context.lambda = node;
    current = node->scope;

    visit(node->body.get());
    resolveGotos(node->scope);

    std::vector<const Type *> params;
    for (const std::unique_ptr<ast::ParamDecl> &param : node->params) {
        params.push_back(param->symbol ? param->symbol->type : errorType());
    }
    const Type *returnType = context.returnType ? context.returnType : voidType();
    closure->signature = table.functionType(returnType, params, false);

    context = saved;
    current = savedScope;
    return setType(node, closure, false);
}

//casts are only made by this pass, after their operand has been checked
const Type *TypeChecker::visitImplicitCastExpr(ast::ImplicitCastExpr *node) { return node->type; }

} // namespace sema
