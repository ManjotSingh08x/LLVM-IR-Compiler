# IR Generator Implementation Plan & Downstream Agent Specification

This document contains the complete, step-by-step implementation blueprint for building the Intermediate Representation (`src/ir/`) phase of the compiler. It is designed to be self-contained so that any downstream developer or coding agent can review and implement the entire IR generator without ambiguity.

For conceptual background, architectural rationale, and worked C++-to-IR examples, see [`docs/ir-guide.md`](ir-guide.md).

> **Status (2026-10-24): implemented.** The code in `src/ir/` follows this plan. Every detail that changed while it was built is listed, with the reason, in [`ir-change-log.md` §7](ir-change-log.md#7-where-the-implementation-differs-from-the-plan-and-why). Where a code listing below and that table disagree, the table describes the code as built. No grammar rule was changed ([change log §1](ir-change-log.md#1-grammar-changes)).

---

## 1. Prerequisites & Pre-Implementation Fixes Outside `src/ir/`

Before implementing `src/ir/`, apply these targeted fixes to existing files (1.1–1.5). Section 5 of this document is the edge-case rulebook; read it before writing `ir_visitor.cpp`.

### 1.1 Fix `sema::toLLVMType` for `TypeKind::Record` in [`src/semantic/types.cpp`](../src/semantic/types.cpp#L430-L455)
- **Why**: `sema::toLLVMType` is a debug-printing helper used by `SymbolTable::dump()`. Currently, for `TypeKind::Record`, it returns the full struct definition `"%struct.Point = type { i32, i32 }"` instead of the type reference `"%struct.Point"`. When an array or function type wraps a struct (`Point[4]`), `toLLVMType` produces invalid output `"[4 x %struct.Point = type { i32, i32 }]"`.
- **Change in [`src/semantic/types.cpp`](../src/semantic/types.cpp#L430-L455)**:
  Replace the `case TypeKind::Record:` block in `toLLVMType` with:
  ```cpp
  case TypeKind::Record: {
      const RecordSymbol *rec = asRecord(type);
      return "%" + std::string(rec->isClass ? "class." : "struct.") + rec->mangledName;
  }
  ```
  (Note: `sema::toLLVMRecord` right above `toLLVMType` already prints the full layout for `SymbolKind::Record` symbols in `SymbolTable::dump`!)
  > **Important**: `src/ir/` must **not** call `sema::toLLVMType` or `sema::toLLVMRecord`. Those are strictly for `-symbols.txt` debug dumps. `src/ir/` must implement its own IR type formatting functions (`ir::formatType`, `ir::formatRecordName` and `ir::formatRecordBody`, declared in §2.1).

### 1.2 `malloc` and `free` are IR-internal runtime functions — do **not** add them to `SymbolBuilder::declareBuiltins()`
An earlier revision of this plan registered `malloc(int)` and `free(void*)` as source-visible builtins. That is dropped, because:
- **They are never called from source.** `new`/`delete` are lowered by `IRVisitor` directly to `@malloc`/`@free`; no `CallExpr` ever names them.
- **Users could not call them usefully anyway.** `malloc` would return `void *`, and the language has no explicit casts and no implicit `void * -> T *` conversion (`findConversion` only allows `T * -> void *`), so `int *p = malloc(4);` is a type error.
- **The sema signature would contradict the IR one.** Sema would type the parameter as `int` (`i32`) while the module declares `@malloc(i64)`. A direct call would then print `call ptr @malloc(i32 ...)` against `declare ptr @malloc(i64)`.
- **It would reserve the names.** A user function or variable named `free` or `malloc` would start failing with "redefinition" or "same mangled name".

**Instead**, `ir::Module` owns two runtime helpers, created lazily on first use:
```llvm
declare ptr @malloc(i64)
declare void @free(ptr)
```
`Module::getRuntimeMalloc()` / `getRuntimeFree()` build these with IR-only `sema::FunctionType`s made through `symTab.functionType(...)`. The `i64` parameter uses the `ir::int64Type()` sentinel from §2.1. Do not insert them into any `Scope`.

### 1.3 Fix compound assignment conversion in [`src/semantic/typecheck.cpp`](../src/semantic/typecheck.cpp) (`visitAssignExpr`)
- **Why**: For `+= -= *= /= %=` on arithmetic targets, `visitAssignExpr` calls `convert(node->value, target, ...)`. That converts the **right operand to the target type before the operation**, which C++ does not do:
  - `int x = 7; x *= 2.5;` → sema converts `2.5` to `2` first, so `x == 14` instead of C++'s `17`.
  - `bool b = true; b += 1;` → sema converts `1` to `true` (`i1 1`), and `add i1 1, 1` gives `false` instead of `true`.
  - `char c = 100; c += 300;` → `300` is truncated to `i8 44` before the add.
- **Change**: In the arithmetic branch (not the pointer `+=`/`-=` branch), convert the value to `commonArithmeticType(promote(target), promote(value))` instead of `target`. The IR then does what C++ specifies: `load` the target → convert it to the common type → operate → convert the result back to the target type (a `ToBool` when the target is `bool`) → `store`. `IRVisitor` emits both of those conversions itself through `emitArithmeticCast(Value*, from, to)`. They are not in the AST.
- **If sema cannot be changed**, `IRVisitor` must strip the single outermost arithmetic `ImplicitCastExpr` (kinds `IntegralCast`, `IntegralToFloating`, `FloatingToIntegral`, `FloatingCast`, `ToBool`) from `node->value` and apply the rule above to the unconverted operand. Only `visitAssignExpr` adds such a cast at the top of `node->value`; `decayValue` adds only `ArrayToPointer`/`FunctionToPointer`, so stripping one arithmetic cast there is safe.

### 1.4 Expose the string-literal decoder in [`src/semantic/constants.h`](../src/semantic/constants.h)
- **Why**: Sema sizes every string literal as `[stringLiteralLength(spelling) + 1 x i8]` using the private `readChar` in `constants.cpp`. If `ir::escapeLLVMString` decodes escapes with a second, independent implementation, the two can disagree (`\x41`, `\0`, `\'`). The global's `[N x i8]` would then not match the `c"..."` length or the `ArrayType` that sema recorded for the expression.
- **Change**: Add `std::string decodeStringLiteral(const std::string &spelling);` (returns the raw bytes without quotes and without the trailing `\0`), implemented with the same `readChar` loop as `stringLiteralLength`. `ir::escapeLLVMString` takes the **decoded bytes**: it prints every byte outside `0x20..0x7E`, plus `"` and `\`, as `\XX` (two uppercase hex digits). *(As built: the caller passes the closing `\0` and any padding itself, so one function serves both `"hi"` and `char s[8] = "hi"`.)*

### 1.5 Fix Missing Static Member Definition in [`test/test17_brace_static_multidim.ckp`](../test/test17_brace_static_multidim.ckp#L14-L25)
- **Why**: `class Counter` declares `static int total;` and increments it in `Counter::next()`, but lacks the out-of-class definition `int Counter::total = 0;`.
- **Change in [`test/test17_brace_static_multidim.ckp`](../test/test17_brace_static_multidim.ckp#L25)**:
  Add `int Counter::total = 0;` right after `class Counter { ... };`.
  Additionally, in `IRVisitor`, ensure that any static data member (`SymbolKind::Variable` inside a `Record` scope) is emitted as a global variable in `ir::Module` (defaulting to `0` / `zeroinitializer` if not defined out-of-class).
  > Note: the test fix is only about C++ conformance. Because `IRVisitor` always emits a zero-initialized definition for an undefined static member, the `.ll` is valid with or without it, and `Animal::created` in `test4` is handled the same way. Linking and running the program does not fail either.

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
    Function,      // used by ir::Function in cfg.h
    BasicBlock,
    Instruction
};

// sema has no 64-bit integer type, but GEP sizeof computations, malloc's
// parameter and pointer subtraction need one. This is a distinct
// sema::Type(TypeKind::Int) instance owned by src/ir; formatType checks it
// by address *before* switching on the kind and prints "i64". It is never
// handed back to sema (isSameType would treat it as int).
const sema::Type *int64Type();

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
    // Created only through ir::Module factories, which own them
    Constant(ConstantKind constKind, const sema::Type *type)
        : Value(ValueKind::Constant, type), constKind(constKind) {}

    ConstantKind                constKind;
    long long                   intVal = 0;   // already normalized to the type's width (see §5.1)
    double                      floatVal = 0.0;
    std::string                 strVal;       // decoded bytes for String
    std::vector<const Constant *> elements;
    // 64-bit-ness comes from type == int64Type(); there is no separate isI64 flag

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
std::string escapeLLVMString(const std::string &decodedBytes); // from sema::decodeStringLiteral (§1.4), appends \00
long long   normalizeInt(long long val, const sema::Type *type); // wraps to i1/i8/i32/i64, see §5.1

} // namespace ir

#endif
```

#### Key Implementation Details for `value.cpp`:
1. **`formatType(const sema::Type *type)`**:
   - First: `if (type == int64Type()) return "i64";`
   - `Void` -> `"void"`, `Bool` -> `"i1"`, `Char` -> `"i8"`, `Int` -> `"i32"`, `Float` -> `"float"`, `Double` -> `"double"`
   - `Auto`, `Error`, and an `Array` with `size == 0` must never reach IR generation. Sema rejects all three before IR runs, and unsized `[]` parameters were already turned into pointers. Treat them as an internal error (`assert` plus a diagnostic), not as `"<none>"`.
   - `bool` is stored as `i1` everywhere: in memory, in struct fields, and in arguments. That is valid IR, but it differs from clang's ABI, which stores `bool` as `i8`. Nothing here links against clang-compiled C++, so it doesn't matter, but use `i1` consistently. Never mix `i1` and `i8` for the same bool.
   - `Null`, `Pointer`, `Reference` -> `"ptr"`
   - `Array` -> `"[" + std::to_string(arr->size) + " x " + formatType(arr->element) + "]"`
   - `Record` -> `formatRecordName(asRecord(type))` -> `"%" + (rec->isClass ? "class." : "struct.") + rec->mangledName`
   - `Closure` -> `"%closure." + std::to_string(lambda->line) + "." + std::to_string(lambda->col)`
   - `Function` -> `formatType(fn->returnType) + " (" + params + ")"`
2. **`formatFloatLiteral(double val, bool isFloat32)`**:
   - In LLVM `.ll` syntax, a **`float`** constant written in decimal must be exactly representable as a float. `float 3.14` is rejected with "floating point constant invalid for type" (verified with Apple clang 17). Decimal `double` constants such as `double 3.14` are accepted, but printing every FP constant as a 16-digit hex value is simpler and lossless.
   - The source is `FloatLiteral::spelling`. Parse it with `std::strtod`, which accepts `.5`, `2.` and `1e10` as they appear in `test5`/`test7`. Every `FloatLiteral` is `double` in sema; `float pi = 3.14` arrives as `ImplicitCastExpr(FloatingCast, FloatLiteral)`. When folding that to a `float` constant, round with `(float)val` **before** printing.
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
    ICmpULT, ICmpUGT, ICmpULE, ICmpUGE,   // pointer relational compares (clang uses unsigned for ptr)
    FCmpOEQ, FCmpUNE, FCmpOLT, FCmpOGT, FCmpOLE, FCmpOGE
};
// Operand types reaching BinaryInst are only i32, i64 (int64Type), float, double,
// ptr (compares only) and i1 (only for `xor i1 %v, true` used by `!`).
// Sema promotes bool/char before arithmetic, so there is no i8/i1 add/mul.
// Result type of every compare is boolType() (i1).

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
             const sema::Type *toType, BasicBlock *parent, std::string name)
        : ExprInst(InstCategory::Expression, toType, parent, std::move(name)),
          op(op), operand(operand), fromType(fromType), toType(toType) {}

    void dump(std::ostream &out) const override;

    CastOpKind        op;
    Value            *operand;
    const sema::Type *fromType;
    const sema::Type *toType;
    // (no toI64 flag: casting to i64 means toType == int64Type())
};

// `select i1 %c, ptr %a, ptr %b` — used ONLY for the null-safe DerivedToBase
// pointer adjustment (§5.6). Never use it for ?: (both arms would be evaluated).
class SelectInst : public ExprInst {
public:
    SelectInst(Value *cond, Value *ifTrue, Value *ifFalse, BasicBlock *parent, std::string name)
        : ExprInst(InstCategory::Expression, ifTrue->getType(), parent, std::move(name)),
          cond(cond), ifTrue(ifTrue), ifFalse(ifFalse) {}
    void dump(std::ostream &out) const override;
    Value *cond, *ifTrue, *ifFalse;
};

// `unreachable` — terminator used by Function::finalizeCFG for blocks that are
// kept but can never be executed (see §5.4). Category Branch, no successors.
class UnreachableInst : public Instruction {
public:
    UnreachableInst(const sema::Type *voidTy, BasicBlock *parent)
        : Instruction(InstCategory::Branch, voidTy, parent) {}
    bool isTerminator() const override { return true; }
    void dump(std::ostream &out) const override { out << "  unreachable\n"; }
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

#include "llvm/ADT/StringMap.h"   // Module uses llvm::StringMap; include it explicitly
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

    // 1. DFS from entry over `successors`; delete EVERY non-entry block not reached
    //    (not just empty ones) and erase it from the pred lists of its successors.
    // 2. Every surviving block without a terminator gets one: the fallback return
    //    (§5.4) if it is the function's natural fall-off point, else `unreachable`.
    // See §5.4 for why "remove only empty pred-less blocks" is insufficient.
    void finalizeCFG(class Module &mod);
    void dump(std::ostream &out) const;

    // Every local name (%args, %x.addr allocas, %tN temps, block labels) lives in
    // ONE namespace per function in LLVM. uniqueName("x.addr") returns "x.addr",
    // then "x.addr1", "x.addr2", ...; uniqueName("if.then") likewise. See §5.3.
    std::string uniqueName(const std::string &base);
    llvm::StringMap<unsigned> usedNames;
    unsigned                  nextTemp = 0;   // %t0, %t1, ... also reserved via usedNames

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

    // Runtime helpers, never visible to sema (§1.2): declare ptr @malloc(i64), declare void @free(ptr)
    Function *getRuntimeMalloc();
    Function *getRuntimeFree();

    // Dynamic initialization of globals / static members (§5.8). When non-empty, dump()
    // emits `define internal void @__global_init()` and
    // @llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__global_init, ptr null }]
    Function *globalInitFn = nullptr;

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
   - **CFG edges are added only when the terminator is actually inserted.** If `createBr`/`createCondBr`/`createSwitch` is suppressed by the guard, it must **not** call `addEdge`. Otherwise dead code would give blocks fake predecessors.
   - A suppressed creation method still has to return a usable `Value *`, because callers chain on the result. Return a placeholder: a `Constant` zero/`null` of the requested type, or the (unused) `alloca` for addresses. **Never** return `nullptr`. Code after `return`/`break`/`goto` is still visited, e.g. `return 0; x = y + 1;`, and the visitor must not crash on it.
   - Blocks created **after** the terminator (for example the `while.cond` of a loop that follows a `return`) are real blocks with no predecessors. `finalizeCFG` removes them (§5.4).
3. **Insertion-Point Save/Restore**:
   - `struct InsertPoint { Function *fn; BasicBlock *bb; }; InsertPoint saveIP(); void restoreIP(InsertPoint);`
   - Required because a `LambdaExpr` is lowered by emitting a **whole new function** (`@__lambda_L_C`) while the enclosing function is half built (§5.9), and because implicit constructors/destructors may be synthesized on first use.
4. **Automatic CFG Edge Maintenance**:
   - `createBr(BasicBlock *dest)`: calls `BasicBlock::addEdge(insertBlock, dest)` and appends `BranchInst`.
   - `createCondBr(Value *cond, BasicBlock *trueBB, BasicBlock *falseBB)`: calls `BasicBlock::addEdge(insertBlock, trueBB)` and `BasicBlock::addEdge(insertBlock, falseBB)` and appends `BranchInst`.
   - `createSwitch(Value *cond, BasicBlock *defaultBB)`: calls `BasicBlock::addEdge(insertBlock, defaultBB)` and appends `SwitchInst` (and when cases are added via `addSwitchCase`, adds edges to each case block).

---

### 2.5 [NEW] `src/ir/ir_visitor.h` & `src/ir/ir_visitor.cpp`

`ir::IRVisitor` inherits from `ast::ASTVisitor<IRVisitor, ir::Value *>` (`src/semantic/visitor.h`) and drives `IRBuilder` to construct the `ir::Module`.

#### Internal Data Structures of `IRVisitor`:
```cpp
// `break` binds to the innermost loop OR switch; `continue` binds to the innermost
// loop only. Two stacks express this exactly (a single loopStack + switchStack pair
// cannot tell which of the two is innermost).
struct JumpTarget {
    BasicBlock *block;
    size_t      cleanupDepth; // scopeCleanupStack.size() when the loop/switch was entered
};

struct SwitchContext {
    SwitchInst *switchInst;
    BasicBlock *endBlock;     // also the default destination until a DefaultStmt is seen
    bool        sawDefault = false;
};

struct LocalCleanup {
    Value              *address; // alloca ptr
    const sema::Type   *type;    // RecordType or ArrayType of RecordType
};

// Everything that belongs to ONE function being emitted. Lambda bodies and synthesized
// ctors/dtors are emitted while another function is half built, so IRVisitor keeps a
// stack of these and swaps the whole thing (plus builder.saveIP()/restoreIP()).
struct FunctionContext {
    Function                                         *fn = nullptr;
    const sema::Type                                 *returnType = nullptr; // sema return type (may be Reference)
    Value                                            *thisPtr = nullptr;    // loaded 'this' (methods, ctors, dtors, [this] lambdas)
    const sema::RecordSymbol                         *thisRecord = nullptr; // record 'this' points at
    Value                                            *retvalSlot = nullptr; // alloca for the return value when cleanups exist (§5.7)
    std::unordered_map<const sema::Symbol *, Value *> namedValues;          // locals, params, captures -> address
    std::unordered_map<const ast::LabelStmt *, BasicBlock *> labelBlocks;   // created on first goto OR label, whichever comes first
    std::vector<JumpTarget>                           breakStack;
    std::vector<JumpTarget>                           continueStack;
    std::vector<SwitchContext>                        switchStack;
    std::vector<std::vector<LocalCleanup>>            scopeCleanupStack;
};

class IRVisitor : public ast::ASTVisitor<IRVisitor, Value *> {
    Module                                     &mod;
    sema::SymbolTable                          &symTab;
    IRBuilder                                   builder;

    std::vector<FunctionContext>                fnStack;  // back() is the function being emitted
    FunctionContext &ctx() { return fnStack.back(); }

    // Globals are found by symbol, independent of the current function:
    // file-scope variables, static data members, static locals, functions.
    std::unordered_map<const sema::Symbol *, Value *> globalValues;
    unsigned                                    staticLocalCounter = 0;  // module-wide, keeps @fn.name.N unique

    // Expressions are NOT lowered through visit()'s return value alone. Two entry points:
    Value *emitLValue(ast::Expr *e);   // address (ptr) of an lvalue expression
    Value *emitRValue(ast::Expr *e);   // value; for lvalues = load(emitLValue(e)) unless array/function/record-by-address
    Value *emitCondition(ast::Expr *e);// i1; e is already ImplicitCast(ToBool) or bool-typed
    // Statements are visited with visit(); their return value is ignored.
};
```

#### Step-by-Step Pass Order in `IRVisitor::generate(ast::Program *program)`:
1. **Pass 1 — Register Record Types & Static Data Members**:
   - Recursively walk `symTab.global()` **and every record's `members` scope** (nested records such as `Outer::Inner` in `test4` live there, not in the global scope) to collect all `sema::RecordSymbol`s (`mod.registerRecord(rec)`). Named struct types may refer to each other in any order in LLVM, so the emission order does not matter for validity. Declaration order just keeps the output stable.
   - Records named `Outer.Inner` print as `%struct.Outer.Inner`, which is a valid identifier because `.` is allowed. A record with no bases and no fields prints as `type {}`.
   - For each `RecordSymbol`, inspect `rec->members->symbols`: if a symbol has `SymbolKind::Variable` (`isStatic == true`), pre-create its `GlobalVariable` (external linkage, `global`) with default zero initialization. An out-of-class `VarDecl` in `program->decls` updates the initializer if present; that `VarDecl` has a qualified name (`Counter::total`) and `node->symbol` **is** the member symbol.
2. **Pass 2 — Register Global Variables & Function Declarations**:
   - Walk `program->decls`. For each file-scope `VarDecl`, create or update the `GlobalVariable`:
     - Linkage is `internal global` when `symbol->isStatic` and `global` otherwise.
     - With no initializer, use `zeroinitializer` (or `0` / `null` for scalars). Unlike locals, globals are zero-initialized in C++.
     - Fold the initializer to a constant if it is one (§5.8 lists what counts). Otherwise keep `zeroinitializer` and append the store or constructor call to `@__global_init`.
   - Create a `Function` (a declaration for now) for **every** `FunctionSymbol` in the scope tree before emitting any body, keyed by `mangledName`. A call to a function defined later in the file, or one that is never defined, then resolves. Functions that are never defined stay `declare`. Exception: constructors and destructors with no definition are synthesized in Pass 3.
   - `@printf`/`@scanf` are created from their `FunctionSymbol` (`isBuiltin`, variadic). Emit `declare` for them only if used. `@malloc`/`@free` come from `Module::getRuntimeMalloc/Free()` (§1.2).
3. **Pass 3 — Emit Function, Method, Constructor, and Destructor Bodies**:
   - Walk `program->decls` and, recursively, `RecordDecl::members` (in-class definitions, including nested records). Emit a body for a `FunctionDecl` **iff** `node->symbol && node->symbol->definition == node`. A function can be declared several times (`static int helper(int x);` and then its definition in `test17`), and an out-of-class definition `Dog::bark() {...}` is a top-level `FunctionDecl` with a qualified name, so this rule is the only reliable test.
   - Linkage: `define internal` when `fn->isStatic && fn->parentRecord == nullptr` (a file-scope `static` function). A static **member** function keeps external linkage. `main` is always `define i32 @main(...)` (its `mangledName` is already `"main"`).
   - Synthesize constructors and destructors (§5.10):
     - user-declared ones that have no definition (`Animal()`, `Animal(int)`, `Animal(int,int)`, `~Animal()` in `test4`) get a body that does only the base/member initialization (or destruction);
     - implicit ones are created for records that need them (`Dog`'s bases, `Registered`, `Cat`, `Tagged`, `Counter` with `int start{0}`).
4. **Finalize**: run `Function::finalizeCFG` on every defined function (§5.4). If `@__global_init` has a body, terminate it with `ret void` and emit `@llvm.global_ctors`.

---

## 3. Updates to `src/parser.y`, `Makefile`, and `run.sh`

### 3.1 [MODIFY] [`src/parser.y`](../src/parser.y#L1145-L1187)
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
    ir::IRVisitor irGen(irMod, table, diags);
    irGen.generate(g_program.get());

    // IR generation can still hit something unsupported (§5.12); it reports through
    // the same Diagnostics instead of crashing or writing a half-built module.
    if (!diags.empty()) {
        diags.print(cout, sourcePath);
        return false;
    }

    std::ofstream irFile = openOutput(sourcePath, ".ll");
    if (irFile) irMod.dump(irFile);

    return true;
```
Before the new layout writes into `outputs/<stem>/`, `run.sh` should `rm -rf outputs/<stem>`. Otherwise a stale `<stem>.ll` from an earlier passing run survives after a test starts failing, and the "rejected tests emit no `.ll`" check passes for the wrong reason.

### 3.2 [MODIFY] [`Makefile`](../Makefile#L33-L34)
Add `src/ir/*.cpp` and `src/ir/*.h` to `AST_SRCS` and `AST_HDRS`:
```makefile
AST_SRCS := $(wildcard $(SRC)/ast/*.cpp) $(wildcard $(SRC)/semantic/*.cpp) $(wildcard $(SRC)/ir/*.cpp)
AST_HDRS := $(wildcard $(SRC)/ast/*.h) $(wildcard $(SRC)/semantic/*.h) $(wildcard $(SRC)/ir/*.h) $(SRC)/declarator.h
```

### 3.3 [MODIFY] [`run.sh`](../run.sh)
Rewrite `run.sh` so that for each `test/<base>.ckp`:
1. Creates subfolder `outputs/<base>/`.
2. Copies `test/<base>.ckp` into `outputs/<base>/<base>.ckp` for reference.
3. Runs `"$EXE" "$f" > "outputs/$base/$base.txt" 2>&1`.
4. If the compiler succeeds (exit code 0, meaning Lexer + Parser + SymbolBuilder + TypeChecker + IRGenerator all succeeded):
   - Verifies that `outputs/$base/$base.ll` was generated.
   - Runs
     ```bash
     clang -x ir -c -fverify-intermediate-code -Wno-override-module "outputs/$base/$base.ll" -o /dev/null
     ```
     to check that the `.ll` file is syntactically and semantically valid LLVM IR.
   - Reports `PARSED & IR VALID` (or `IR INVALID` if `clang` rejects the `.ll`).

   > **Why not `clang -x ir -fsyntax-only`** (the command in the earlier revision)? It cannot work: clang refuses every IR input with `fatal error: cannot apply AST actions to LLVM IR file`, so every test would be reported `IR INVALID`.
   >
   > **Why `-fverify-intermediate-code`?** Release builds of clang, Apple clang included, skip the IR verifier by default. A plain `clang -c` accepted a function that used `%z` before defining it. With the flag, clang reports `Instruction does not dominate all uses!` (tested with Apple clang 17.0.0).
   >
   > Two things the verifier does **not** catch:
   > - Integer constants out of range for their type are silently truncated: `i1 4` becomes `false` and `i32 2147483648` wraps. That is why §5.1 normalizes every constant.
   > - Uses inside unreachable blocks are never dominance-checked.
   >
   > Optional stronger check: `clang outputs/$base/$base.ll -o outputs/$base/$base.out` links the program, which also catches undefined functions or static members. It needs input on stdin for `scanf` tests if run.

---

## 4. Verification & Test Suite Matrix

Downstream agents implementing this specification must verify the following criteria:

1. **Zero Build Warnings**:
   ```bash
   make clean && make all
   ```
   Must compile cleanly with `-Wall -std=c++17`.
2. **All 27 Test Files in `bash run.sh`** (the 22 original ones, plus tests 23–27 below):
   - **9 Accepted Tests (`test1`, `test2`, `test3`, `test4`, `test5`, `test7`, `test12`, `test17`, `test22`)**:
     Must pass all compiler phases, populate `outputs/<name>/` with `<name>.ckp`, `<name>.txt`, `<name>-ast.txt`, `<name>-symbols.txt`, and `<name>.ll`, and pass the §3.3 `clang -x ir -c -fverify-intermediate-code` check with 0 errors.
   - **13 Rejected Error Tests (`test6`, `test8`, `test9`, `test10`, `test11`, `test13`, `test14`, `test15`, `test16`, `test18`, `test19`, `test20`, `test21`)**:
     Must be rejected with the same error count as before the IR phase existed. Record the counts from the current `run.sh` output before starting, because IR generation must not change them. They must **not** emit a `.ll` file.
3. **Run tests for the supported features** *(as agreed: replaces the earlier idea of one edge-case file covering every §5 rule)*. Tests 23–27 each cover one group of the README's "Supported syntax" list, using only plain forms of those features. `run.sh` links and runs each one and compares its output with `test/expected/<name>.out`, which was produced by compiling the same source with clang++:
   - `test23_ir_expressions`: every operator group, compound assignment (§1.3), increment and decrement
   - `test24_ir_control_flow`: `if`/`else` with returns in every branch (§5.4), `switch`, loops, `goto`
   - `test25_ir_arrays_pointers`: arrays, pointers, references (§5.2), brace initialisers (§5.13), `new`/`delete`
   - `test26_ir_classes`: inheritance, constructor and destructor order, scope-exit destruction (§5.7, §5.10)
   - `test27_ir_statics_lambdas`: static storage (§5.8), lambdas (§5.9)

---

## 5. Edge-Case Rulebook (Read Before Writing `ir_visitor.cpp`)

Each rule below was checked against what `src/semantic/typecheck.cpp` actually leaves in the AST, and the IR snippets were checked with `clang -x ir -c -fverify-intermediate-code` (Apple clang 17). Where the conceptual guide (`ir-guide.md`) disagrees, this section wins.

### 5.1 Integer constants must be normalized to their LLVM type

`sema::evaluateConstant` walks **through** `ImplicitCastExpr` without applying it, and computes in `long long`. So the folded value is in the type of the *innermost* literal, not of the destination:

| Source | `evaluateConstant` gives | Naive IR | Correct IR |
| :--- | :--- | :--- | :--- |
| `bool b = 4;` | `4` | `i1 4` → **silently `false`** | `i1 1` (ToBool: `v != 0`) |
| `char c = 300;` | `300` | `i8 300` (wraps silently) | `i8 44` |
| `int x = 1 << 31;` | `2147483648` | `i32 2147483648` | `i32 -2147483648` |
| `float f = 2;` (global) | `2` | `float 2` → **parse error** | `float 0x4000000000000000` |
| `double d = 'a';` | `97` | `double 97` → **parse error** | `double 0x4058400000000000` |

Rules:
- Fold a constant initializer **by looking at the outermost `ImplicitCastExpr` chain yourself**. Get the integer value with `evaluateConstant(sub)`, then apply each cast kind: `ToBool` → `v != 0`; `IntegralCast` to `char` → `(int8_t)v`; `IntegralToFloating` → `(double)v`, then `(float)` if the target is `float`.
- `normalizeInt(v, type)`: `i1` → `v != 0`, `i8` → `(int8_t)v`, `i32` → `(int32_t)v`, `i64` → `v`. **Every** `Constant::intVal` goes through it, including `CaseStmt::constant` (the switch condition is always promoted to `int`, so case values are `i32`).
- `FloatLiteral` is never handled by `evaluateConstant`; fold it separately (§2.1 item 2). A float initializer that mixes a literal with arithmetic (`double d = 1.5 * 2;`) can either be folded with a small FP folder or go to dynamic init (§5.8). Both are correct.
- `CharLiteral` values come from `sema::charLiteralValue`, so `'\xFF'` is `255` and must print as `i8 -1`.

### 5.2 References: sema adds **no cast node** when a reference binds

`findConversion` returns zero steps for an exact reference binding (the only step it can add is `DerivedToBase`), and `convert()` just marks the expression `isLValue = true`. The AST therefore looks the same for `int y = x;` and `int &y = x;`. **The IR must look at the destination type** and evaluate the source with `emitLValue` whenever that type is a `ReferenceType`:

| Construct | Destination type to inspect | Lowering |
| :--- | :--- | :--- |
| `int &r = x;` / `int (&a)[2][3] = grid;` (test2, test7, test22) | `VarDecl::symbol->type` | `%r.addr = alloca ptr`; `store ptr <emitLValue(init)>, ptr %r.addr` |
| `swap(left, right)` with `int &` params | `fnType->params[i]` (use `loweredType` for methods, offset by 1 for `this`) | pass `emitLValue(arg)` |
| ~~`int &get() { return x; }`~~ | *not implemented: the grammar cannot declare a function returning a reference (change log §7 #11)* | – |
| a by-copy lambda capture of a reference variable | `stripReference(capture.symbol->type)` | copy the **referee value**; sema's closure layout (`toLLVMType(Closure)`) already uses `stripReference` |
| a by-ref lambda capture of a reference variable | – | store the referee's address (`load ptr, ptr %r.addr`), not `%r.addr` |

The general `DeclRefExpr` rule: when `symbol->type` is a `ReferenceType`, `emitLValue` = `load ptr, ptr <slot>`; otherwise `emitLValue` = `<slot>`. Reference **members** are rejected by `SymbolBuilder`, so fields are never references.

### 5.3 Local names: one namespace per function, uniquified

In LLVM, arguments, instruction results **and block labels** share one namespace per function. With the naming scheme in the guide, all of these produce duplicate definitions:
- two locals called `x` in sibling or nested blocks → two `%x.addr`
- a parameter called `t0` → collides with the temporary `%t0`
- a parameter or local called `entry` → collides with the `entry:` label
- two `if` statements → two `if.then` labels unless every label gets a counter

Rule: every name goes through `Function::uniqueName(base)` (§2.3), including temporaries (`"t" + N`, still checked) and labels (`if.then`, `for.cond`, ...). Reserve `"entry"` before anything else is named. An **unnamed parameter** (`ParamDecl::name.empty()`) gets `uniqueName("arg" + index)`. Never use purely numeric names (`%0`): LLVM requires those to be sequential.

The `entry` block must have **no predecessors**. Never branch back to it: a `LabelStmt` always starts a fresh block, even when it is the first statement of the function.

### 5.4 Dead code and CFG finalization

The original rule ("remove empty non-entry blocks with no predecessors; add a fallback terminator to the final block if reachable") is not enough. Take `classify` in `test1`, an `if / else if / else` where every branch returns:
1. The inner `if` leaves `if.end1` empty and with no predecessors, and the builder's insertion point is now `if.end1`.
2. The outer `if` then appends `br label %if.end0` **to `if.end1`**, which is not terminated, so the guard does not stop it.
3. Now `if.end1` is non-empty and `if.end0` has a predecessor. Neither is removed by the "empty and pred-less" rule. Both are really unreachable.
4. Any **non-final** block left without a terminator (for example a `LabelStmt` block only reached by dead code) would be invalid IR. The old rule only patched the final block.

Rule (implemented in `finalizeCFG`):
1. Walk from `entry` over `successors` (DFS). Delete every block that is not reached.
2. Give every remaining block that lacks a terminator one:
   - for `void` functions, constructors and destructors: `ret void`
   - for `main`: `ret i32 0`
   - for any other non-void function: return a zero value of the return type (`0`, `0x0` as FP hex, `null`, or `zeroinitializer` for records). Emitting `unreachable` would also be valid, but the default return is friendlier for the test programs.
3. Use `unreachable` for a block whose only purpose was a dead join.

Deleting unreachable blocks is always safe, because a reachable block cannot branch into an unreachable one. Dominance is not an issue either: allocas sit in `entry`, which dominates everything reachable, and every other value is a temporary used in the block that defines it. The exceptions are the `&&`/`||`/`?:` temporaries, and those go through allocas (§5.5).

### 5.5 Expression lowering details that the guide leaves open

| Construct | What sema gives | Lowering |
| :--- | :--- | :--- |
| `ParenExpr` | same `type`/`isLValue` as `sub` | transparent in both `emitLValue` and `emitRValue` |
| `-x` | operand already promoted to `int`/`float`/`double` | `sub i32 0, %x` / `fneg double %x` |
| `~x` | operand promoted to `int` | `xor i32 %x, -1` |
| `!x` | operand wrapped in `ToBool` | `xor i1 %c, true` |
| `+x` | promoted operand | the operand itself |
| `p + n`, `n + p`, `p - n` | `n` converted to `int` | `getelementptr inbounds <pointee>, ptr %p, i32 %n`; for `-`, negate `n` first (`sub i32 0, %n`) |
| `p - q` (test22 `third - first`) | result `int`, same pointee type | `ptrtoint` both to `i64`, `sub`, `sdiv exact` by `sizeof(pointee)` (GEP-null trick, §9 of the guide), `trunc` to `i32` |
| `p < q` etc. on pointers | same type after sema's conversion | `icmp ult/ugt/ule/uge ptr` (`eq`/`ne` for `==`/`!=`). `null` operands are already `NullToPointer`-cast or typed `null` |
| `/`, `%`, `>>` | `int` operands | `sdiv`, `srem`, `ashr` (no unsigned types exist) |
| `c ? a : b` where **`node->isLValue`** (both arms lvalues of the same type; test1/test7 `(a > b) ? a : b`) | arms are *not* decayed or converted | branch, and store the **address** `emitLValue(arm)` into an entry `alloca ptr`; the result address is `load ptr` from it. Do not use `select`: it would evaluate both arms. |
| `c ? a : b` rvalue | arms converted to the common type | branch + entry `alloca <T>`, as with `&&` in guide §7.3. With a `void` result there is no slot. |
| `a = b` | `isLValue = true`, `value` already converted | `store`; `emitLValue` returns the target address, `emitRValue` returns the stored value |
| `a op= b` | see §1.3 | load, convert to common type, op, convert back, store; pointer `+=`/`-=` use GEP |
| `x++` / `++x` | `++x` is an lvalue, `x++` is not | as in guide §11.3; `++x` must support `emitLValue` (returns `%addr`) |
| record value copy (`Point q = p;`, `p = q;`, record args/returns) | no copy-ctor calls exist for records without ctors | `load %struct.Point, ptr %src` + `store`; first-class aggregates are legal in `call`/`ret`/`define` |
| record **rvalue** that needs an address (`make().x`, calling a method on a returned object, `DerivedToBase` on a returned object) | – | **materialize**: entry `alloca` temp, `store` the value, use the temp's address |
| variadic args (`printf`) | sema already promoted: `float`→`double`, `bool`/`char`→`int`; arrays decayed | print the call with the full signature: `call i32 (ptr, ...) @printf(ptr @.str.N, ...)` |

**Calls**: decide the callee strictly in this order:
1. **`CallExpr::function != nullptr`** — a direct call to `@mangledName` with `function->loweredType`.
   - If `function->isMethod()` (not merely `parentRecord != nullptr`, which is also true for static methods), prepend `this`:
     - callee is a `MemberExpr` with `.` → `emitLValue(base)`, materialized if the base is an rvalue;
     - callee is a `MemberExpr` with `->` → `emitRValue(base)`;
     - callee is a bare `DeclRefExpr` inside a method or a `[this]` lambda → `ctx().thisPtr`.
   - Then adjust `this` from the object's record to `function->parentRecord` along the base path (§5.6). An inherited call such as `d.setAge(5)` needs this.
   - A static method called through an object (`c.next()` in test17) gets **no** `this`. Evaluate the base anyway if it has side effects.
2. **Callee type is `ClosureType`** → `call @__lambda_L_C(ptr <emitLValue(callee)>, args...)`, with the return type from `closure->signature`.
3. **Callee type is `FunctionType`** (an lvalue such as `(*fpp)(2, 3)` in test22) → the function pointer is `emitLValue(callee)`, and `emitLValue(*p)` = `emitRValue(p)`.
4. **Callee type is `PointerType` to `FunctionType`** (`fp(4, 5)`, `binaryOp(left, right)`) → `emitRValue(callee)`; print `call i32 %tN(i32 ..., i32 ...)`.

**Names that resolve to members** (`DeclRefExpr` without `this->`, e.g. `secretId + legs` in `Animal::hidden`, `total++` in `Counter::next`):
- `SymbolKind::Field` → GEP from `ctx().thisPtr`, after a `DerivedToBase` adjustment when `symbol->parentRecord != ctx().thisRecord`.
- `SymbolKind::Variable` with `parentRecord` (a static member) → `globalValues[symbol]`. Never GEP.
- `MemberExpr` whose `symbol` is a static member (`obj.total`) → also the global.
- `DeclRefExpr` naming a `FunctionSymbol` (as the operand of `FunctionToPointer`) → `emitLValue` is the `Function` itself (`@mangled`).

### 5.6 `DerivedToBase` has three shapes

`findConversion` produces `DerivedToBase` for (a) pointers, (b) reference binding (cast type = the base *record*, `isLValue = true`), and (c) **by-value slicing** (`Base b = derived;`, cast type = base record, rvalue). The base may be indirect (`isDerivedFrom` searches all depths), so compute the full path by DFS over `bases`. Sema has already rejected ambiguous member lookups. Repeated bases through two paths are not detected by sema; take the first path found.

- **(b) lvalue** → `emitLValue(cast)` = a chain of `getelementptr inbounds %Derived, ptr %obj, i32 0, i32 <baseIdx>`.
- **(c) slicing** → `emitRValue(cast)` = `load %Base, ptr <that chain>`. If the derived operand is an rvalue, materialize it first (§5.5).
- **(a) pointer** → the same GEP chain, **but if any index on the path is non-zero** (the pointer is converted to a non-first base, e.g. `Trackable *t = &registered;` in test4), a null pointer must stay null. `getelementptr inbounds` on `null` with a non-zero offset is poison. Emit:
  ```llvm
  %isnull = icmp eq ptr %p, null
  %adj    = getelementptr inbounds %class.Registered, ptr %p, i32 0, i32 1
  %res    = select i1 %isnull, ptr null, ptr %adj
  ```
  If all indices are 0, the GEP alone is fine. This mirrors clang.

### 5.7 Cleanups (destructors) on every exit path

`ReturnStmt` order matters. Running destructors *before* evaluating the return value is wrong for `return d.age;` when `d` has a destructor (`main` in test4 has live `Dog d` and `Cat c` at `return 0;`). Rule:
1. Evaluate the return value (`emitRValue`, or `emitLValue` for reference returns).
2. If any cleanup scope is active, `store` the value into `ctx().retvalSlot` (an entry alloca created lazily).
3. Emit destructor calls for **all** scopes, innermost first, LIFO within each scope.
4. `ret` the value (reload it from `retvalSlot` if one was used).

`break` / `continue` run cleanups for the scopes from `scopeCleanupStack.size() - 1` down to the target's `cleanupDepth`, **without popping**: sibling code after the jump still needs the stack. The end of a `CompoundStmt`, and of a `ForStmt` (its init declarations live in the `for` scope), pops normally and emits destructors only if the current block is not terminated.

`goto` (test1 `top:`, test7 `retry:`): before the IR pass, map every `LabelStmt` to the chain of enclosing `CompoundStmt`/`ForStmt` scopes. On a `goto`, emit cleanups for every scope on the current chain that is **not** on the target's chain (everything below the common prefix). In C++, jumping *into* a scope past an object's initialization is ill-formed. Sema does not check this, so IR generation reports it as unsupported (§5.12) if the target chain contains a scope with a pending object initialization.

Only objects whose record **needs destruction** are pushed: the record has a user `destructor`, or any base or field (recursively, arrays included) needs destruction. Memoize this per record.

### 5.8 Global and static-local initialization

What counts as a constant initializer for a global (anything else is dynamic):
- integer, char and bool constants via §5.1, and FP literals (with casts)
- `null` / `NullToPointer`
- a string literal used as a pointer (`ArrayToPointer` of a `StringLiteral`) → `ptr @.str.N`
- `&g` / an `ArrayToPointer` of a global array / a `FunctionToPointer` of a function → `ptr @g`. The plain global address is valid because its first element sits at offset 0.
- a string literal initializing a `char` array → `c"...\00"`, padded with `\00` up to the array size
- brace lists of any of the above → `[ ... ]` / `{ ... }`. **Missing trailing elements are zero** (`int a[4]{1}` → `[i32 1, i32 0, i32 0, i32 0]`), and an empty list is `zeroinitializer` (`static int seen[4]{}` in test17).

**Dynamic** file-scope or static-member initializers, and objects whose constructor or implicit constructor must run, are lowered into `@__global_init` (registered through `@llvm.global_ctors`, §2.3) in declaration order. Global objects with a destructor are destroyed in reverse order by `@__global_fini`. *(As built: `@__global_init` registers it with `call i32 @atexit(ptr @__global_fini)`, as clang does. The `@llvm.global_dtors` route was tried and dropped: on macOS it becomes a `__mod_term_func` section that the loader no longer runs, so the destructors never ran.)*

**Static locals** (`static int calls = 0;` in test17 `main`):
- Name: `@<enclosing mangled name>.<name>.<staticLocalCounter++>`, using the lambda's `__lambda_L_C` when inside a lambda. The linkage is `internal global`.
- A constant initializer is folded into the global, as above, and runs exactly once.
- A **non-constant** initializer, or a record with a constructor, needs a guard so it runs once and on first execution only:
  ```llvm
  @main.x.1.guard = internal global i1 false
  ...
    %g = load i1, ptr @main.x.1.guard
    br i1 %g, label %static.done, label %static.init
  static.init:
    ; evaluate initializer, store into @main.x.1
    store i1 true, ptr @main.x.1.guard
    br label %static.done
  static.done:
  ```
- Static locals are never added to `scopeCleanupStack`.

### 5.9 Lambdas inside half-built functions

- Lower a `LambdaExpr` in place: `saveIP()`, push a new `FunctionContext`, emit `define internal <ret> @__lambda_<line>_<col>(ptr %__closure, <params>)` completely, pop, then `restoreIP()`. Only after that, build the closure object in the outer function. **Every** per-function state lives in `FunctionContext` (§2.5): locals, labels, loop/switch stacks, cleanups, `this`, and the return type. A lambda's `return` must use the lambda's deduced `closure->signature->returnType`, not the outer function's.
- The closure struct layout must exactly match sema's `toLLVMType(Closure)`, field `i` = `usedCaptures[i]`:
  - `ByCopy` of variable `v` → `formatType(stripReference(v->type))`
  - `ByRef` → `ptr`
  - `This` → `ptr`, with `symbol == nullptr`
  
  A lambda with no captures gets `%closure.L.C = type {}`. It still has a closure parameter, and the closure object is an `alloca` of the empty type.
- **Nested lambdas**: `captureVariable` appends a capture to **every** lambda between the use and the variable's declaration. When the inner closure is built *inside the outer lambda's body*, `ctx().namedValues[symbol]` already points at the outer closure's field (`ByCopy`) or the loaded pointer (`ByRef`), so the generic rule "copy from / take the address of `namedValues[symbol]`" composes correctly. It works only because `namedValues` is per function.
- **Captured parameters**: `isLocal` treats `SymbolKind::Parameter` as capturable. `namedValues` holds the parameter's `.addr` alloca, so parameters need no special case.
- `this` inside a `[this]` / `[=]` / `[&]` lambda: `ctx().thisPtr` = `load ptr` from the `This` capture field, and `ctx().thisRecord` = the outer method's record. Sema adds `This` captures implicitly for `[=]`/`[&]`.
- Inside the lambda, `ByCopy` captures map `namedValues[sym]` to the GEP into `%__closure` (writes then go to the closure copy, matching a `mutable` lambda; sema does not reject writes). `ByRef` captures map to `load ptr` from the field.
- Copying a closure object (`auto g = f;`) is an aggregate load/store. Calling it uses the address of `g`.

### 5.10 Constructors and destructors: synthesis rules and names

Definitions:
- **needsConstruction(R)**: `R->constructors` is non-empty, or any base needs construction, or any field has an in-class initializer (`VarDecl::init` / `ctorArgs` on the field's `decl`), or any field's record (or array element record) needs construction.
- **needsDestruction(R)**: as in §5.7.

Which constructor runs:
- `VarDecl::constructor` / `NewExpr::constructor` / `InitListExpr::constructor` is the user constructor if one was selected.
- If it is `nullptr` and the record has **no** user constructors:
  - with 0 args → the implicit default constructor, only if `needsConstruction(R)`; otherwise nothing runs and the memory is left uninitialized, like clang;
  - with 1 arg → an aggregate copy (§5.5).

Naming of synthesized functions:

| What | Name | When |
| :--- | :--- | :--- |
| body for a declared-but-undefined ctor/dtor | its existing `FunctionSymbol::mangledName` (e.g. `class_6Animal_C_i`) | `fn->definition == nullptr` |
| implicit default constructor | `class_<recordPath>_C_v` (same scheme as `SymbolBuilder::defineMethod`) | `R->constructors.empty() && needsConstruction(R)` |
| implicit destructor | `class_<recordPath>_D_v` | `R->destructor == nullptr && needsDestruction(R)` |

There is no collision: the implicit name is only used when no user constructor or destructor exists. `recordPath` length-prefixes each level (`5Outer5Inner`); reproduce it in `src/ir` or expose it from `symbolbuilder.cpp`.

Every constructor body, user-written or synthesized, starts with:
1. a default constructor call for each base in order (GEP to base `i`); if the base has user constructors but no default constructor (an overload with zero params), report an error (§5.12);
2. for each field in order: its in-class initializer if any (test17 `int start{0};`), else the default constructor if its record needs construction, else nothing. Array fields of records loop over every element (§5.11 loop shape).

Every destructor body ends with field destructors in reverse order, then base destructors in reverse order.

`this` inside these bodies is `%this` spilled to `%this.addr` and loaded, exactly as for methods. In-class initializer expressions are evaluated with `ctx().thisPtr` set, because they may name other fields.

**Arrays of records** (`Dog dogs[3];`, multi-dimensional too): emit a loop over the flattened element count. `VarDecl::constructor` is the element constructor; sema unwraps nested arrays for this.
```llvm
  ; i.addr = alloca i64 (entry)
  store i64 0, ptr %i.addr
  br label %arrinit.cond
arrinit.cond:
  %i = load i64, ptr %i.addr
  %done = icmp eq i64 %i, <N>
  br i1 %done, label %arrinit.end, label %arrinit.body
arrinit.body:
  %el = getelementptr inbounds %class.Dog, ptr %arr, i64 %i
  call void @class_3Dog_C_v(ptr %el)
  %next = add i64 %i, 1
  store i64 %next, ptr %i.addr
  br label %arrinit.cond
arrinit.end:
```
Destruction loops run from `N-1` down to `0`.

### 5.11 `new` / `delete` forms

| Form (`NewForm`) | Lowering after `malloc(sizeof)` |
| :--- | :--- |
| `Plain` — `new int`, `new Bar` | scalar: uninitialized; record: `node->constructor` or the implicit default constructor (§5.10) |
| `Args` — `new int(5)`, `new int()`, `new Animal(3)` | scalar with 1 arg: store it; scalar with **0 args: store zero** (value-initialization); record: constructor with args |
| `InitList` — `new Point{5, 6}` (test17) | if `constructor`, call it with the list; else aggregate init, zero-filling missing fields |
| `Array` — `new int[n]`, `new int*[4]`, `new int[2][3]` | count `dims[0]` → `sext` to `i64`; size = GEP-null over `ElemTy` with that count; record elements → constructor loop (§5.10) with a runtime bound |
| `ArrayInit` — `new int[3]{7,8,9}`, `new int[2][3]{{...},{...}}` (test17) | element-wise stores; **elements beyond the list are zero**, or default-constructed for records; nested lists recurse into the inner `[3 x i32]` |

`delete p` / `delete[] p`: a `null` operand is a no-op in C++. Calling `free(null)` is fine, but a destructor call on `null` is not, so wrap destructor calls in `icmp eq ptr %p, null` → skip.

**`delete[]` of records with destructors needs the element count**, and nothing stores it. Choose one and document it in the code:
- **(recommended)** an array cookie, as clang does. `new R[n]` with `needsDestruction(R)` allocates `8 + n*sizeof(R)`, stores `n` as an `i64` at offset 0, and returns `raw + 8`. `delete[] p` reads the count from `p - 8`, runs destructors in reverse, and then frees `p - 8`. Arrays of trivially-destructible types get no cookie.
- or report `delete[]` on such records as unsupported (§5.12).

`delete` (non-array) applied to memory from `new[]`, as in `test3` (`delete buffer;`) and `test7` (`delete heapData;`), is undefined behaviour in C++. For `int` it lowers to the same `free`, so those tests stay valid. Do not "fix" them.

### 5.12 Things IR generation must refuse instead of miscompiling

`IRVisitor` takes `sema::Diagnostics &` (§3.1) and reports with the node's `line`/`col` instead of asserting:
- a base or field record with user constructors but **no default constructor**, when an implicit or synthesized constructor needs it
- `goto` that jumps into a scope past an object that needs construction (§5.7)
- ~~global objects that need destruction~~ (implemented through `atexit`, §5.8)
- `delete[]` of records needing destruction, if the cookie is not implemented (§5.11)
- temporaries of records needing destruction that are discarded (`make();` returning a `Dog` by value). Without temporary-lifetime tracking their destructor never runs. Either accept this as a documented leak or report it.

None of these occur in the 9 accepted tests. They exist so new tests fail loudly.

### 5.13 Local aggregate initialization

- `int arr[3]{1, 2, 3};`, `int grid[2][3]{{...}}`, `Point p{1, 2};`, `Point q = {3, 4};` (test17): when the list is not complete (fewer elements than the array size or field count, or `{}`), first `store <T> zeroinitializer, ptr %x.addr`, then store each provided element through a GEP. When every element is a constant, a single `store <T> [ ... ]` of the aggregate constant is equivalent.
- `char s[10] = "hi";` → store the bytes and pad with `\0` up to 10. Equivalent: `store [10 x i8] c"hi\00\00..."`.
- `int b{};` → `store i32 0`. `int b;` → no store.
- An `InitListExpr` with `constructor != nullptr` is a constructor call with the list's elements as arguments, not an aggregate.
- `auto` variables: sema rewrote `symbol->type` to the deduced type, so nothing special is needed. A lambda `auto f = [..](..){..};` is a `ClosureType` slot initialized by §5.9.

### 5.14 `switch` details

- The condition is already promoted to `int` (sema `visitSwitchStmt`), so every case value is `i32 normalizeInt(constant)`.
- Statements before the first `case` are unreachable. The block after `switch` is terminated, so the guard suppresses them, but `alloca`s for declarations there are still hoisted, which is harmless.
- `default` may appear anywhere, including first or in the middle. `SwitchInst::defaultDest` starts as `switch.end` and is patched when a `DefaultStmt` is visited (`SwitchContext::sawDefault`). Fall-through into `default` works like fall-through into a `case`.
- `CaseStmt`/`DefaultStmt` nested inside inner `{ }` blocks of the switch body attach to `ctx().switchStack.back()`. A `switch` nested inside a case pushes its own context, so inner cases never leak outward.
