#ifndef IR_VALUE_H
#define IR_VALUE_H

#include <ostream>
#include <string>
#include <vector>

#include "ir/ir_types.h"

namespace ir {

class Function;

enum class ValueKind {
    Constant,
    Argument,
    GlobalVariable,
    Function,
    BasicBlock,
    Instruction
};

//base of all operands
class Value {
public:
    Value(ValueKind kind, const sema::Type *type, const std::string &name)
        : kind(kind), type(type), name(name) {}
    virtual ~Value() = default;

    Value(const Value &) = delete;
    Value &operator=(const Value &) = delete;

    ValueKind          getKind() const { return kind; }
    const sema::Type  *getType() const { return type; }
    const std::string &getName() const { return name; }
    void               setName(const std::string &newName) { name = newName; }

    virtual std::string getOperand() const = 0;

    std::string getTypedOperand() const { return formatType(type) + " " + getOperand(); }

protected:
    ValueKind         kind;
    const sema::Type *type;
    std::string       name;
};

enum class ConstantKind {
    Int,
    Float,
    Null,
    ZeroInit,
    String,
    Aggregate,
    Address
};

//compile-time value owned by module
class Constant : public Value {
public:
    Constant(ConstantKind constKind, const sema::Type *type)
        : Value(ValueKind::Constant, type, ""), constKind(constKind) {}

    std::string getOperand() const override;

    ConstantKind                  constKind;
    long long                     intValue = 0;
    double                        floatValue = 0.0;
    std::string                   bytes;
    std::vector<const Constant *> elements;
    const Value                  *target = nullptr;
};

class Argument : public Value {
public:
    Argument(const sema::Type *type, const std::string &name, Function *parent, size_t index)
        : Value(ValueKind::Argument, type, name), parent(parent), index(index) {}

    std::string getOperand() const override { return "%" + name; }

    Function *parent;
    size_t    index;
};

enum class Linkage {
    External,
    Internal,
    Private
};

//global variable, operand is its address
class GlobalVariable : public Value {
public:
    GlobalVariable(const std::string &name, const sema::Type *valueType, const sema::Type *pointerType,
                   const Constant *initializer, Linkage linkage, bool isConstant)
        : Value(ValueKind::GlobalVariable, pointerType, name), valueType(valueType),
          initializer(initializer), linkage(linkage), isConstant(isConstant) {}

    std::string getOperand() const override { return "@" + name; }

    void dump(std::ostream &out) const;

    const sema::Type *valueType;
    const Constant   *initializer;
    Linkage           linkage;
    bool              isConstant;
};

}

#endif
