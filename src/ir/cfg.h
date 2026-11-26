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

class BasicBlock : public Value {
public:
    BasicBlock(const std::string &label, Function *parent)
        : Value(ValueKind::BasicBlock, sema::voidType(), label), parent(parent) {}

    std::string getOperand() const override { return "%" + name; }

    bool isTerminated() const {
        return !instructions.empty() && instructions.back()->isTerminator();
    }

    void append(std::unique_ptr<Instruction> inst);

    void insertAlloca(std::unique_ptr<Instruction> inst);

    void insertBefore(Instruction *pos, std::unique_ptr<Instruction> inst);
    void insertAtFront(std::unique_ptr<Instruction> inst);

    void erase(Instruction *inst);

    std::unique_ptr<Instruction> detach(Instruction *inst);

    //splits block at instruction, moving the rest
    BasicBlock *splitAt(Instruction *inst, const std::string &label);

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

class Function : public Value {
public:
    Function(const std::string &name, const sema::FunctionType *fnType, const sema::Type *pointerType,
             Linkage linkage)
        : Value(ValueKind::Function, pointerType, name), fnType(fnType), linkage(linkage) {}

    std::string getOperand() const override { return "@" + name; }

    Argument *addArgument(const sema::Type *type, const std::string &baseName);

    BasicBlock *createBlock(const std::string &label);
    void        placeBlock(BasicBlock *block);
    BasicBlock *getEntryBlock() const { return layout.empty() ? nullptr : layout.front(); }
    bool        isDefinition() const { return !layout.empty(); }

    const std::vector<BasicBlock *> &getBlocks() const { return layout; }

    void removeBlock(BasicBlock *block);

    void placeBlockBefore(BasicBlock *block, BasicBlock *pos);
    void placeBlockAfter(BasicBlock *block, BasicBlock *pos);

    void recomputeCFG();

    std::string uniqueName(const std::string &base);
    std::string nextTempName();

    void discard(std::unique_ptr<Instruction> inst);

    //builds block edges and finishes the function
    void finalizeCFG(Module &module);

    void dump(std::ostream &out) const;

    const sema::FunctionType              *fnType;
    Linkage                                linkage;
    bool                                   isUsed = false;
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

    const sema::Type *pointerType();

    Constant *getInt(const sema::Type *type, long long value);
    Constant *getInt32(long long value) { return getInt(sema::intType(), value); }
    Constant *getInt64(long long value) { return getInt(sema::longType(), value); }
    Constant *getBool(bool value) { return getInt(sema::boolType(), value ? 1 : 0); }
    Constant *getFloat(const sema::Type *type, double value);
    Constant *getNull(const sema::Type *type);
    Constant *getZero(const sema::Type *type);
    Constant *getString(const sema::Type *arrayType, const std::string &bytes);
    Constant *getAggregate(const sema::Type *type, const std::vector<const Constant *> &elements);
    Constant *getAddress(const Value *target);

    GlobalVariable *createGlobal(const std::string &name, const sema::Type *valueType,
                                 const Constant *initializer, Linkage linkage, bool isConstant);
    GlobalVariable *findGlobal(const std::string &name) const;

    GlobalVariable *getStringLiteral(const std::string &bytesWithNull);

    Function *createFunction(const std::string &name, const sema::FunctionType *sourceType, Linkage linkage);
    //maps source signature to the lowered one
    const sema::FunctionType *abiSignature(const sema::FunctionType *sourceType);
    Function *findFunction(const std::string &name) const;

    Function *getRuntimeMalloc();
    Function *getRuntimeFree();
    Function *getRuntimeAtexit();

    void addRecord(const sema::RecordSymbol *record) { records.push_back(record); }
    void addClosure(const ast::LambdaExpr *lambda) { closures.push_back(lambda); }

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

}

#endif
