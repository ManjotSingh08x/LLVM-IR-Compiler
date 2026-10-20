#include "semantic/symbols.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

namespace sema {

//scope

Scope *Scope::addChild(ScopeKind childKind, int childLine, int childCol) {
    children.emplace_back(new Scope(childKind, this, childLine, childCol));
    Scope *child = children.back().get();
    child->record = record;
    child->function = function;
    child->lambda = lambda;
    if (childKind == ScopeKind::Global || childKind == ScopeKind::Record) {
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

//symbol table

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

ClosureType *SymbolTable::closureType(ast::LambdaExpr *lambda) {
    ClosureType *closure = new ClosureType(lambda);
    keep(closure);
    return closure;
}

//dump

namespace {

const char *scopeKindName(ScopeKind kind) {
    switch (kind) {
    case ScopeKind::Global:   return "global";
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

std::string llvmSpelling(llvm::Type *type) {
    if (type == nullptr) return "<none>";
    std::string out;
    llvm::raw_string_ostream stream(out);
    type->print(stream);
    return stream.str();
}

//a named struct prints as its whole definition, nested bases and all. the name
//with one level of fields is what is worth reading
std::string llvmRecordBody(llvm::Type *type) {
    llvm::StructType *record = llvm::dyn_cast<llvm::StructType>(type);
    if (record == nullptr) return llvmSpelling(type);
    std::string out = "%" + record->getName().str() + " { ";
    for (unsigned i = 0; i < record->getNumElements(); i++) {
        if (i > 0) out += ", ";
        llvm::Type *field = record->getElementType(i);
        llvm::StructType *nested = llvm::dyn_cast<llvm::StructType>(field);
        out += nested ? "%" + nested->getName().str() : llvmSpelling(field);
    }
    return out + " }";
}

const char *symbolKindName(const Symbol &symbol) {
    switch (symbol.kind) {
    case SymbolKind::Variable:  return symbol.parentRecord ? "static member" : "variable";
    case SymbolKind::Parameter: return "parameter";
    case SymbolKind::Field:     return "field";
    case SymbolKind::Record:    return static_cast<const RecordSymbol &>(symbol).isClass ? "class" : "struct";
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

void dumpScope(std::ostream &out, Scope *scope, int depth, llvm::LLVMContext &context) {
    std::string pad(depth * 2, ' ');
    out << pad << "scope " << scopeKindName(scope->kind);
    if (scope->kind == ScopeKind::Record && scope->record) out << " " << scope->record->qualifiedName();
    if (scope->kind == ScopeKind::Function && scope->function) out << " " << scope->function->name;
    out << " <" << scope->line << ":" << scope->col << ">\n";

    for (const std::unique_ptr<Symbol> &symbol : scope->symbols) {
        out << pad << "  " << symbolKindName(*symbol) << " " << symbol->name;
        if (symbol->kind != SymbolKind::Record) out << " : " << typeToString(symbol->type);
        out << accessName(symbol->access);
        if (symbol->isStatic) out << " static";
        if (!symbol->mangledName.empty()) out << " [" << symbol->mangledName << "]";

        if (symbol->kind == SymbolKind::Function) {
            const FunctionSymbol &fn = static_cast<const FunctionSymbol &>(*symbol);
            if (fn.isMethod()) out << " this-lowered: " << typeToString(fn.loweredType);
            if (!fn.isBuiltin && !fn.definition) out << " (no definition)";
            if (fn.loweredType) out << " llvm: " << llvmSpelling(toLLVM(fn.loweredType, context));
        } else if (symbol->kind == SymbolKind::Record) {
            out << " llvm: " << llvmRecordBody(toLLVM(symbol->type, context));
        } else if (symbol->type && !symbol->type->is(TypeKind::Auto)) {
            out << " llvm: " << llvmSpelling(toLLVM(symbol->type, context));
        }
        out << "\n";
    }
    for (const std::unique_ptr<Scope> &child : scope->children) dumpScope(out, child.get(), depth + 1, context);
}

} // namespace

void SymbolTable::dump(std::ostream &out) {
    //a context of its own, only to show what each type lowers to
    llvm::LLVMContext context;
    dumpScope(out, root.get(), 0, context);
}

} // namespace sema
