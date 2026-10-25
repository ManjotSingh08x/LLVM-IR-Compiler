#include "semantic/types.h"


#include "semantic/symbols.h"

namespace sema {

//builtins

namespace {

const Type theVoid(TypeKind::Void);
const Type theBool(TypeKind::Bool);
const Type theChar(TypeKind::Char);
const Type theInt(TypeKind::Int);
const Type theFloat(TypeKind::Float);
const Type theDouble(TypeKind::Double);
const Type theNull(TypeKind::Null);
const Type theAuto(TypeKind::Auto);
const Type theError(TypeKind::Error);

} // namespace

const Type *voidType() { return &theVoid; }
const Type *boolType() { return &theBool; }
const Type *charType() { return &theChar; }
const Type *intType() { return &theInt; }
const Type *floatType() { return &theFloat; }
const Type *doubleType() { return &theDouble; }
const Type *nullType() { return &theNull; }
const Type *autoType() { return &theAuto; }
const Type *errorType() { return &theError; }

//views

const PointerType *asPointer(const Type *type) {
    return type && type->is(TypeKind::Pointer) ? static_cast<const PointerType *>(type) : nullptr;
}

const ReferenceType *asReference(const Type *type) {
    return type && type->is(TypeKind::Reference) ? static_cast<const ReferenceType *>(type) : nullptr;
}

const ArrayType *asArray(const Type *type) {
    return type && type->is(TypeKind::Array) ? static_cast<const ArrayType *>(type) : nullptr;
}

const FunctionType *asFunction(const Type *type) {
    return type && type->is(TypeKind::Function) ? static_cast<const FunctionType *>(type) : nullptr;
}

const ClosureType *asClosure(const Type *type) {
    return type && type->is(TypeKind::Closure) ? static_cast<const ClosureType *>(type) : nullptr;
}

RecordSymbol *asRecord(const Type *type) {
    return type && type->is(TypeKind::Record) ? static_cast<const RecordType *>(type)->record : nullptr;
}

//classification

bool isIntegral(const Type *type) {
    return type->is(TypeKind::Bool) || type->is(TypeKind::Char) || type->is(TypeKind::Int);
}

bool isFloating(const Type *type) {
    return type->is(TypeKind::Float) || type->is(TypeKind::Double);
}

bool isArithmetic(const Type *type) { return isIntegral(type) || isFloating(type); }

bool isScalar(const Type *type) {
    return isArithmetic(type) || type->is(TypeKind::Pointer) || type->is(TypeKind::Null);
}

bool isObjectPointer(const Type *type) {
    const PointerType *pointer = asPointer(type);
    return pointer && !pointer->pointee->is(TypeKind::Void) && !pointer->pointee->is(TypeKind::Function);
}

bool isError(const Type *type) { return type == nullptr || type->is(TypeKind::Error); }

bool isSameType(const Type *a, const Type *b) {
    if (a == b) return true;
    if (a == nullptr || b == nullptr || a->getKind() != b->getKind()) return false;

    switch (a->getKind()) {
    case TypeKind::Pointer:
        return isSameType(asPointer(a)->pointee, asPointer(b)->pointee);
    case TypeKind::Reference:
        return isSameType(asReference(a)->referee, asReference(b)->referee);
    case TypeKind::Array:
        return asArray(a)->size == asArray(b)->size
            && isSameType(asArray(a)->element, asArray(b)->element);
    case TypeKind::Function: {
        const FunctionType *fa = asFunction(a);
        const FunctionType *fb = asFunction(b);
        if (fa->isVariadic != fb->isVariadic || fa->params.size() != fb->params.size()) return false;
        if (!isSameType(fa->returnType, fb->returnType)) return false;
        for (size_t i = 0; i < fa->params.size(); i++) {
            if (!isSameType(fa->params[i], fb->params[i])) return false;
        }
        return true;
    }
    case TypeKind::Record:
        return asRecord(a) == asRecord(b);
    case TypeKind::Closure:
        return asClosure(a)->lambda == asClosure(b)->lambda;
    default:
        //the builtins have no parts, the same kind is the same type
        return true;
    }
}

bool isDerivedFrom(const RecordSymbol *derived, const RecordSymbol *base) {
    for (const BaseClass &parent : derived->bases) {
        if (parent.record == base || isDerivedFrom(parent.record, base)) return true;
    }
    return false;
}

const Type *stripReference(const Type *type) {
    const ReferenceType *reference = asReference(type);
    return reference ? reference->referee : type;
}

const Type *decay(const Type *type, SymbolTable &table) {
    if (const ArrayType *array = asArray(type)) return table.pointerTo(array->element);
    if (type->is(TypeKind::Function)) return table.pointerTo(type);
    return type;
}

const Type *promote(const Type *type) {
    if (type->is(TypeKind::Bool) || type->is(TypeKind::Char)) return intType();
    return type;
}

const Type *commonArithmeticType(const Type *a, const Type *b) {
    if (a->is(TypeKind::Double) || b->is(TypeKind::Double)) return doubleType();
    if (a->is(TypeKind::Float) || b->is(TypeKind::Float)) return floatType();
    return intType();
}

//conversions

namespace {

ConversionRank worse(ConversionRank a, ConversionRank b) { return a > b ? a : b; }

//the cast between two arithmetic types
ast::CastKind arithmeticCast(const Type *from, const Type *to) {
    if (to->is(TypeKind::Bool)) return ast::CastKind::ToBool;
    if (isIntegral(from) && isIntegral(to)) return ast::CastKind::IntegralCast;
    if (isIntegral(from)) return ast::CastKind::IntegralToFloating;
    if (isIntegral(to)) return ast::CastKind::FloatingToIntegral;
    return ast::CastKind::FloatingCast;
}

bool isPromotion(const Type *from, const Type *to) {
    if (to->is(TypeKind::Int)) return from->is(TypeKind::Bool) || from->is(TypeKind::Char);
    if (to->is(TypeKind::Double)) return from->is(TypeKind::Float);
    return false;
}

} // namespace

Conversion findConversion(const Type *from, bool isLValue, const Type *to, SymbolTable &table) {
    Conversion result;

    //an error has already been reported, let it through without another one
    if (isError(from) || isError(to)) {
        result.rank = ConversionRank::Exact;
        return result;
    }

    //a reference binds to an object of its type, or of a type derived from it
    if (const ReferenceType *reference = asReference(to)) {
        if (!isLValue) return result;
        if (isSameType(from, reference->referee)) {
            result.rank = ConversionRank::Exact;
        } else if (asRecord(from) && asRecord(reference->referee)
                   && isDerivedFrom(asRecord(from), asRecord(reference->referee))) {
            result.rank = ConversionRank::Conversion;
            result.steps.push_back({ ast::CastKind::DerivedToBase, reference->referee });
        }
        return result;
    }

    if (isSameType(from, to)) {
        result.rank = ConversionRank::Exact;
        return result;
    }

    //array and function decay count as an exact match
    const Type *value = from;
    if (from->is(TypeKind::Array)) {
        value = decay(from, table);
        result.steps.push_back({ ast::CastKind::ArrayToPointer, value });
    } else if (from->is(TypeKind::Function)) {
        value = decay(from, table);
        result.steps.push_back({ ast::CastKind::FunctionToPointer, value });
    }
    result.rank = ConversionRank::Exact;
    if (isSameType(value, to)) return result;

    if (isArithmetic(value) && isArithmetic(to)) {
        result.rank = worse(result.rank, isPromotion(value, to) ? ConversionRank::Promotion
                                                                : ConversionRank::Conversion);
        result.steps.push_back({ arithmeticCast(value, to), to });
        return result;
    }

    if (to->is(TypeKind::Bool) && (value->is(TypeKind::Pointer) || value->is(TypeKind::Null))) {
        result.rank = ConversionRank::Conversion;
        result.steps.push_back({ ast::CastKind::ToBool, to });
        return result;
    }

    if (value->is(TypeKind::Null) && to->is(TypeKind::Pointer)) {
        result.rank = ConversionRank::Conversion;
        result.steps.push_back({ ast::CastKind::NullToPointer, to });
        return result;
    }

    const PointerType *fromPointer = asPointer(value);
    const PointerType *toPointer = asPointer(to);
    if (fromPointer && toPointer) {
        if (toPointer->pointee->is(TypeKind::Void) && !fromPointer->pointee->is(TypeKind::Function)) {
            result.rank = ConversionRank::Conversion;
            result.steps.push_back({ ast::CastKind::PointerToVoid, to });
            return result;
        }
        RecordSymbol *fromRecord = asRecord(fromPointer->pointee);
        RecordSymbol *toRecord = asRecord(toPointer->pointee);
        if (fromRecord && toRecord && isDerivedFrom(fromRecord, toRecord)) {
            result.rank = ConversionRank::Conversion;
            result.steps.push_back({ ast::CastKind::DerivedToBase, to });
            return result;
        }
    }

    //a derived object copied into a base object keeps only the base part
    if (asRecord(value) && asRecord(to) && isDerivedFrom(asRecord(value), asRecord(to))) {
        result.rank = ConversionRank::Conversion;
        result.steps.push_back({ ast::CastKind::DerivedToBase, to });
        return result;
    }

    result.rank = ConversionRank::Impossible;
    result.steps.clear();
    return result;
}

//spelling

namespace {

std::string spell(const Type *type, const std::string &inside);

bool isArrayOrFunction(const Type *type) {
    return type->is(TypeKind::Array) || type->is(TypeKind::Function);
}

std::string builtinName(const Type *type) {
    switch (type->getKind()) {
    case TypeKind::Void:   return "void";
    case TypeKind::Bool:   return "bool";
    case TypeKind::Char:   return "char";
    case TypeKind::Int:    return "int";
    case TypeKind::Float:  return "float";
    case TypeKind::Double: return "double";
    case TypeKind::Null:   return "null";
    case TypeKind::Auto:   return "auto";
    case TypeKind::Error:  return "<error>";
    default:               return "?";
    }
}

std::string paramList(const FunctionType *fn) {
    std::string out = "(";
    for (size_t i = 0; i < fn->params.size(); i++) {
        if (i > 0) out += ", ";
        out += typeToString(fn->params[i]);
    }
    if (fn->isVariadic) out += fn->params.empty() ? "..." : ", ...";
    return out + ")";
}

//built from the outside in, inside is what has been spelled around the name
std::string spell(const Type *type, const std::string &inside) {
    switch (type->getKind()) {
    case TypeKind::Pointer:
    case TypeKind::Reference: {
        const Type *inner = type->is(TypeKind::Pointer) ? asPointer(type)->pointee
                                                        : asReference(type)->referee;
        std::string mark = type->is(TypeKind::Pointer) ? "*" : "&";
        if (isArrayOrFunction(inner)) return spell(inner, "(" + mark + inside + ")");
        return spell(inner, mark + inside);
    }
    case TypeKind::Array: {
        const ArrayType *array = asArray(type);
        std::string size = array->size == 0 ? "" : std::to_string(array->size);
        return spell(array->element, inside + "[" + size + "]");
    }
    case TypeKind::Function: {
        const FunctionType *fn = asFunction(type);
        return spell(fn->returnType, inside + paramList(fn));
    }
    default: {
        std::string name;
        if (RecordSymbol *record = asRecord(type)) {
            name = record->qualifiedName();
        } else if (const ClosureType *closure = asClosure(type)) {
            name = "lambda <" + std::to_string(closure->lambda->line) + ":"
                 + std::to_string(closure->lambda->col) + ">";
        } else {
            name = builtinName(type);
        }
        if (inside.empty()) return name;
        if (inside[0] == '*' || inside[0] == '&') return name + inside;
        return name + " " + inside;
    }
    }
}

std::string lengthPrefixed(const std::string &name) { return std::to_string(name.size()) + name; }

std::string recordPath(const RecordSymbol *record) {
    return (record->outer ? recordPath(record->outer) : "") + lengthPrefixed(record->name);
}

} // namespace

std::string typeToString(const Type *type) {
    if (type == nullptr) return "<none>";
    return spell(type, "");
}

std::string mangleType(const Type *type) {
    switch (type->getKind()) {
    case TypeKind::Void:      return "v";
    case TypeKind::Bool:      return "b";
    case TypeKind::Char:      return "c";
    case TypeKind::Int:       return "i";
    case TypeKind::Float:     return "f";
    case TypeKind::Double:    return "d";
    case TypeKind::Pointer:   return "P" + mangleType(asPointer(type)->pointee);
    case TypeKind::Reference: return "R" + mangleType(asReference(type)->referee);
    case TypeKind::Array:
        return "A" + std::to_string(asArray(type)->size) + "_" + mangleType(asArray(type)->element);
    case TypeKind::Function: {
        const FunctionType *fn = asFunction(type);
        std::string out = "F" + mangleType(fn->returnType);
        if (fn->params.empty()) out += "v";
        for (const Type *param : fn->params) out += mangleType(param);
        return out + "E";
    }
    case TypeKind::Record: {
        //a nested record is wrapped in N..E so its parts are not read as parameters
        const RecordSymbol *record = asRecord(type);
        if (record->outer) return "N" + recordPath(record) + "E";
        return lengthPrefixed(record->name);
    }
    default:
        return "?";
    }
}

//lowering

std::string toLLVMRecord(const RecordSymbol *record) {
    if (!record) return "<none>";
    std::string name = "%" + std::string(record->isClass ? "class." : "struct.") + record->mangledName;
    std::string out = name + " { ";
    bool first = true;
    for (const BaseClass &base : record->bases) {
        if (!first) out += ", ";
        first = false;
        out += "%" + std::string(base.record->isClass ? "class." : "struct.") + base.record->mangledName;
    }
    for (const Symbol *field : record->fields) {
        if (!first) out += ", ";
        first = false;
        if (field->type && field->type->is(TypeKind::Record)) {
            const RecordSymbol *frec = asRecord(field->type);
            out += "%" + std::string(frec->isClass ? "class." : "struct.") + frec->mangledName;
        } else {
            out += toLLVMType(field->type);
        }
    }
    out += " }";
    return out;
}

std::string toLLVMType(const Type *type) {
    if (!type) return "<none>";
    switch (type->getKind()) {
    case TypeKind::Void:   return "void";
    case TypeKind::Bool:   return "i1";
    case TypeKind::Char:   return "i8";
    case TypeKind::Int:    return "i32";
    case TypeKind::Float:  return "float";
    case TypeKind::Double: return "double";

    case TypeKind::Null:
    case TypeKind::Pointer:
    case TypeKind::Reference:
        return "ptr";

    case TypeKind::Array: {
        const ArrayType *array = asArray(type);
        return "[" + std::to_string(array->size) + " x " + toLLVMType(array->element) + "]";
    }
    case TypeKind::Function: {
        const FunctionType *fn = asFunction(type);
        std::string out = toLLVMType(fn->returnType) + " (";
        for (size_t i = 0; i < fn->params.size(); i++) {
            if (i > 0) out += ", ";
            out += toLLVMType(fn->params[i]);
        }
        if (fn->isVariadic) {
            if (!fn->params.empty()) out += ", ";
            out += "...";
        }
        out += ")";
        return out;
    }
    //a record inside another type is named, its layout is written by toLLVMRecord
    case TypeKind::Record: {
        const RecordSymbol *rec = asRecord(type);
        return "%" + std::string(rec->isClass ? "class." : "struct.") + rec->mangledName;
    }
    case TypeKind::Closure: {
        const ClosureType *closure = asClosure(type);
        std::string elems;
        bool first = true;
        for (const ast::LambdaCapture &capture : closure->lambda->usedCaptures) {
            if (!first) elems += ", ";
            first = false;
            if (capture.kind == ast::LambdaCapture::Kind::ByCopy && capture.symbol) {
                elems += toLLVMType(stripReference(capture.symbol->type));
            } else {
                elems += "ptr";
            }
        }
        if (elems.empty()) {
            return "{}";
        } else {
            return "{ " + elems + " }";
        }
    }
    case TypeKind::Auto:
    case TypeKind::Error:
        return "<none>";
    }
    return "<none>";
}

} // namespace sema
