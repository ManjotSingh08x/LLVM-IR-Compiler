#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

#include "ast/ast.h"

namespace sema {

class RecordSymbol;
class SymbolTable;

enum class TypeKind {
    Void, Bool, Char, SChar, UChar, Short, UShort, Int, UInt, Long, ULong, Float, Double,
    Null,
    Auto,
    Error,
    Pointer, Reference, Array, Function, Record, Closure
};

class Type {
public:
    explicit Type(TypeKind kind) : kind(kind) {}
    virtual ~Type() = default;

    Type(const Type &) = delete;
    Type &operator=(const Type &) = delete;

    TypeKind getKind() const { return kind; }
    bool is(TypeKind other) const { return kind == other; }

    bool isConst() const { return unqualifiedType != nullptr; }

    const Type *unqualifiedType = nullptr;

private:
    const TypeKind kind;
};

class PointerType : public Type {
public:
    explicit PointerType(const Type *pointee) : Type(TypeKind::Pointer), pointee(pointee) {}

    const Type *pointee;
};

class ReferenceType : public Type {
public:
    explicit ReferenceType(const Type *referee) : Type(TypeKind::Reference), referee(referee) {}

    const Type *referee;
};

class ArrayType : public Type {
public:
    ArrayType(const Type *element, uint64_t size)
        : Type(TypeKind::Array), element(element), size(size) {}

    const Type *element;
    uint64_t    size;
};

class FunctionType : public Type {
public:
    FunctionType(const Type *returnType, std::vector<const Type *> params, bool isVariadic)
        : Type(TypeKind::Function), returnType(returnType), params(std::move(params)),
          isVariadic(isVariadic) {}

    const Type               *returnType;
    std::vector<const Type *> params;
    bool                      isVariadic;
};

class RecordType : public Type {
public:
    explicit RecordType(RecordSymbol *record) : Type(TypeKind::Record), record(record) {}

    RecordSymbol *record;
};

class ClosureType : public Type {
public:
    explicit ClosureType(ast::LambdaExpr *lambda) : Type(TypeKind::Closure), lambda(lambda) {}

    ast::LambdaExpr    *lambda;
    const FunctionType *signature = nullptr;
};

const Type *voidType();
const Type *boolType();
const Type *charType();
const Type *scharType();
const Type *ucharType();
const Type *shortType();
const Type *ushortType();
const Type *intType();
const Type *uintType();
const Type *longType();
const Type *ulongType();
const Type *floatType();
const Type *doubleType();
const Type *nullType();
const Type *autoType();
const Type *errorType();

const PointerType   *asPointer(const Type *type);
const ReferenceType *asReference(const Type *type);
const ArrayType     *asArray(const Type *type);
const FunctionType  *asFunction(const Type *type);
const ClosureType   *asClosure(const Type *type);
RecordSymbol        *asRecord(const Type *type);

bool isIntegral(const Type *type);
bool isUnsigned(const Type *type);
unsigned integerSize(const Type *type);
bool isFloating(const Type *type);
bool isArithmetic(const Type *type);
bool isScalar(const Type *type);
bool isObjectPointer(const Type *type);
bool isError(const Type *type);
bool isIncompleteRecord(const Type *type);

bool isSameType(const Type *a, const Type *b);

bool isSameUnqualified(const Type *a, const Type *b);

const Type *unqualified(const Type *type);

//checks whether a record inherits from another
bool isDerivedFrom(const RecordSymbol *derived, const RecordSymbol *base);

const Type *stripReference(const Type *type);

//arrays and functions become pointers
const Type *decay(const Type *type, SymbolTable &table);

//applies integer promotion rules
const Type *promote(const Type *type);

//finds the common type of two operands
const Type *commonArithmeticType(const Type *a, const Type *b);

enum class ConversionRank { Exact, Promotion, Conversion, Impossible };

struct ConversionStep {
    ast::CastKind kind;
    const Type   *result;
};

struct Conversion {
    ConversionRank              rank = ConversionRank::Impossible;
    std::vector<ConversionStep> steps;
    bool                        addsConst = false;
};

//ranks the implicit conversion between two types
Conversion findConversion(const Type *from, bool isLValue, const Type *to, SymbolTable &table);

std::string typeToString(const Type *type);

//builds the type's name fragment for symbols
std::string mangleType(const Type *type);

std::string toLLVMType(const Type *type);

std::string toLLVMRecord(const RecordSymbol *record);

}

#endif
