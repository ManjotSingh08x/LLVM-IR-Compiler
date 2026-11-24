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

//a run of instructions entered only at the top and left only by its terminator
class BasicBlock : public Value {
public:
    BasicBlock(const std::string &label, Function *parent)
        : Value(ValueKind::BasicBlock, sema::voidType(), label), parent(parent) {}

    std::string getOperand() const override { return "%" + name; }

    bool isTerminated() const {
        return !instructions.empty() && instructions.back()->isTerminator();
    }

    void append(std::unique_ptr<Instruction> inst);

    //adds an alloca after the existing ones, entry block only
    void insertAlloca(std::unique_ptr<Instruction> inst);

    void insertBefore(Instruction *pos, std::unique_ptr<Instruction> inst);
    void insertAtFront(std::unique_ptr<Instruction> inst);

    //takes an instruction out, the function keeps it alive unwritten
    void erase(Instruction *inst);

    std::unique_ptr<Instruction> detach(Instruction *inst);

    //moves inst and everything after it into a new block right after this one, left unterminated
    BasicBlock *splitAt(Instruction *inst, const std::string &label);

    //the last instruction when it is a terminator, otherwise null
    Instruction *getTerminator() const;

    void dump(std::ostream &out) const;

    static void addEdge(BasicBlock *from, BasicBlock *to);

    Function                                 *parent;
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<BasicBlock *>                 predecessors;
    std::vector<BasicBlock *>                 successors;
    size_t                                    allocaCount = 0;
    bool                                      isPlaced = false;
};

//a function: a declaration when it has no blocks, a definition otherwise
class Function : public Value {
public:
    Function(const std::string &name, const sema::FunctionType *fnType, const sema::Type *pointerType,
             Linkage linkage)
        : Value(ValueKind::Function, pointerType, name), fnType(fnType), linkage(linkage) {}

    std::string getOperand() const override { return "@" + name; }

    Argument *addArgument(const sema::Type *type, const std::string &baseName);

    //makes a block owned by the function, placed in the layout once written into
    BasicBlock *createBlock(const std::string &label);
    void        placeBlock(BasicBlock *block);
    BasicBlock *getEntryBlock() const { return layout.empty() ? nullptr : layout.front(); }
    bool        isDefinition() const { return !layout.empty(); }

    //blocks in layout order, entry first
    const std::vector<BasicBlock *> &getBlocks() const { return layout; }

    //takes a block out of the layout, it stays owned by the function
    void removeBlock(BasicBlock *block);

    void placeBlockBefore(BasicBlock *block, BasicBlock *pos);
    void placeBlockAfter(BasicBlock *block, BasicBlock *pos);

    //rebuilds every predecessor and successor list from the terminators, without repeats
    void recomputeCFG();

    //a unique local name within the function: name, name.1, name.2
    std::string uniqueName(const std::string &base);
    std::string nextTempName();

    //keeps an instruction built after a terminator alive without writing it
    void discard(std::unique_ptr<Instruction> inst);

    //drops unreachable blocks and ends unterminated ones with a zero return
    void finalizeCFG(Module &module);

    void dump(std::ostream &out) const;

    const sema::FunctionType              *fnType;  //the lowered type, with this first for methods
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

class Module {
public:
    Module(sema::SymbolTable &symTab, const std::string &sourceName)
        : symTab(symTab), sourceName(sourceName) {}

    //the type of address values, llvm only sees ptr
    const sema::Type *pointerType();

    //integer constants are wrapped to the width of their type
    Constant *getInt(const sema::Type *type, long long value);
    Constant *getInt32(long long value) { return getInt(sema::intType(), value); }
    Constant *getInt64(long long value) { return getInt(sema::longType(), value); }
    Constant *getBool(bool value) { return getInt(sema::boolType(), value ? 1 : 0); }
    Constant *getFloat(const sema::Type *type, double value);
    Constant *getNull(const sema::Type *type);
    Constant *getZero(const sema::Type *type);   //0, 0.0, null or zeroinitializer
    Constant *getString(const sema::Type *arrayType, const std::string &bytes);
    Constant *getAggregate(const sema::Type *type, const std::vector<const Constant *> &elements);
    Constant *getAddress(const Value *target);

    GlobalVariable *createGlobal(const std::string &name, const sema::Type *valueType,
                                 const Constant *initializer, Linkage linkage, bool isConstant);
    GlobalVariable *findGlobal(const std::string &name) const;

    //one private constant per distinct string literal
    GlobalVariable *getStringLiteral(const std::string &bytesWithNull);

    Function *createFunction(const std::string &name, const sema::FunctionType *sourceType, Linkage linkage);
    const sema::FunctionType *abiSignature(const sema::FunctionType *sourceType);
    Function *findFunction(const std::string &name) const;

    Function *getRuntimeMalloc();
    Function *getRuntimeFree();
    Function *getRuntimeAtexit();

    void addRecord(const sema::RecordSymbol *record) { records.push_back(record); }
    void addClosure(const ast::LambdaExpr *lambda) { closures.push_back(lambda); }

    //the function run before main, null until needed
    Function *globalInit = nullptr;

    void dump(std::ostream &out) const;

    const std::vector<std::unique_ptr<Function>> &getFunctions() const { return functions; }

    const std::vector<std::unique_ptr<GlobalVariable>> &getGlobals() const { return globals; }

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
