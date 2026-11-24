#include "semantic/symbols.h"

namespace sema {

Scope *Scope::addChild(ScopeKind childKind, int childLine, int childCol) {
    children.emplace_back(new Scope(childKind, this, childLine, childCol));
    Scope *child = children.back().get();
    child->record = record;
    child->function = function;
    child->lambda = lambda;
    if (childKind == ScopeKind::Global || childKind == ScopeKind::Namespace || childKind == ScopeKind::Record) {
        child->function = nullptr;
        child->lambda = nullptr;
    }
    return child;
}

Symbol *Scope::add(std::unique_ptr<Symbol> symbol, bool findable) {
    Symbol *raw = symbol.get();
    raw->scope = this;
    if (findable) table[raw->name].push_back(raw);
    symbols.push_back(std::move(symbol));
    return raw;
}

const std::vector<Symbol *> *Scope::find(const std::string &name) const {
    auto it = table.find(name);
    if (it == table.end()) return nullptr;
    return &it->second;
}

Scope *Scope::functionScope() {
    for (Scope *scope = this; scope != nullptr; scope = scope->parent) {
        if (scope->kind == ScopeKind::Function || scope->kind == ScopeKind::Lambda) return scope;
    }
    return nullptr;
}

std::string RecordSymbol::qualifiedName() const {
    return outer ? outer->qualifiedName() + "::" + name : namespacePrefix(scope) + name;
}

std::string NamespaceSymbol::qualifiedName() const { return namespacePrefix(scope) + name; }

std::string namespacePath(const Scope *scope) {
    if (scope == nullptr || scope->kind != ScopeKind::Namespace) return "";
    return namespacePath(scope->parent) + std::to_string(scope->space->name.size()) + scope->space->name;
}

std::string namespacePrefix(const Scope *scope) {
    if (scope == nullptr || scope->kind != ScopeKind::Namespace) return "";
    return namespacePrefix(scope->parent) + scope->space->name + "::";
}

std::string QualifierTarget::spelling() const {
    if (record) return std::string(record->isClass ? "class '" : "struct '") + record->qualifiedName() + "'";
    if (scope && scope->space) return "namespace '" + scope->space->qualifiedName() + "'";
    return "the global namespace";
}

namespace {

//a namespace or class that can hold the next part of a qualified name
Symbol *container(const std::vector<Symbol *> *found) {
    if (found == nullptr) return nullptr;
    Symbol *symbol = (*found)[0];
    return symbol->kind == SymbolKind::Namespace || symbol->kind == SymbolKind::Record ? symbol : nullptr;
}

} // namespace

QualifierTarget resolveQualifier(Scope *from, const std::vector<std::string> &qualifier, int line, int col) {
    QualifierTarget target;
    Symbol *current = nullptr;
    for (Scope *scope = from; scope != nullptr && current == nullptr; scope = scope->parent) {
        Symbol *symbol = container(scope->find(qualifier[0]));
        if (symbol == nullptr) continue;
        bool before = symbol->line < line || (symbol->line == line && symbol->col < col);
        if (scope->kind == ScopeKind::Record || symbol->line == 0 || before) current = symbol;
    }
    if (current == nullptr) {
        target.error = "'" + qualifier[0] + "' is not a namespace or class";
        return target;
    }
    for (size_t i = 1; i <= qualifier.size(); i++) {
        if (current->kind == SymbolKind::Namespace) {
            target.scope = static_cast<NamespaceSymbol *>(current)->members;
            target.record = nullptr;
        } else {
            RecordSymbol *record = static_cast<RecordSymbol *>(current);
            if (record->members == nullptr) {
                target.error = "'" + record->qualifiedName() + "' is an incomplete type";
                return target;
            }
            target.scope = record->members;
            target.record = record;
        }
        if (i == qualifier.size()) break;
        Symbol *next = container(target.scope->find(qualifier[i]));
        if (next == nullptr) {
            target.error = "'" + qualifier[i] + "' is not a namespace or class in " + target.spelling();
            target.scope = nullptr;
            target.record = nullptr;
            return target;
        }
        current = next;
    }
    return target;
}

SymbolTable::SymbolTable() : root(new Scope(ScopeKind::Global, nullptr, 1, 1)) {}

const Type *SymbolTable::keep(Type *type) {
    types.emplace_back(type);
    return type;
}

const Type *SymbolTable::pointerTo(const Type *pointee) { return keep(new PointerType(pointee)); }

const Type *SymbolTable::referenceTo(const Type *referee) { return keep(new ReferenceType(referee)); }

const Type *SymbolTable::arrayOf(const Type *element, uint64_t size) {
    return keep(new ArrayType(element, size));
}

const FunctionType *SymbolTable::functionType(const Type *returnType, std::vector<const Type *> params,
                                              bool isVariadic) {
    FunctionType *fn = new FunctionType(returnType, std::move(params), isVariadic);
    keep(fn);
    return fn;
}

const Type *SymbolTable::recordType(RecordSymbol *record) { return keep(new RecordType(record)); }

const Type *SymbolTable::constOf(const Type *type) {
    if (type->isConst()) return type;
    Type *qualified = nullptr;
    switch (type->getKind()) {
    case TypeKind::Array:
        return arrayOf(constOf(asArray(type)->element), asArray(type)->size);
    case TypeKind::Reference:
    case TypeKind::Function:
    case TypeKind::Closure:
    case TypeKind::Null:
    case TypeKind::Error:
        return type;
    case TypeKind::Pointer:
        qualified = new PointerType(asPointer(type)->pointee);
        break;
    case TypeKind::Record:
        qualified = new RecordType(asRecord(type));
        break;
    default:
        qualified = new Type(type->getKind());
        break;
    }
    qualified->unqualifiedType = type;
    return keep(qualified);
}

ClosureType *SymbolTable::closureType(ast::LambdaExpr *lambda) {
    ClosureType *closure = new ClosureType(lambda);
    keep(closure);
    return closure;
}

namespace {

const char *scopeKindName(ScopeKind kind) {
    switch (kind) {
    case ScopeKind::Global:   return "global";
    case ScopeKind::Namespace: return "namespace";
    case ScopeKind::Record:   return "record";
    case ScopeKind::Function: return "function";
    case ScopeKind::Block:    return "block";
    case ScopeKind::Lambda:   return "lambda";
    }
    return "?";
}

const char *accessName(ast::AccessKind access) {
    switch (access) {
    case ast::AccessKind::Public:    return " public";
    case ast::AccessKind::Private:   return " private";
    case ast::AccessKind::Protected: return " protected";
    case ast::AccessKind::None:      break;
    }
    return "";
}

const char *symbolKindName(const Symbol &symbol) {
    switch (symbol.kind) {
    case SymbolKind::Variable:  return symbol.parentRecord ? "static member" : "variable";
    case SymbolKind::Parameter: return "parameter";
    case SymbolKind::Field:     return "field";
    case SymbolKind::Record:    return static_cast<const RecordSymbol &>(symbol).isClass ? "class" : "struct";
    case SymbolKind::Typedef:   return "typedef";
    case SymbolKind::Namespace: return "namespace";
    case SymbolKind::Function: {
        const FunctionSymbol &fn = static_cast<const FunctionSymbol &>(symbol);
        if (fn.isConstructor) return "constructor";
        if (fn.isDestructor) return "destructor";
        if (fn.parentRecord) return fn.isStatic ? "static method" : "method";
        return "function";
    }
    }
    return "?";
}

void dumpScope(std::ostream &out, Scope *scope, int depth) {
    std::string pad(depth * 2, ' ');
    out << pad << "scope " << scopeKindName(scope->kind);
    if (scope->kind == ScopeKind::Record && scope->record) out << " " << scope->record->qualifiedName();
    if (scope->kind == ScopeKind::Function && scope->function) out << " " << scope->function->name;
    if (scope->kind == ScopeKind::Namespace && scope->space) out << " " << scope->space->qualifiedName();
    out << " <" << scope->line << ":" << scope->col << ">\n";

    for (const std::unique_ptr<Symbol> &symbol : scope->symbols) {
        out << pad << "  " << symbolKindName(*symbol) << " " << symbol->name;
        if (symbol->kind != SymbolKind::Record && symbol->kind != SymbolKind::Namespace) {
            out << " : " << typeToString(symbol->type);
        }
        out << accessName(symbol->access);
        if (symbol->isStatic) out << " static";
        if (!symbol->mangledName.empty()) out << " [" << symbol->mangledName << "]";

        if (symbol->kind == SymbolKind::Function) {
            const FunctionSymbol &fn = static_cast<const FunctionSymbol &>(*symbol);
            if (fn.isMethod()) out << " this-lowered: " << typeToString(fn.loweredType);
            if (!fn.isBuiltin && !fn.definition) out << " (no definition)";
            if (fn.loweredType) out << " llvm: " << toLLVMType(fn.loweredType);
        } else if (symbol->kind == SymbolKind::Record) {
            out << " llvm: " << toLLVMRecord(static_cast<const RecordSymbol *>(symbol.get()));
        } else if (symbol->kind != SymbolKind::Namespace && symbol->type && !symbol->type->is(TypeKind::Auto)) {
            out << " llvm: " << toLLVMType(symbol->type);
        }
        out << "\n";
    }
    for (const std::unique_ptr<Scope> &child : scope->children) dumpScope(out, child.get(), depth + 1);
}

} // namespace

void SymbolTable::dump(std::ostream &out) {
    dumpScope(out, root.get(), 0);
}

} // namespace sema
