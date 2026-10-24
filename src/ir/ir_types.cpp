#include "ir/ir_types.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace ir {

const sema::Type *int64Type() {
    static const sema::Type int64(sema::TypeKind::Int);
    return &int64;
}

bool isInt64(const sema::Type *type) { return type == int64Type(); }

std::string formatType(const sema::Type *type) {
    if (isInt64(type)) return "i64";

    switch (type->getKind()) {
    case sema::TypeKind::Void:   return "void";
    case sema::TypeKind::Bool:   return "i1";
    case sema::TypeKind::Char:   return "i8";
    case sema::TypeKind::Int:    return "i32";
    case sema::TypeKind::Float:  return "float";
    case sema::TypeKind::Double: return "double";

    //every pointer is the opaque ptr, the pointee is kept in the sema type
    case sema::TypeKind::Null:
    case sema::TypeKind::Pointer:
    case sema::TypeKind::Reference:
        return "ptr";

    case sema::TypeKind::Array: {
        const sema::ArrayType *array = sema::asArray(type);
        return "[" + std::to_string(array->size) + " x " + formatType(array->element) + "]";
    }
    case sema::TypeKind::Function: {
        const sema::FunctionType *fn = sema::asFunction(type);
        std::string out = formatType(fn->returnType) + " (";
        for (size_t i = 0; i < fn->params.size(); i++) {
            if (i > 0) out += ", ";
            out += formatType(fn->params[i]);
        }
        if (fn->isVariadic) {
            if (!fn->params.empty()) out += ", ";
            out += "...";
        }
        return out + ")";
    }
    case sema::TypeKind::Record:
        return formatRecordName(sema::asRecord(type));
    case sema::TypeKind::Closure:
        return closureTypeName(sema::asClosure(type)->lambda);

    //the semantic phase reports these, the ir is only built without errors
    case sema::TypeKind::Auto:
    case sema::TypeKind::Error:
        break;
    }
    return "<invalid type>";
}

std::string formatRecordName(const sema::RecordSymbol *record) {
    return "%" + std::string(record->isClass ? "class." : "struct.") + record->mangledName;
}

std::string formatRecordBody(const sema::RecordSymbol *record) {
    std::string elements;
    for (const sema::BaseClass &base : record->bases) {
        if (!elements.empty()) elements += ", ";
        elements += formatRecordName(base.record);
    }
    for (const sema::Symbol *field : record->fields) {
        if (!elements.empty()) elements += ", ";
        elements += formatType(field->type);
    }
    if (elements.empty()) return "{}";
    return "{ " + elements + " }";
}

std::string closureTypeName(const ast::LambdaExpr *lambda) {
    return "%closure." + std::to_string(lambda->line) + "." + std::to_string(lambda->col);
}

//must match sema::toLLVMType for a closure: a copied variable is stored as its
//value (a copied reference as the value it refers to), everything else as ptr
std::string formatClosureBody(const ast::LambdaExpr *lambda) {
    std::string elements;
    for (const ast::LambdaCapture &capture : lambda->usedCaptures) {
        if (!elements.empty()) elements += ", ";
        if (capture.kind == ast::LambdaCapture::Kind::ByCopy) {
            elements += formatType(sema::stripReference(capture.symbol->type));
        } else {
            elements += "ptr";
        }
    }
    if (elements.empty()) return "{}";
    return "{ " + elements + " }";
}

std::string lambdaFunctionName(const ast::LambdaExpr *lambda) {
    return "__lambda_" + std::to_string(lambda->line) + "_" + std::to_string(lambda->col);
}

std::string formatFloatLiteral(double value, bool isFloat32) {
    double exact = value;
    if (isFloat32) exact = (double)(float)value;

    uint64_t bits = 0;
    std::memcpy(&bits, &exact, sizeof bits);
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "0x%016llX", (unsigned long long)bits);
    return buffer;
}

std::string escapeLLVMString(const std::string &bytes) {
    std::string out = "c\"";
    for (size_t i = 0; i < bytes.size(); i++) {
        unsigned char c = (unsigned char)bytes[i];
        if (c >= 0x20 && c <= 0x7E && c != '"' && c != '\\') {
            out += (char)c;
        } else {
            char buffer[8];
            std::snprintf(buffer, sizeof buffer, "\\%02X", c);
            out += buffer;
        }
    }
    return out + "\"";
}

unsigned integerBits(const sema::Type *type) {
    if (isInt64(type)) return 64;
    switch (type->getKind()) {
    case sema::TypeKind::Bool: return 1;
    case sema::TypeKind::Char: return 8;
    case sema::TypeKind::Int:  return 32;
    default:                   return 0;
    }
}

long long normalizeInt(long long value, const sema::Type *type) {
    switch (integerBits(type)) {
    case 1:  return value & 1;
    case 8:  return (long long)(int8_t)(uint8_t)(value & 0xFF);
    case 32: return (long long)(int32_t)(uint32_t)(value & 0xFFFFFFFFLL);
    default: return value;
    }
}

} // namespace ir
