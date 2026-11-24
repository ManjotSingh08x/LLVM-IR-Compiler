#include "semantic/types.h"

#include "semantic/symbols.h"

namespace sema {

namespace {

const Type theVoid(TypeKind::Void);
const Type theBool(TypeKind::Bool);
const Type theChar(TypeKind::Char);
const Type theSChar(TypeKind::SChar);
const Type theUChar(TypeKind::UChar);
const Type theShort(TypeKind::Short);
const Type theUShort(TypeKind::UShort);
const Type theInt(TypeKind::Int);
const Type theUInt(TypeKind::UInt);
const Type theLong(TypeKind::Long);
const Type theULong(TypeKind::ULong);
const Type theFloat(TypeKind::Float);
const Type theDouble(TypeKind::Double);
const Type theNull(TypeKind::Null);
const Type theAuto(TypeKind::Auto);
const Type theError(TypeKind::Error);

}

const Type *voidType() { return &theVoid; }
const Type *boolType() { return &theBool; }
const Type *charType() { return &theChar; }
const Type *scharType() { return &theSChar; }
const Type *ucharType() { return &theUChar; }
const Type *shortType() { return &theShort; }
const Type *ushortType() { return &theUShort; }
const Type *intType() { return &theInt; }
const Type *uintType() { return &theUInt; }
const Type *longType() { return &theLong; }
const Type *ulongType() { return &theULong; }
const Type *floatType() { return &theFloat; }
const Type *doubleType() { return &theDouble; }
const Type *nullType() { return &theNull; }
const Type *autoType() { return &theAuto; }
const Type *errorType() { return &theError; }

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

bool isIntegral(const Type *type) { return integerSize(type) != 0; }

bool isUnsigned(const Type *type) {
    return type->is(TypeKind::UChar) || type->is(TypeKind::UShort) || type->is(TypeKind::UInt)
        || type->is(TypeKind::ULong);
}

unsigned integerSize(const Type *type) {
    switch (type->getKind()) {
    case TypeKind::Bool:
    case TypeKind::Char:
    case TypeKind::SChar:
    case TypeKind::UChar: return 1;
    case TypeKind::Short:
    case TypeKind::UShort: return 2;
    case TypeKind::Int:
    case TypeKind::UInt:  return 4;
    case TypeKind::Long:
    case TypeKind::ULong: return 8;
    default:              return 0;
    }
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

bool isIncompleteRecord(const Type *type) { return asRecord(type) && !asRecord(type)->isComplete; }

const Type *unqualified(const Type *type) {
    return type && type->isConst() ? type->unqualifiedType : type;
}

namespace {

bool sameType(const Type *a, const Type *b, bool withConst) {
    if (a == b) return true;
    if (a == nullptr || b == nullptr || a->getKind() != b->getKind()) return false;
    if (withConst && a->isConst() != b->isConst()) return false;
    a = unqualified(a);
    b = unqualified(b);
    if (a == b) return true;

    switch (a->getKind()) {
    case TypeKind::Pointer:
        return sameType(asPointer(a)->pointee, asPointer(b)->pointee, withConst);
    case TypeKind::Reference:
        return sameType(asReference(a)->referee, asReference(b)->referee, withConst);
    case TypeKind::Array:
        return asArray(a)->size == asArray(b)->size
            && sameType(asArray(a)->element, asArray(b)->element, withConst);
    case TypeKind::Function: {
        const FunctionType *fa = asFunction(a);
        const FunctionType *fb = asFunction(b);
        if (fa->isVariadic != fb->isVariadic || fa->params.size() != fb->params.size()) return false;
        if (!sameType(fa->returnType, fb->returnType, withConst)) return false;
        for (size_t i = 0; i < fa->params.size(); i++) {
            if (!sameType(fa->params[i], fb->params[i], withConst)) return false;
        }
        return true;
    }
    case TypeKind::Record:
        return asRecord(a) == asRecord(b);
    case TypeKind::Closure:
        return asClosure(a)->lambda == asClosure(b)->lambda;
    default:
        return true;
    }
}

}

bool isSameType(const Type *a, const Type *b) { return sameType(a, b, true); }

bool isSameUnqualified(const Type *a, const Type *b) { return sameType(a, b, false); }

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
    if (isIntegral(type) && integerSize(type) < 4) return intType();
    return unqualified(type);
}

const Type *commonArithmeticType(const Type *a, const Type *b) {
    a = unqualified(a);
    b = unqualified(b);
    if (a->is(TypeKind::Double) || b->is(TypeKind::Double)) return doubleType();
    if (a->is(TypeKind::Float) || b->is(TypeKind::Float)) return floatType();
    a = promote(a);
    b = promote(b);
    if (isSameType(a, b)) return a;
    if (isUnsigned(a) == isUnsigned(b)) return integerSize(a) >= integerSize(b) ? a : b;
    const Type *u = isUnsigned(a) ? a : b;
    const Type *s = isUnsigned(a) ? b : a;
    if (integerSize(u) >= integerSize(s)) return u;
    return s;
}

namespace {

ConversionRank worse(ConversionRank a, ConversionRank b) { return a > b ? a : b; }

ast::CastKind arithmeticCast(const Type *from, const Type *to) {
    if (to->is(TypeKind::Bool)) return ast::CastKind::ToBool;
    if (isIntegral(from) && isIntegral(to)) return ast::CastKind::IntegralCast;
    if (isIntegral(from)) return ast::CastKind::IntegralToFloating;
    if (isIntegral(to)) return ast::CastKind::FloatingToIntegral;
    return ast::CastKind::FloatingCast;
}

bool isPromotion(const Type *from, const Type *to) {
    if (to->is(TypeKind::Int)) return isIntegral(from) && integerSize(from) < 4;
    if (to->is(TypeKind::Double)) return from->is(TypeKind::Float);
    return false;
}

}

namespace {

bool bindsDirectly(const Type *from, const Type *referee, Conversion &result) {
    if (from->isConst() && !referee->isConst()) return false;
    const Type *object = unqualified(from);
    const Type *target = unqualified(referee);
    result.addsConst = referee->isConst() && !from->isConst();
    if (isSameType(object, target)) {
        result.rank = ConversionRank::Exact;
        return true;
    }
    if (asRecord(object) && asRecord(target) && isDerivedFrom(asRecord(object), asRecord(target))) {
        result.rank = ConversionRank::Conversion;
        result.steps.push_back({ ast::CastKind::DerivedToBase, referee });
        return true;
    }
    return false;
}

bool qualifiesTo(const PointerType *from, const PointerType *to, bool &addsConst) {
    const Type *f = from->pointee;
    const Type *t = to->pointee;
    bool constAbove = true;
    while (true) {
        if (f->isConst() && !t->isConst()) return false;
        if (t->isConst() && !f->isConst()) {
            if (!constAbove) return false;
            addsConst = true;
        }
        constAbove = constAbove && t->isConst();
        const PointerType *fp = asPointer(unqualified(f));
        const PointerType *tp = asPointer(unqualified(t));
        if (fp == nullptr || tp == nullptr) return isSameType(unqualified(f), unqualified(t));
        f = fp->pointee;
        t = tp->pointee;
    }
}

}

Conversion findConversion(const Type *from, bool isLValue, const Type *to, SymbolTable &table) {
    Conversion result;

    if (isError(from) || isError(to)) {
        result.rank = ConversionRank::Exact;
        return result;
    }

    if (const ReferenceType *reference = asReference(to)) {
        if (isLValue && bindsDirectly(from, reference->referee, result)) return result;
        result = Conversion();
        if (!reference->referee->isConst()) return result;
        result = findConversion(from, isLValue, unqualified(reference->referee), table);
        result.addsConst = true;
        return result;
    }

    from = unqualified(from);
    to = unqualified(to);

    if (isSameType(from, to)) {
        result.rank = ConversionRank::Exact;
        return result;
    }

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
        if (qualifiesTo(fromPointer, toPointer, result.addsConst)) return result;
        result.addsConst = false;
        bool keepsConst = !fromPointer->pointee->isConst() || toPointer->pointee->isConst();
        if (keepsConst && toPointer->pointee->is(TypeKind::Void) && !fromPointer->pointee->is(TypeKind::Function)) {
            result.rank = ConversionRank::Conversion;
            result.steps.push_back({ ast::CastKind::PointerToVoid, to });
            return result;
        }
        RecordSymbol *fromRecord = asRecord(fromPointer->pointee);
        RecordSymbol *toRecord = asRecord(toPointer->pointee);
        if (keepsConst && fromRecord && toRecord && isDerivedFrom(fromRecord, toRecord)) {
            result.rank = ConversionRank::Conversion;
            result.steps.push_back({ ast::CastKind::DerivedToBase, to });
            return result;
        }
    }

    if (asRecord(value) && asRecord(to) && isDerivedFrom(asRecord(value), asRecord(to))) {
        result.rank = ConversionRank::Conversion;
        result.steps.push_back({ ast::CastKind::DerivedToBase, to });
        return result;
    }

    result.rank = ConversionRank::Impossible;
    result.steps.clear();
    return result;
}

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
    case TypeKind::SChar:  return "signed char";
    case TypeKind::UChar:  return "unsigned char";
    case TypeKind::Short:  return "short";
    case TypeKind::UShort: return "unsigned short";
    case TypeKind::Int:    return "int";
    case TypeKind::UInt:   return "unsigned int";
    case TypeKind::Long:   return "long";
    case TypeKind::ULong:  return "unsigned long";
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

std::string spell(const Type *type, const std::string &inside) {
    switch (type->getKind()) {
    case TypeKind::Pointer:
    case TypeKind::Reference: {
        const Type *inner = type->is(TypeKind::Pointer) ? asPointer(type)->pointee
                                                        : asReference(type)->referee;
        std::string mark = type->is(TypeKind::Pointer) ? "*" : "&";
        if (type->isConst()) mark += inside.empty() ? "const" : "const ";
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
        if (type->isConst()) name = "const " + name;
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

}

std::string typeToString(const Type *type) {
    if (type == nullptr) return "<none>";
    return spell(type, "");
}

std::string mangleType(const Type *type) {
    if (type->isConst()) return "K" + mangleType(unqualified(type));
    switch (type->getKind()) {
    case TypeKind::Void:      return "v";
    case TypeKind::Bool:      return "b";
    case TypeKind::Char:      return "c";
    case TypeKind::SChar:     return "a";
    case TypeKind::UChar:     return "h";
    case TypeKind::Short:     return "s";
    case TypeKind::UShort:    return "t";
    case TypeKind::Int:       return "i";
    case TypeKind::UInt:      return "j";
    case TypeKind::Long:      return "l";
    case TypeKind::ULong:     return "m";
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
        const RecordSymbol *record = asRecord(type);
        const RecordSymbol *outermost = record;
        while (outermost->outer) outermost = outermost->outer;
        std::string spaces = namespacePath(outermost->scope);
        if (record->outer || !spaces.empty()) return "N" + spaces + recordPath(record) + "E";
        return lengthPrefixed(record->name);
    }
    default:
        return "?";
    }
}

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
    case TypeKind::Char:
    case TypeKind::SChar:
    case TypeKind::UChar:  return "i8";
    case TypeKind::Short:
    case TypeKind::UShort: return "i16";
    case TypeKind::Int:
    case TypeKind::UInt:   return "i32";
    case TypeKind::Long:
    case TypeKind::ULong:  return "i64";
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

}
