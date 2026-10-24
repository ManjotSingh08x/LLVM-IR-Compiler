# IR Phase Change Log

This is the running record of every change made to the project while adding the
CFG and LLVM IR phase. Each entry says what was changed, where, and why, in plain
English. New entries go at the bottom of the relevant section with the date.

Related documents:
- [`ir-implementation-plan.md`](ir-implementation-plan.md): the agreed plan. Section 5 is the edge-case rulebook.
- [`ir-guide.md`](ir-guide.md): the conceptual guide.

---

## 1. Grammar changes

**None.** `src/lexer.l` is unchanged, and no grammar rule in `src/parser.y` was added, removed or edited. The compiler accepts exactly the same language as before. That language is listed under "Supported syntax" in the README, and the 22 original tests exercise it.

The only edits to `src/parser.y` are in the C++ driver code after the second `%%` (see section 3).

Several test programs written during this work hit syntax the grammar does not accept. In every case **the test was rewritten to stay inside the grammar**. The grammar was not extended. For the record, these forms are rejected and stay rejected:

| Form | Example | What the tests use instead |
| :--- | :--- | :--- |
| Constructor arguments with `()` on a file-scope object | `Logger g(1);` | not used in tests |
| Calling a static method through the class name | `Registry::bump()` | `obj.bump()` |
| A function returning a reference | `int &larger(int &a, int &b)` | not used |
| Explicit casts | `(int)x` | an implicit conversion through a typed variable |
| Arrays of function pointers | `int (*ops[2])(int, int)` | not used |
| A label directly in front of a declaration | `again: Tracer t(1);` | a statement between them (C++17 rejects this too) |

---

## 2. Semantic phase changes (`src/semantic/`)

### 2026-10-24: `toLLVMType` prints a record by name (plan §1.1)
- **File**: `src/semantic/types.cpp`, `toLLVMType`, `case TypeKind::Record`.
- **What**: a record type now prints as `%struct.Point` instead of its full layout `%struct.Point = type { i32, i32 }`.
- **Why**: when a record sits inside another type, the old output was not valid LLVM, e.g. `[4 x %struct.Point = type { i32, i32 }]`. The full layout is still printed once per record by `toLLVMRecord`. This only affects the `-symbols.txt` debug dump; the IR phase has its own formatter.

### 2026-10-24: `decodeStringLiteral` added (plan §1.4)
- **Files**: `src/semantic/constants.h`, `src/semantic/constants.cpp`.
- **What**: a new function returns the characters of a string literal with escapes decoded. It uses the same `readChar` loop that `stringLiteralLength` uses.
- **Why**: sema sizes a string literal as `[length + 1 x i8]`. If the IR phase decoded escapes (`\x41`, `\0`, `\"`) with separate code, the two could disagree and produce a global whose size does not match its contents. One decoder means one answer.

### 2026-10-24: compound assignment converts to the common type (plan §1.3)
- **File**: `src/semantic/typecheck.cpp`, `visitAssignExpr`, the arithmetic branch for `+= -= *= /= %=`.
- **What**: the right-hand side is now converted to `commonArithmeticType(promote(target), promote(value))` instead of to the target's type.
- **Why**: converting first gave wrong answers. `int x = 7; x *= 2.5;` gave 14 instead of 17. `bool b = true; b += 1;` gave `false`. `char c = 100; c += 300;` truncated 300 before adding. C++ computes `x = x op v` in the common type and converts only the result back. The IR now does that, and the results match clang++ (checked by `test23_ir_expressions`).
- **Effect on errors**: none. Both sides are already arithmetic at that point, so the new conversion cannot fail. All 13 rejected tests report the same error counts as before.

---

## 3. Driver changes (`src/parser.y`, C++ code only)

### 2026-10-24: one output folder per source file (plan §3.1)
- **What**: `openOutput` writes to `outputs/<stem>/<stem><suffix>` instead of `outputs/<stem><suffix>`.
- **Why**: each test now has five or six files: the source copy, the printed output, the AST, the symbols, the `.ll`, and the program output. A folder per test keeps them together, as the plan specifies.

### 2026-10-24: IR generation after semantic analysis (plan §3.1)
- **What**: when semantic analysis reports no errors, `analyze` builds an `ir::Module`, runs `ir::IRVisitor::generate`, and writes `outputs/<stem>/<stem>.ll`. If the IR phase reports an error (see plan §5.12), it is printed like a semantic error, the program exits non-zero, and no `.ll` is written. A successful run now also prints `IR generation successful.`.
- **Why**: this is the new compiler phase. IR errors go through the existing `Diagnostics` object so all errors look the same and are counted the same way by `run.sh`.

---

## 4. Build and test runner

### 2026-10-24: Makefile builds `src/ir/` (plan §3.2)
- **What**: `$(SRC)/ir/*.cpp` and `$(SRC)/ir/*.h` were added to `AST_SRCS` and `AST_HDRS`.
- **Why**: so the new files are compiled and a header change triggers a rebuild.

### 2026-10-24: `run.sh` checks the IR and runs programs (plan §3.3)
- **What**:
  - For each test, `run.sh` recreates `outputs/<name>/` and copies the source into it.
  - It checks every `.ll` with `clang -x ir -c -fverify-intermediate-code`.
  - When `test/expected/<name>.out` exists, it links the program into `build/run/<name>`, runs it, and compares the output with that file.
  - It reports a total, and exits non-zero if any IR is invalid or any output differs.
- **Why**:
  - The plan's original check, `clang -x ir -fsyntax-only`, cannot work: clang rejects every IR file with "cannot apply AST actions to LLVM IR file".
  - Release builds of clang skip the IR verifier unless `-fverify-intermediate-code` is given. Without it, a broken file (a value used before it is defined) was accepted.
  - Running the programs proves the IR does the right thing, not just that it is well formed.
  - The linked programs go under `build/`, which git ignores, so binaries never land in `outputs/`.
- **Rejected tests**: `run.sh` also reports an error if a rejected test left a `.ll` file behind.

### 2026-10-24: README describes the IR phase
- **What**: the Build, Run, Output, Test coverage and Layout sections now cover:
  - the third phase line `IR generation successful.`;
  - the `.ll` file in `outputs/<name>/`;
  - the clang check and the program runs done by `run.sh`;
  - tests 23–27 and `test/expected/`;
  - the `src/ir/` files and the three IR documents.
- **Why**: the README is the first thing a reader opens, and it described a two-phase analyzer.

### 2026-10-24: old flat output files removed
- **What**: the tracked files `outputs/<name>.txt`, `outputs/<name>-ast.txt` and `outputs/<name>-symbols.txt` were deleted. The same files are now produced inside `outputs/<name>/`.
- **Why**: they were replaced by the per-test folder layout. Leaving them would have kept stale copies next to the new ones.

---

## 5. Test changes (`test/`)

### 2026-10-24: `test17` defines its static member (plan §1.5)
- **What**: added `int Counter::total = 0;` after `class Counter`.
- **Why**: the class declares `static int total;` and uses it, but never defined it. That is a link error in real C++. The IR phase gives an undefined static member zero storage anyway, so the `.ll` was valid either way, but the test should be correct C++.

### 2026-10-24: new run tests 23–27, one per supported feature group
Each test prints its results with `printf`. Its expected output, in `test/expected/<name>.out`, was produced by compiling the **same source with clang++** and running it, not by our own compiler. The tests only use plain forms of the features in the README's "Supported syntax" list.

| Test | README feature group it covers |
| :--- | :--- |
| `test23_ir_expressions` | arithmetic, bitwise, shift, relational, logical, conditional, assignment and compound assignment, prefix/postfix, char/float/double values |
| `test24_ir_control_flow` | `if`/`else`, `switch`/`case`/`default` with fallthrough, `for`/`while`/`do`, `break`/`continue`, `goto`/labels, recursion |
| `test25_ir_arrays_pointers` | integer and char arrays, multi-dimensional arrays, multi-level pointers, pointer to array, references, structs, brace initialisers, `new`/`delete` |
| `test26_ir_classes` | classes, `this`, single and multiple inheritance, constructors and destructors, static members, a nested struct |
| `test27_ir_statics_lambdas` | static globals, functions and locals; lambdas without captures, by copy, by reference, `[=]`, `[&]`, `[this]` |

### 2026-10-24: earlier broader tests withdrawn
- **What**: a first set of tests (globals, operators, control flow, objects, lambdas, functions, arrays, IR rejections) was deleted and replaced by tests 23–27.
- **Why**: they went beyond the features the project lists: tricks with function pointers, overloads, mutual recursion, global objects with destructors, name-collision cases, stdin input and a rejection test. The decision was to test only the supported feature list.

---

## 6. The new IR library (`src/ir/`)

Everything here is new. The class layout follows plan §2:

```
Value
├── Constant            (ConstantKind: Int, Float, Null, ZeroInit, String, Aggregate, Address)
├── Argument
├── GlobalVariable
├── Function            (owns BasicBlocks and Arguments)
├── BasicBlock          (owns Instructions, keeps predecessors/successors)
└── Instruction
    ├── ReturnInst
    ├── BranchInst, SwitchInst, UnreachableInst
    ├── MemoryInst ── AllocaInst, LoadInst, StoreInst, GEPInst
    ├── CallInst
    └── ExprInst  ── BinaryInst, UnaryInst, CastInst, SelectInst
```

| File | Responsibility (one each) |
| :--- | :--- |
| `ir_types.h/.cpp` | turning sema types and constant values into LLVM text; the IR-only 64-bit integer type |
| `value.h/.cpp` | `Value`, `Constant`, `Argument`, `GlobalVariable` |
| `instruction.h/.cpp` | the instruction classes and how each one prints |
| `cfg.h/.cpp` | `BasicBlock`, `Function` (including CFG clean-up), `Module` (owner of everything, prints the `.ll`) |
| `ir_builder.h/.cpp` | `IRBuilder`, the only code that puts instructions into blocks |
| `record_layout.h/.cpp` | `RecordLayout`: field and base positions, and whether a type needs construction or destruction. It only answers questions and writes no IR. |
| `constant_folder.h/.cpp` | `ConstantFolder`: the constant value of a global's initializer, or "not a constant" |
| `ir_visitor.h` + `ir_visitor.cpp` | `IRVisitor`: the passes, function bodies and globals |
| `ir_statements.cpp` | statements, locals, scopes and destructor clean-ups, `goto` |
| `ir_expressions.cpp` | lvalues, rvalues, operators and implicit casts |
| `ir_calls.cpp` | calls and lambdas |
| `ir_objects.cpp` | initialization, construction, destruction, base classes, `new`/`delete` |

---

## 7. Where the implementation differs from the plan, and why

The plan's design was followed. These are the places where details changed while building it.

| # | Plan said | Implemented | Reason |
| :- | :--- | :--- | :--- |
| 1 | `Module` uses `llvm::StringMap` | `std::map` | No new LLVM dependency in `src/ir/`, and `std::map` is plain standard C++ that is easy to read. Sema still uses `llvm::StringMap`. |
| 2 | Type formatters in `value.h` | Separate `ir_types.h` | One responsibility per file. The formatters are used by every class, not only by values. |
| 3 | Constant kinds `Int`, `Bool`, `Char`, ... | `Bool` and `Char` folded into `Int` (the type gives the width); new `Address` kind | An integer constant is printed the same way at every width. `Address` (`ptr @g`) is needed because global initializers such as `int *p = &target;` must be constants. |
| 4 | `escapeLLVMString` appends `\00` | the caller passes every byte, including `\0` and padding | The same function then serves `"hi"` (3 bytes) and `char s[8] = "hi"` (8 bytes). |
| 5 | Global destructors through `@llvm.global_dtors` | `@__global_init` calls `atexit(@__global_fini)` | On macOS `@llvm.global_dtors` becomes a `__mod_term_func` section, which the system loader no longer runs, so destructors never ran. This is also how clang does it, and output printed by destructors appears before stdout is flushed. |
| 6 | A suppressed instruction returns a placeholder constant | The instruction is still made and handed to `Function::discard` | The caller gets a real value of the right type with no special cases. The discarded instructions are never printed. |
| 7 | `insertAllocaAfterLastAlloca` / `allocaInsertIndex` on `Function` | `BasicBlock::insertAlloca` with `allocaCount` | The count belongs to the entry block, which is where the allocas go. |
| 8 | `ClosureRecordDef` list | `Module::addClosure(LambdaExpr*)`; the layout is printed from `usedCaptures` | One source of truth, the same captures sema used for its own layout. |
| 9 | Local slots named `%x.addr` | locals `%x`, parameters `%x.addr` | Same as clang `-O0`. It makes clear which slots hold parameters. |
| 10 | Blocks appear in creation order | A block joins the output when the builder starts writing into it | Blocks are printed in the order the code runs through them, as clang does, so the `.ll` reads top to bottom. |
| 11 | Reference-returning calls and `return` of `T&` | Removed | The grammar cannot declare a function that returns a reference, so that code could never run. |
| 12 | `test23_ir_edge_cases.ckp` with every §5 rule | Tests 23–27, one per README feature group | The decision was to test only the supported feature list (see section 5). |
| 13 | `FunctionContext` fields | Same idea, plus `LabelInfo` per label and `returnBlock` for destructors | `LabelInfo` lets a forward `goto` know which destructors to run. `returnBlock` makes a `return` inside a destructor still destroy the members. |
| 14 | Forward element loops exit on `icmp eq` | They exit on `icmp uge` | If a list gives more elements than a runtime `new T[n]` count, the loop starts past its end and `eq` would never stop. |

### Kept from the plan although no test uses it yet
These parts of plan §5 handle forms the grammar accepts. They stay so that such a program compiles correctly or is refused with a clear message, instead of producing wrong IR:
- the 8-byte element count in front of `new T[n]` when `T` has a destructor, so `delete[]` can destroy every element (§5.11);
- constructors and destructors for global objects (§5.8);
- the run-once guard for a static local with a non-constant initializer (§5.8);
- the two errors of §5.12: a base or member class with no constructor taking no arguments, and a `goto` that jumps into a scope past an object's construction.

### Known limitations (unchanged from the plan)
- A static local object with a destructor is never destroyed.
- A record returned by value and then discarded is never destroyed. There are no destructor calls for temporaries.
- `bool` is stored as `i1`, not `i8` as clang does. This is valid IR, but the layout does not match clang's C++ ABI.

---

## 8. Verification record

| Date | Check | Result |
| :--- | :--- | :--- |
| 2026-10-24 | Build with `-Wall -std=c++17` (with a stand-in for `llvm/ADT/StringMap.h`, because LLVM could not be installed: disk full) | 0 warnings, 0 errors |
| 2026-10-24 | `run.sh`: 27 tests | 14 accepted, 13 rejected. Every accepted `.ll` passes `clang -x ir -c -fverify-intermediate-code`. Tests 23–27 print exactly what clang++ prints for the same source. The 13 rejected tests report the same error counts as before this work: 16, 13, 1, 1, 1, 2, 1, 1, 6, 25, 6, 11, 15. |
| 2026-10-24 | Original tests 4, 17 and 22 against clang++ (scratch copies, not added to the repo) | The same output and exit code as clang++. These tests read uninitialized variables (test17 `cube`, test22 `grid`, `line`, `origin`, `g.cells`), or declare constructors without bodies, which C++ cannot link (test4 `Animal`). The copies only initialize those values and give the constructors bodies that print. This covers implicit constructors and destructors, protected and multiple inheritance, nested classes, static locals, static members, multi-dimensional `new`, function pointers and pointer arithmetic. |
| 2026-10-24 | Original tests 3 and 7 linked and run (input `5` for `scanf`) | Every printed value checked by hand (`squares=204`, `trace=12`, `evens=42`, `gcd=6`, `fact=720`, `67600`, ...). |
| 2026-10-24 | LLVM 23.1.4 installed with Homebrew; clean `make` through the unchanged Makefile, the stand-in header retired | 0 warnings, 0 errors |
| 2026-10-24 | `make test` with the real-LLVM build | Same as the `run.sh` row above: 14 IR valid, 5 of 5 run tests match, 13 rejected with unchanged counts |
| 2026-10-24 | LLVM 23's own verifier, `opt -passes=verify`, on all 14 generated `.ll` files (a second, newer verifier than Apple clang 17) | 14 checked, 0 failed |
