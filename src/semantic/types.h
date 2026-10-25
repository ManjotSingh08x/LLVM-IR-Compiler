#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

#include "ast/ast.h"


namespace sema {

class RecordSymbol;
class SymbolTable;

//semantic type, compare with isSameType, never by address
enum class TypeKind {
    Void, Bool, Char, Int, Float, Double,
    Null,     //the type of the null literal, converts to any pointer
    Auto,     //an auto variable before its initializer is checked
    Error,    //the type of an expression that already produced an error
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

//a size of 0 is an unsized [] until its size is known
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
    bool                      isVariadic;  //printf and scanf
};

//a struct or class, identified by its declaration
class RecordType : public Type {
public:
    explicit RecordType(RecordSymbol *record) : Type(TypeKind::Record), record(record) {}

    RecordSymbol *record;
};

//a lambda's own type, its signature known once the body is checked
class ClosureType : public Type {
public:
    explicit ClosureType(ast::LambdaExpr *lambda) : Type(TypeKind::Closure), lambda(lambda) {}

    ast::LambdaExpr    *lambda;
    const FunctionType *signature = nullptr;
};

//builtin types
const Type *voidType();
const Type *boolType();
const Type *charType();
const Type *intType();
const Type *floatType();
const Type *doubleType();
const Type *nullType();
const Type *autoType();
const Type *errorType();

//views of a type as one of the compound kinds, null when it is not that kind
const PointerType   *asPointer(const Type *type);
const ReferenceType *asReference(const Type *type);
const ArrayType     *asArray(const Type *type);
const FunctionType  *asFunction(const Type *type);
const ClosureType   *asClosure(const Type *type);
RecordSymbol        *asRecord(const Type *type);

//classification
bool isIntegral(const Type *type);    //bool, char, int
bool isFloating(const Type *type);    //float, double
bool isArithmetic(const Type *type);
bool isScalar(const Type *type);      //arithmetic, pointers and null: usable as a condition
bool isObjectPointer(const Type *type);  //a pointer that can be dereferenced to an object
bool isError(const Type *type);

//structural equality: same kind and same parts, records by declaration
bool isSameType(const Type *a, const Type *b);

//true when base is a base of derived at any depth
bool isDerivedFrom(const RecordSymbol *derived, const RecordSymbol *base);

//the referred type for a reference, the type itself otherwise
const Type *stripReference(const Type *type);

//array-to-pointer and function-to-pointer decay
const Type *decay(const Type *type, SymbolTable &table);

//bool and char become int, the integer promotion
const Type *promote(const Type *type);

//the common arithmetic type: double, then float, then int
const Type *commonArithmeticType(const Type *a, const Type *b);

//how well a value converts to a type, best first
enum class ConversionRank { Exact, Promotion, Conversion, Impossible };

//the implicit casts from a value to a target type, in order
struct ConversionStep {
    ast::CastKind kind;
    const Type   *result;
};

struct Conversion {
    ConversionRank              rank = ConversionRank::Impossible;
    std::vector<ConversionStep> steps;
};

//the implicit conversion from type from to type to
Conversion findConversion(const Type *from, bool isLValue, const Type *to, SymbolTable &table);

//C style spelling, for messages and dumps: int **(*)[10]
std::string typeToString(const Type *type);

//the type code used in mangled names, see the README
std::string mangleType(const Type *type);

//the llvm type a value is stored as, pointers are opaque ptr
std::string toLLVMType(const Type *type);

//the llvm record layout spelling: bases in order, then fields
std::string toLLVMRecord(const RecordSymbol *record);

} // namespace sema

#endif
