#ifndef IR_CFG_H
#define IR_CFG_H

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "ir/instruction.h"

namespace ir {

class Function;
class Module;

//a straight run of instructions that is only entered at the top and only left
//by the terminator at the bottom. the blocks of a function, with the branches
//between them as edges, form its control flow graph
class BasicBlock : public Value {
public:
    BasicBlock(const std::string &label, Function *parent)
        : Value(ValueKind::BasicBlock, sema::voidType(), label), parent(parent) {}

    std::string getOperand() const override { return "%" + name; }

    bool isTerminated() const {
        return !instructions.empty() && instructions.back()->isTerminator();
    }

    //adds an instruction at the end of the block
    void append(std::unique_ptr<Instruction> inst);

    //adds an alloca after the allocas already at the top of the block. only
    //used on the entry block, see IRBuilder::createEntryAlloca
    void insertAlloca(std::unique_ptr<Instruction> inst);

    void dump(std::ostream &out) const;

    //records that control can flow from one block to another
    static void addEdge(BasicBlock *from, BasicBlock *to);

    Function                                 *parent;
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<BasicBlock *>                 predecessors;
    std::vector<BasicBlock *>                 successors;
    size_t                                    allocaCount = 0;
    bool                                      isPlaced = false;  //in the function's layout yet
};

//a function: a declaration when it has no blocks, a definition otherwise
class Function : public Value {
public:
    Function(const std::string &name, const sema::FunctionType *fnType, const sema::Type *pointerType,
             Linkage linkage)
        : Value(ValueKind::Function, pointerType, name), fnType(fnType), linkage(linkage) {}

    std::string getOperand() const override { return "@" + name; }

    Argument *addArgument(const sema::Type *type, const std::string &baseName);

    //makes a block owned by the function. it is not part of the output until
    //placeBlock puts it in the layout, which the builder does when it starts
    //writing into the block, so blocks appear in the order they are filled
    BasicBlock *createBlock(const std::string &label);
    void        placeBlock(BasicBlock *block);
    BasicBlock *getEntryBlock() const { return layout.empty() ? nullptr : layout.front(); }
    bool        isDefinition() const { return !layout.empty(); }

    //every local name (arguments, instruction results and block labels) shares
    //one namespace per function. the first request for a name gets it, later
    //ones get name.1, name.2, ...
    std::string uniqueName(const std::string &base);
    std::string nextTempName();

    //keeps an instruction that was made while no block could take it (code
    //after a return). it is never written out, but stays alive so the values
    //that use it are valid objects
    void discard(std::unique_ptr<Instruction> inst);

    //removes the blocks that cannot be reached from entry and ends every block
    //left without a terminator with a return of the zero value
    void finalizeCFG(Module &module);

    void dump(std::ostream &out) const;

    const sema::FunctionType              *fnType;  //the lowered type: this comes first for methods
    Linkage                                linkage;
    bool                                   isUsed = false;  //called or its address taken
    std::vector<std::unique_ptr<Argument>> args;

private:
    std::vector<std::unique_ptr<BasicBlock>>  ownedBlocks;
    std::vector<BasicBlock *>                 layout;
    std::vector<std::unique_ptr<Instruction>> discarded;
    std::map<std::string, unsigned>           usedNames;
    unsigned                                  tempCounter = 0;
};

//the whole translation unit: types, globals and functions, and the owner of
//every constant
class Module {
public:
    Module(sema::SymbolTable &symTab, const std::string &sourceName)
        : symTab(symTab), sourceName(sourceName) {}

    //the type given to address values: what they point at is tracked by the
    //code that made them, llvm only sees ptr
    const sema::Type *pointerType();

    //constants. integers are wrapped to the width of their type
    Constant *getInt(const sema::Type *type, long long value);
    Constant *getInt32(long long value) { return getInt(sema::intType(), value); }
    Constant *getInt64(long long value) { return getInt(int64Type(), value); }
    Constant *getBool(bool value) { return getInt(sema::boolType(), value ? 1 : 0); }
    Constant *getFloat(const sema::Type *type, double value);
    Constant *getNull(const sema::Type *type);
    Constant *getZero(const sema::Type *type);   //0, 0.0, null or zeroinitializer
    Constant *getString(const sema::Type *arrayType, const std::string &bytes);
    Constant *getAggregate(const sema::Type *type, const std::vector<const Constant *> &elements);
    Constant *getAddress(const Value *target);   //ptr @name of a global or function

    //globals
    GlobalVariable *createGlobal(const std::string &name, const sema::Type *valueType,
                                 const Constant *initializer, Linkage linkage, bool isConstant);
    GlobalVariable *findGlobal(const std::string &name) const;

    //a private constant for a string literal, one per distinct contents:
    //@.str = private unnamed_addr constant [6 x i8] c"hello\00"
    GlobalVariable *getStringLiteral(const std::string &bytesWithNull);

    //functions
    Function *createFunction(const std::string &name, const sema::FunctionType *fnType, Linkage linkage);
    Function *findFunction(const std::string &name) const;

    //the c library functions new and delete are lowered to. they are not
    //symbols of the source program
    Function *getRuntimeMalloc();
    Function *getRuntimeFree();
    Function *getRuntimeAtexit();   //declare i32 @atexit(ptr), runs global destructors

    //named struct types to write at the top of the module
    void addRecord(const sema::RecordSymbol *record) { records.push_back(record); }
    void addClosure(const ast::LambdaExpr *lambda) { closures.push_back(lambda); }

    //the function run before main, listed in @llvm.global_ctors. null until a
    //global needs code to initialise or destroy it
    Function *globalInit = nullptr;

    void dump(std::ostream &out) const;

    sema::SymbolTable &symTab;

private:
    Constant *keep(Constant *constant);

    std::string                                    sourceName;
    const sema::Type                              *genericPointer = nullptr;
    std::vector<const sema::RecordSymbol *>        records;
    std::vector<const ast::LambdaExpr *>           closures;
    std::vector<std::unique_ptr<Constant>>         constants;
    std::vector<std::unique_ptr<GlobalVariable>>   globals;
    std::vector<std::unique_ptr<Function>>         functions;
    std::map<std::string, GlobalVariable *>        globalMap;
    std::map<std::string, Function *>              functionMap;
    std::map<std::string, GlobalVariable *>        stringLiterals;
};

} // namespace ir

#endif
