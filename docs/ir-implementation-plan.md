# IR Generator Implementation Plan & Downstream Agent Specification

This document contains the complete, step-by-step implementation blueprint for building the Intermediate Representation (`src/ir/`) phase of the compiler. It is designed to be self-contained so that any downstream developer or coding agent can review and implement the entire IR generator without ambiguity.

For conceptual background, architectural rationale, and worked C++-to-IR examples, see [`docs/ir-guide.md`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/docs/ir-guide.md).

---

## 1. Prerequisites & Pre-Implementation Fixes Outside `src/ir/`

Before implementing `src/ir/`, apply these three targeted fixes to existing files:

### 1.1 Fix `sema::toLLVMType` for `TypeKind::Record` in [`src/semantic/types.cpp`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/src/semantic/types.cpp#L430-L455)
- **Why**: `sema::toLLVMType` is a debug-printing helper used by `SymbolTable::dump()`. Currently, for `TypeKind::Record`, it returns the full struct definition `"%struct.Point = type { i32, i32 }"` instead of the type reference `"%struct.Point"`. When an array or function type wraps a struct (`Point[4]`), `toLLVMType` produces invalid output `"[4 x %struct.Point = type { i32, i32 }]"`.
- **Change in [`src/semantic/types.cpp`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/src/semantic/types.cpp#L430-L455)**:
  Replace the `case TypeKind::Record:` block in `toLLVMType` with:
  ```cpp
  case TypeKind::Record: {
      const RecordSymbol *rec = asRecord(type);
      return "%" + std::string(rec->isClass ? "class." : "struct.") + rec->mangledName;
  }
  ```
  (Note: `sema::toLLVMRecord` right above `toLLVMType` already prints the full layout for `SymbolKind::Record` symbols in `SymbolTable::dump`!)
  > **Important**: `src/ir/` must **not** call `sema::toLLVMType` or `sema::toLLVMRecord`. Those are strictly for `-symbols.txt` debug dumps. `src/ir/` must implement its own IR type formatting functions (`ir::formatType` and `ir::formatRecordDef`).

### 1.2 Prepopulate `malloc` and `free` in [`src/semantic/symbolbuilder.cpp`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/src/semantic/symbolbuilder.cpp#L58-L68)
- **Why**: `SymbolBuilder::declareBuiltins()` currently only declares `printf` and `scanf`. It should also prepopulate `malloc` and `free` in the global scope of `SymbolTable` for dynamic memory management.
- **Change in [`src/semantic/symbolbuilder.cpp`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/src/semantic/symbolbuilder.cpp#L58-L68)**:
  In `SymbolBuilder::declareBuiltins()`, after registering `printf` and `scanf`, register:
  - `malloc`: `FunctionType(table.pointerTo(voidType()), { intType() }, false)` with `isBuiltin = true`, `loweredType = signature`, `setMangledName(fn, "malloc")`.
  - `free`: `FunctionType(voidType(), { table.pointerTo(voidType()) }, false)` with `isBuiltin = true`, `loweredType = signature`, `setMangledName(fn, "free")`.
  *(Note: When `ir::Module` emits the external declaration for `@malloc` in `.ll`, emit `declare ptr @malloc(i64)` and `declare void @free(ptr)` so 64-bit byte counts from `ptrtoint ... to i64` match `@malloc(i64)` cleanly.)*

### 1.3 Fix Missing Static Member Definition in [`test/test17_brace_static_multidim.ckp`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/test/test17_brace_static_multidim.ckp#L14-L25)
- **Why**: `class Counter` declares `static int total;` and increments it in `Counter::next()`, but lacks the out-of-class definition `int Counter::total = 0;`.
- **Change in [`test/test17_brace_static_multidim.ckp`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/test/test17_brace_static_multidim.ckp#L25)**:
  Add `int Counter::total = 0;` right after `class Counter { ... };`.
  Additionally, in `IRVisitor`, ensure that any static data member (`SymbolKind::Variable` inside a `Record` scope) is emitted as a global variable in `ir::Module` (defaulting to `0` / `zeroinitializer` if not defined out-of-class).

---

## 2. File-by-File Specification for `src/ir/`

All IR generator files live in `src/ir/` under `namespace ir`.

### 2.1 [NEW] `src/ir/value.h` & `src/ir/value.cpp`

Defines the base `ir::Value` class, `ir::Constant`, `ir::Argument`, `ir::GlobalVariable`, and the standalone IR type formatters.

```cpp
#ifndef IR_VALUE_H
#define IR_VALUE_H

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include "semantic/symbols.h"
#include "semantic/types.h"

namespace ir {

class Function;

enum class ValueKind {
    Constant,
    Argument,
    GlobalVariable,
    BasicBlock,
    Instruction
};

class Value {
public:
    Value(ValueKind kind, const sema::Type *type, std::string name = "")
        : kind(kind), type(type), name(std::move(name)) {}
    virtual ~Value() = default;

    Value(const Value &) = delete;
    Value &operator=(const Value &) = delete;

    ValueKind         getKind() const { return kind; }
    const sema::Type *getType() const { return type; }
    const std::string &getName() const { return name; }
    void              setName(std::string newName) { name = std::move(newName); }

    // Returns how this value is referenced as an operand in an instruction:
    // e.g., "%t0", "@global_x", "42", "null", "%entry"
    virtual std::string getOperand() const = 0;

protected:
    ValueKind         kind;
    const sema::Type *type;
    std::string       name;
};

enum class ConstantKind {
    Int,          // i32 or i64 integer constant
    Float,        // float or double constant
    Bool,         // i1 true (1) or false (0)
    Char,         // i8 integer constant
    Null,         // null pointer constant
    ZeroInit,     // zeroinitializer for aggregates/arrays
    String,       // c"...\00" for string literal globals
    Aggregate     // { ... } or [ ... ] constant array/struct initializer
};

class Constant : public Value {
public:
    // Factory helpers owned by ir::Module
    ConstantKind                constKind;
    long long                   intVal = 0;
    double                      floatVal = 0.0;
    std::string                 strVal;
    std::vector<const Constant *> elements;
    bool                        isI64 = false; // true for 64-bit GEP/sizeof constants

    std::string getOperand() const override;
};

class Argument : public Value {
public:
    Argument(const sema::Type *type, std::string name, Function *parent, size_t index)
        : Value(ValueKind::Argument, type, std::move(name)), parent(parent), index(index) {}

    std::string getOperand() const override { return "%" + name; }

    Function *parent;
    size_t    index;
};

class GlobalVariable : public Value {
public:
    GlobalVariable(std::string mangledName, const sema::Type *valueType,
                   const sema::Type *ptrType, const Constant *init = nullptr,
                   bool isConstant = false, bool isInternal = false)
        : Value(ValueKind::GlobalVariable, ptrType, std::move(mangledName)),
          valueType(valueType), initializer(init),
          isConstant(isConstant), isInternal(isInternal) {}

    std::string getOperand() const override { return "@" + name; }
    void dump(std::ostream &out) const;

    const sema::Type *valueType;   // Explicit pointee/allocated type
    const Constant   *initializer; // nullptr for external declarations
    bool              isConstant;  // true for string literals
    bool              isInternal;  // true for static globals and static locals
};

// Standalone LLVM IR type formatters (do NOT use sema::toLLVMType)
std::string formatType(const sema::Type *type);
std::string formatRecordName(const sema::RecordSymbol *record);
std::string formatRecordBody(const sema::RecordSymbol *record);
std::string formatFloatLiteral(double val, bool isFloat32);
std::string escapeLLVMString(const std::string &rawLexerSpelling, size_t &decodedLenWithNull);

} // namespace ir

#endif
```

#### Key Implementation Details for `value.cpp`:
1. **`formatType(const sema::Type *type)`**:
   - `Void` -> `"void"`, `Bool` -> `"i1"`, `Char` -> `"i8"`, `Int` -> `"i32"`, `Float` -> `"float"`, `Double` -> `"double"`
   - `Null`, `Pointer`, `Reference` -> `"ptr"`
   - `Array` -> `"[" + std::to_string(arr->size) + " x " + formatType(arr->element) + "]"`
   - `Record` -> `formatRecordName(asRecord(type))` -> `"%" + (rec->isClass ? "class." : "struct.") + rec->mangledName`
   - `Closure` -> `"%closure." + std::to_string(lambda->line) + "." + std::to_string(lambda->col)`
   - `Function` -> `formatType(fn->returnType) + " (" + params + ")"`
2. **`formatFloatLiteral(double val, bool isFloat32)`**:
   - In LLVM `.ll` syntax, floating-point constants that cannot be represented as exact decimal fractions (like `3.14` or `1.5e-3` for `float`) are rejected by `clang` / `llvm-as` ("floating point constant invalid for type") unless formatted as a 64-bit IEEE-754 hexadecimal constant `0x...`!
   - Specifically: for `float` (`isFloat32 == true`), cast `double(float(val))` and reinterpret its `uint64_t` bits via `std::memcpy`, printing `0x` + 16 hex digits! For `double`, reinterpret `double(val)` bits as `uint64_t` and print `0x` + 16 hex digits! LLVM `clang` / `llvm-as` accepts 16-digit IEEE-754 hex (`0x40091EB860000000`) for **all** `float` and `double` constants with 100% precision and zero rounding errors!

---

### 2.2 [NEW] `src/ir/instruction.h` & `src/ir/instruction.cpp`

Defines `ir::Instruction` and its four distinct sub-categories:
1. **Return & Branch Terminators**: `ReturnInst`, `BranchInst`, `SwitchInst`
2. **Memory Instructions (`MemoryInst`)**: `AllocaInst`, `LoadInst`, `StoreInst`, `GEPInst`
3. **Call Instruction (`CallInst`)**
4. **Expression Instructions (`ExprInst`)**: `BinaryInst`, `UnaryInst`, `CastInst`

```cpp
#ifndef IR_INSTRUCTION_H
#define IR_INSTRUCTION_H

#include <ostream>
#include <utility>
#include <vector>

#include "ir/value.h"

namespace ir {

class BasicBlock;

enum class InstCategory {
    Return,
    Branch,
    Memory,
    Call,
    Expression
};

class Instruction : public Value {
public:
    Instruction(InstCategory category, const sema::Type *type, BasicBlock *parent, std::string name = "")
        : Value(ValueKind::Instruction, type, std::move(name)),
          category(category), parent(parent) {}

    InstCategory getCategory() const { return category; }
    BasicBlock  *getParent() const { return parent; }
    void         setParent(BasicBlock *bb) { parent = bb; }

    virtual bool isTerminator() const { return false; }
    virtual void dump(std::ostream &out) const = 0;

    std::string getOperand() const override { return "%" + name; }

protected:
    InstCategory category;
    BasicBlock  *parent;
};

// 1. Return-Style Terminator
class ReturnInst : public Instruction {
public:
    ReturnInst(Value *returnValue, const sema::Type *voidTy, BasicBlock *parent)
        : Instruction(InstCategory::Return, voidTy, parent), returnValue(returnValue) {}

    bool isTerminator() const override { return true; }
    void dump(std::ostream &out) const override;

    Value *returnValue; // nullptr for ret void
};

// 2. Branch-Style Terminators
class BranchInst : public Instruction {
public:
    // Unconditional: br label %target
    BranchInst(BasicBlock *target, const sema::Type *voidTy, BasicBlock *parent)
        : Instruction(InstCategory::Branch, voidTy, parent),
          target(target), condition(nullptr), falseTarget(nullptr) {}

    // Conditional: br i1 %cond, label %target, label %falseTarget
    BranchInst(Value *cond, BasicBlock *trueTarget, BasicBlock *falseTarget,
               const sema::Type *voidTy, BasicBlock *parent)
        : Instruction(InstCategory::Branch, voidTy, parent),
          target(trueTarget), condition(cond), falseTarget(falseTarget) {}

    bool isTerminator() const override { return true; }
    bool isConditional() const { return condition != nullptr; }
    void dump(std::ostream &out) const override;

    BasicBlock *target;
    Value      *condition;
    BasicBlock *falseTarget;
};

class SwitchInst : public Instruction {
public:
    SwitchInst(Value *condition, BasicBlock *defaultDest, const sema::Type *voidTy, BasicBlock *parent)
        : Instruction(InstCategory::Branch, voidTy, parent),
          condition(condition), defaultDest(defaultDest) {}

    bool isTerminator() const override { return true; }
    void addCase(long long val, BasicBlock *dest) { cases.emplace_back(val, dest); }
    void dump(std::ostream &out) const override;

    Value                                           *condition;
    BasicBlock                                      *defaultDest;
    std::vector<std::pair<long long, BasicBlock *>>  cases;
};

// 3. Memory Instructions (Every memory instruction stores pointee/element sema::Type*)
class MemoryInst : public Instruction {
public:
    MemoryInst(const sema::Type *resultType, const sema::Type *pointeeType,
               BasicBlock *parent, std::string name = "")
        : Instruction(InstCategory::Memory, resultType, parent, std::move(name)),
          pointeeType(pointeeType) {}

    const sema::Type *getPointeeType() const { return pointeeType; }

protected:
    const sema::Type *pointeeType;
};

class AllocaInst : public MemoryInst {
public:
    AllocaInst(const sema::Type *allocatedType, const sema::Type *ptrType,
               BasicBlock *parent, std::string name)
        : MemoryInst(ptrType, allocatedType, parent, std::move(name)) {}

    void dump(std::ostream &out) const override;
};

class LoadInst : public MemoryInst {
public:
    LoadInst(Value *address, const sema::Type *loadType, BasicBlock *parent, std::string name)
        : MemoryInst(loadType, loadType, parent, std::move(name)), address(address) {}

    void dump(std::ostream &out) const override;

    Value *address;
};

class StoreInst : public MemoryInst {
public:
    StoreInst(Value *value, Value *address, const sema::Type *storedType,
              const sema::Type *voidTy, BasicBlock *parent)
        : MemoryInst(voidTy, storedType, parent), value(value), address(address) {}

    void dump(std::ostream &out) const override;

    Value *value;
    Value *address;
};

class GEPInst : public MemoryInst {
public:
    GEPInst(const sema::Type *sourceElementType, const sema::Type *resultPtrType,
            Value *basePtr, std::vector<Value *> indices,
            BasicBlock *parent, std::string name, bool inBounds = true)
        : MemoryInst(resultPtrType, sourceElementType, parent, std::move(name)),
          basePtr(basePtr), indices(std::move(indices)), inBounds(inBounds) {}

    void dump(std::ostream &out) const override;

    Value               *basePtr;
    std::vector<Value *> indices;
    bool                 inBounds;
};

// 4. Call Instruction
class CallInst : public Instruction {
public:
    CallInst(Value *callee, const sema::FunctionType *fnType,
             std::vector<Value *> args, BasicBlock *parent, std::string name = "")
        : Instruction(InstCategory::Call, fnType->returnType, parent, std::move(name)),
          callee(callee), fnType(fnType), args(std::move(args)) {}

    void dump(std::ostream &out) const override;

    Value                    *callee;
    const sema::FunctionType *fnType;
    std::vector<Value *>      args;
};

// 5. Expression Instructions (Pure register value operations)
class ExprInst : public Instruction {
public:
    using Instruction::Instruction;
};

enum class BinaryOpKind {
    Add, FAdd, Sub, FSub, Mul, FMul, SDiv, FDiv, SRem,
    Shl, AShr, And, Or, Xor,
    ICmpEQ, ICmpNE, ICmpSLT, ICmpSGT, ICmpSLE, ICmpSGE,
    FCmpOEQ, FCmpUNE, FCmpOLT, FCmpOGT, FCmpOLE, FCmpOGE
};

class BinaryInst : public ExprInst {
public:
    BinaryInst(BinaryOpKind op, Value *lhs, Value *rhs, const sema::Type *resultType,
               BasicBlock *parent, std::string name)
        : ExprInst(InstCategory::Expression, resultType, parent, std::move(name)),
          op(op), lhs(lhs), rhs(rhs) {}

    void dump(std::ostream &out) const override;

    BinaryOpKind op;
    Value       *lhs;
    Value       *rhs;
};

enum class UnaryOpKind {
    FNeg
};

class UnaryInst : public ExprInst {
public:
    UnaryInst(UnaryOpKind op, Value *operand, const sema::Type *resultType,
              BasicBlock *parent, std::string name)
        : ExprInst(InstCategory::Expression, resultType, parent, std::move(name)),
          op(op), operand(operand) {}

    void dump(std::ostream &out) const override;

    UnaryOpKind op;
    Value      *operand;
};

enum class CastOpKind {
    SExt, ZExt, Trunc,
    SIToFP, UIToFP, FPToSI,
    FPExt, FPTrunc,
    PtrToInt, IntToPtr, BitCast
};

class CastInst : public ExprInst {
public:
    CastInst(CastOpKind op, Value *operand, const sema::Type *fromType,
             const sema::Type *toType, BasicBlock *parent, std::string name,
             bool toI64 = false)
        : ExprInst(InstCategory::Expression, toType, parent, std::move(name)),
          op(op), operand(operand), fromType(fromType), toType(toType), toI64(toI64) {}

    void dump(std::ostream &out) const override;

    CastOpKind        op;
    Value            *operand;
    const sema::Type *fromType;
    const sema::Type *toType;
    bool              toI64; // true when casting ptrtoint to i64 or sext i32 to i64
};

} // namespace ir

#endif
```

---

### 2.3 [NEW] `src/ir/cfg.h` & `src/ir/cfg.cpp`

Defines `ir::BasicBlock`, `ir::Function`, and `ir::Module`.

```cpp
#ifndef IR_CFG_H
#define IR_CFG_H

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "ir/instruction.h"

namespace ir {

class Function;

class BasicBlock : public Value {
public:
    BasicBlock(std::string label, Function *parent)
        : Value(ValueKind::BasicBlock, sema::voidType(), std::move(label)), parent(parent) {}

    std::string getOperand() const override { return "%" + name; }

    bool isTerminated() const {
        return !instructions.empty() && instructions.back()->isTerminator();
    }

    void addInstruction(std::unique_ptr<Instruction> inst);
    void insertAllocaAfterLastAlloca(std::unique_ptr<AllocaInst> allocaInst);
    void dump(std::ostream &out, bool isEntry) const;

    Function                                  *parent;
    std::vector<std::unique_ptr<Instruction>>  instructions;
    std::vector<BasicBlock *>                  predecessors;
    std::vector<BasicBlock *>                  successors;

    static void addEdge(BasicBlock *from, BasicBlock *to);
};

class Function : public Value {
public:
    Function(std::string mangledName, const sema::FunctionType *fnType,
             const sema::Type *ptrType, bool isExternal = false, bool isInternal = false)
        : Value(ValueKind::Function, ptrType, std::move(mangledName)),
          fnType(fnType), isExternal(isExternal), isInternal(isInternal) {}

    std::string getOperand() const override { return "@" + name; }

    Argument   *addArgument(const sema::Type *type, std::string argName);
    BasicBlock *createBasicBlock(std::string label);
    BasicBlock *getEntryBlock() const { return blocks.empty() ? nullptr : blocks.front().get(); }

    // Removes empty unreachable non-entry blocks and ensures all surviving blocks have a terminator
    void finalizeCFG(class Module &mod);
    void dump(std::ostream &out) const;

    const sema::FunctionType                 *fnType;
    bool                                      isExternal;
    bool                                      isInternal;
    std::vector<std::unique_ptr<Argument>>    args;
    std::vector<std::unique_ptr<BasicBlock>>  blocks;
    size_t                                    allocaInsertIndex = 0;
};

struct ClosureRecordDef {
    std::string                    name; // e.g., "%closure.265.22"
    std::vector<const sema::Type *> fieldTypes;
};

class Module {
public:
    Module(sema::SymbolTable &symTab) : symTab(symTab) {}

    Constant *getInt32(long long val);
    Constant *getInt64(long long val);
    Constant *getBool(bool val);
    Constant *getChar(char val);
    Constant *getFloat(double val, const sema::Type *ty);
    Constant *getNull(const sema::Type *ptrTy);
    Constant *getZeroInit(const sema::Type *ty);
    Constant *getAggregate(const sema::Type *ty, std::vector<const Constant *> elems);
    GlobalVariable *getOrCreateStringLiteral(const std::string &rawSpelling);

    void registerRecord(const sema::RecordSymbol *record);
    void registerClosureRecord(std::string name, std::vector<const sema::Type *> fields);
    GlobalVariable *createGlobal(std::string mangledName, const sema::Type *valTy,
                                 const Constant *init, bool isConstant, bool isInternal);
    Function *getOrCreateFunction(std::string mangledName, const sema::FunctionType *fnType,
                                  bool isExternal, bool isInternal = false);

    void dump(std::ostream &out) const;

    sema::SymbolTable                              &symTab;
    std::vector<const sema::RecordSymbol *>         records;
    std::vector<ClosureRecordDef>                   closures;
    std::vector<std::unique_ptr<Constant>>          constants;
    std::vector<std::unique_ptr<GlobalVariable>>    globals;
    std::vector<std::unique_ptr<Function>>          functions;
    llvm::StringMap<GlobalVariable *>               stringLiterals;
    llvm::StringMap<Function *>                     functionMap;
    llvm::StringMap<GlobalVariable *>               globalMap;
};

} // namespace ir

#endif
```

---

### 2.4 [NEW] `src/ir/ir_builder.h` & `src/ir/ir_builder.cpp`

`ir::IRBuilder` is the factory that creates instructions, inserts them into the active `BasicBlock`, assigns unique sequential register and label names, and updates CFG edges (`BasicBlock::addEdge`).

#### Key Responsibilities of `IRBuilder`:
1. **Entry-Block `alloca` Hoisting**:
   - `AllocaInst *createEntryAlloca(Function *fn, const sema::Type *allocatedType, const std::string &varName)`:
   - Inserts `AllocaInst` at `fn->getEntryBlock()` at index `fn->allocaInsertIndex++` so all `alloca`s stay at the very top of `entry` before any stores or branches.
2. **Terminator Guard**:
   - Every instruction creation method checks `if (!insertBlock || insertBlock->isTerminated())`. If already terminated, it does not append to `insertBlock` (preventing invalid instructions after `ret` / `br`).
3. **Automatic CFG Edge Maintenance**:
   - `createBr(BasicBlock *dest)`: calls `BasicBlock::addEdge(insertBlock, dest)` and appends `BranchInst`.
   - `createCondBr(Value *cond, BasicBlock *trueBB, BasicBlock *falseBB)`: calls `BasicBlock::addEdge(insertBlock, trueBB)` and `BasicBlock::addEdge(insertBlock, falseBB)` and appends `BranchInst`.
   - `createSwitch(Value *cond, BasicBlock *defaultBB)`: calls `BasicBlock::addEdge(insertBlock, defaultBB)` and appends `SwitchInst` (and when cases are added via `addSwitchCase`, adds edges to each case block).

---

### 2.5 [NEW] `src/ir/ir_visitor.h` & `src/ir/ir_visitor.cpp`

`ir::IRVisitor` inherits from `ast::ASTVisitor<IRVisitor, ir::Value *>` (`src/semantic/visitor.h`) and drives `IRBuilder` to construct the `ir::Module`.

#### Internal Data Structures of `IRVisitor`:
```cpp
struct LoopTargets {
    BasicBlock *continueBlock;
    BasicBlock *breakBlock;
    size_t      scopeDepth; // number of active RAII scopes outside/at loop entry
};

struct SwitchContext {
    SwitchInst *switchInst;
    BasicBlock *defaultBlock;
    BasicBlock *endBlock;
    size_t      scopeDepth;
};

struct LocalCleanup {
    Value              *address; // alloca ptr
    const sema::Type   *type;    // RecordType or ArrayType of RecordType
};

class IRVisitor : public ast::ASTVisitor<IRVisitor, Value *> {
    Module                                     &mod;
    sema::SymbolTable                          &symTab;
    IRBuilder                                   builder;

    Function                                   *currentFn = nullptr;
    Value                                      *currentThisPtr = nullptr; // loaded 'this' ptr in methods/lambdas
    std::unordered_map<const sema::Symbol *, Value *> namedValues; // locals, params, static locals
    std::unordered_map<const ast::LabelStmt *, BasicBlock *> labelBlocks;

    std::vector<LoopTargets>                    loopStack;
    std::vector<SwitchContext>                  switchStack;
    std::vector<std::vector<LocalCleanup>>      scopeCleanupStack;
    int                                         staticLocalCounter = 0;
};
```

#### Step-by-Step Pass Order in `IRVisitor::generate(ast::Program *program)`:
1. **Pass 1 — Register Record Types & Static Data Members**:
   - Recursively walk `symTab.global()` scopes to collect all `sema::RecordSymbol`s in declaration order (`mod.registerRecord(rec)`).
   - For each `RecordSymbol`, inspect `rec->members->symbols`: if a symbol has `SymbolKind::Variable` (`isStatic == true`), pre-create its `GlobalVariable` in `mod` with default zero initialization (an out-of-class `VarDecl` in `program->decls` will update its initializer if present).
2. **Pass 2 — Register Global Variables & External Function Declarations**:
   - Walk `program->decls`:
     - For file-scope `VarDecl`: evaluate constant initializer via `sema::evaluateConstant` (or float/string/aggregate constant) and create/update `GlobalVariable`. (If a global has a non-constant initializer or constructor call, record it for `__cxx_global_var_init` / entry of `main`).
     - Pre-declare `@printf`, `@scanf`, `@malloc`, `@free` in `mod` when used.
3. **Pass 3 — Emit Function, Method, Constructor, and Destructor Bodies**:
   - Walk `program->decls` (and recursively `RecordDecl::members` for in-class methods/ctors/dtors).
   - Also synthesize `= default` constructors and destructors when a `RecordSymbol` declares a constructor/destructor without a definition (`fn->definition == nullptr` in `test4_oop_features.ckp`) or needs an implicit constructor/destructor to chain base class or in-class member initializers (`int start{0}` in `test17_brace_static_multidim.ckp`).

---

## 3. Updates to `src/parser.y`, `Makefile`, and `run.sh`

### 3.1 [MODIFY] [`src/parser.y`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/src/parser.y#L1145-L1187)
Update `openOutput` and `analyze` so that all outputs for `test/<stem>.ckp` are placed inside the per-test subfolder `outputs/<stem>/`:
- `outputs/<stem>/<stem>-ast.txt`
- `outputs/<stem>/<stem>-symbols.txt`
- `outputs/<stem>/<stem>.ll` (generated only when `diags.empty()` is true!)

```cpp
// Opens outputs/<stem>/<stem><suffix> for writing
static std::ofstream openOutput(const char *sourcePath, const std::string &suffix) {
    std::string stem = std::filesystem::path(sourcePath).stem().string();
    std::filesystem::path outDir = std::filesystem::path("outputs") / stem;
    std::error_code ec;
    std::filesystem::create_directories(outDir, ec);

    std::filesystem::path path = outDir / (stem + suffix);
    std::ofstream file(path);
    if (!file) cerr << "error: cannot write " << path.string() << "\n";
    return file;
}
```
And at the end of `analyze(const char *sourcePath)`:
```cpp
    if (!diags.empty()) {
        diags.print(cout, sourcePath);
        return false;
    }

    ir::Module irMod(table);
    ir::IRVisitor irGen(irMod, table);
    irGen.generate(g_program.get());

    std::ofstream irFile = openOutput(sourcePath, ".ll");
    if (irFile) irMod.dump(irFile);

    return true;
```

### 3.2 [MODIFY] [`Makefile`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/Makefile#L33-L34)
Add `src/ir/*.cpp` and `src/ir/*.h` to `AST_SRCS` and `AST_HDRS`:
```makefile
AST_SRCS := $(wildcard $(SRC)/ast/*.cpp) $(wildcard $(SRC)/semantic/*.cpp) $(wildcard $(SRC)/ir/*.cpp)
AST_HDRS := $(wildcard $(SRC)/ast/*.h) $(wildcard $(SRC)/semantic/*.h) $(wildcard $(SRC)/ir/*.h) $(SRC)/declarator.h
```

### 3.3 [MODIFY] [`run.sh`](file:///home/manjot/Files/Code/CompilerDesign/LLVM-IR-Compiler/run.sh)
Rewrite `run.sh` so that for each `test/<base>.ckp`:
1. Creates subfolder `outputs/<base>/`.
2. Copies `test/<base>.ckp` into `outputs/<base>/<base>.ckp` for reference.
3. Runs `"$EXE" "$f" > "outputs/$base/$base.txt" 2>&1`.
4. If the compiler succeeds (exit code 0, meaning Lexer + Parser + SymbolBuilder + TypeChecker + IRGenerator all succeeded):
   - Verifies that `outputs/$base/$base.ll` was generated.
   - Runs `clang -x ir -fsyntax-only -Wno-override-module "outputs/$base/$base.ll"` to verify that the `.ll` file is syntactically and semantically valid LLVM IR!
   - Reports `PARSED & IR VALID` (or `IR INVALID` if `clang` rejects the `.ll`).

---

## 4. Verification & Test Suite Matrix

Downstream agents implementing this specification must verify the following criteria:

1. **Zero Build Warnings**:
   ```bash
   make clean && make all
   ```
   Must compile cleanly with `-Wall -std=c++17`.
2. **All 22 Test Files in `bash run.sh`**:
   - **10 Accepted Tests (`test1`, `test2`, `test3`, `test4`, `test5`, `test7`, `test12`, `test17`, `test22`)**:
     Must pass all compiler phases, populate `outputs/<name>/` with `<name>.ckp`, `<name>.txt`, `<name>-ast.txt`, `<name>-symbols.txt`, and `<name>.ll`, and pass `clang -x ir -fsyntax-only outputs/<name>/<name>.ll` with 0 errors.
   - **12 Rejected Error Tests (`test6`, `test8`, `test9`, `test10`, `test11`, `test13`, `test14`, `test15`, `test16`, `test18`, `test19`, `test20`, `test21`)**:
     Must be rejected with the exact expected error count and must **not** emit a `.ll` file.
