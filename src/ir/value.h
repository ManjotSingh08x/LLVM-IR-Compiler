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

//anything an instruction can use as an operand
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

    //how the value is written as an operand: %t0, @g, 42, null
    virtual std::string getOperand() const = 0;

    std::string getTypedOperand() const { return formatType(type) + " " + getOperand(); }

protected:
    ValueKind         kind;
    const sema::Type *type;
    std::string       name;
};

enum class ConstantKind {
    Int,        //bool, char, int and long, the type gives the width
    Float,
    Null,
    ZeroInit,   //zeroinitializer: an array or record of zeros
    String,     //c"..." for a character array
    Aggregate,
    Address     //@name of a global or function, used inside initializers
};

//a compile-time value, owned by the Module
class Constant : public Value {
public:
    Constant(ConstantKind constKind, const sema::Type *type)
        : Value(ValueKind::Constant, type, ""), constKind(constKind) {}

    std::string getOperand() const override;

    ConstantKind                  constKind;
    long long                     intValue = 0;    //already wrapped to the width of the type
    double                        floatValue = 0.0;
    std::string                   bytes;           //for String: every byte, closing \0 included
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
    External,   //visible to other files: plain globals and functions
    Internal,   //only this file: static globals, static functions, static locals
    Private     //only this file and nameless: string literals
};

//a global variable, as an operand it is its address and valueType is what it holds
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

} // namespace ir

#endif
