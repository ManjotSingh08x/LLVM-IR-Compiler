#ifndef IR_VALUE_H
#define IR_VALUE_H

#include <ostream>
#include <string>
#include <vector>

#include "ir/ir_types.h"

namespace ir {

class Function;

//what a Value is, so code holding a Value * can tell the kinds apart
enum class ValueKind {
    Constant,
    Argument,
    GlobalVariable,
    Function,
    BasicBlock,
    Instruction
};

//anything an instruction can use as an operand, with a sema type and a name
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

    //how the value is written where it is used: %t0, @global_1x, 42, null
    virtual std::string getOperand() const = 0;

    //the operand with its type in front: i32 %t0
    std::string getTypedOperand() const { return formatType(type) + " " + getOperand(); }

protected:
    ValueKind         kind;
    const sema::Type *type;
    std::string       name;
};

enum class ConstantKind {
    Int,        //bool, char, int and i64 values. the type gives the width
    Float,      //float and double
    Null,       //the null pointer
    ZeroInit,   //zeroinitializer: an array or record of zeros
    String,     //c"..." for a character array
    Aggregate,  //[ ... ] for an array or { ... } for a record
    Address     //the address of a global or a function, @name, used inside initializers
};

//a value known while compiling. constants are made by the Module, which owns them
class Constant : public Value {
public:
    Constant(ConstantKind constKind, const sema::Type *type)
        : Value(ValueKind::Constant, type, ""), constKind(constKind) {}

    std::string getOperand() const override;

    ConstantKind                  constKind;
    long long                     intValue = 0;    //already wrapped to the width of the type
    double                        floatValue = 0.0;
    std::string                   bytes;           //for String: every byte, closing \0 included
    std::vector<const Constant *> elements;        //for Aggregate, in order
    const Value                  *target = nullptr; //for Address: the global or function
};

//a parameter of a function as it arrives: %a in define i32 @f(i32 %a)
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

    //@name = internal global i32 0
    void dump(std::ostream &out) const;

    const sema::Type *valueType;
    const Constant   *initializer;
    Linkage           linkage;
    bool              isConstant;
};

} // namespace ir

#endif
