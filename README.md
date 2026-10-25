# CSC-305 Compiler: a C++ Subset to LLVM IR

## Introduction

This project is a compiler for a subset of C++. It reads a source file, checks
it, and translates it into LLVM IR, the text form of the intermediate language
used by the LLVM compiler tools. The generated IR can be compiled into a
working program with `clang`.

The compiler works in four phases, and each phase runs only if the previous one
found no errors:

1. **Scanner** (`src/lexer.l`): splits the source text into tokens.
2. **Parser** (`src/parser.y`, `src/ast/`): checks the grammar and builds the
   abstract syntax tree (AST).
3. **Semantic analysis** (`src/semantic/`): builds the symbol table, then checks
   the types and the meaning of every declaration, statement and expression.
4. **IR generation** (`src/ir/`): builds a control flow graph for every function
   and writes it out as an LLVM IR file (`.ll`).

### Team members

- Adityavardhan Singh (24322005)
- Garima Sidana (24113043)
- Manjot Singh (24116048)
- Naivadhya Garg (24115106)

### Implementation language and tools

- The compiler is written in **C++17**.
- The scanner is generated with **Flex** and the parser with **Bison**.

### LLVM utilities used

- **`llvm-config`** gives the Makefile the compiler and linker flags for the
  installed LLVM.
- **The LLVM Support library** is the only LLVM library that is linked. From it
  the compiler uses one data structure, `llvm::StringMap`, a hash map from
  names to values, which stores the names in each scope of the symbol table.
- **No LLVM IR libraries are used.** The compiler does not use `llvm::Module`,
  `llvm::IRBuilder` or `llvm::Type`. The IR phase builds its own control flow
  graph and writes the `.ll` text itself.
- **`clang`** is used only by the test script. It checks that every generated
  `.ll` file is valid LLVM IR, and it compiles and runs the test programs whose
  output is compared with an expected result.

### Test file format

Test programs are plain C++ source files with the extension `.ckp`. The
extension is only our naming convention: the compiler reads any file path and
does not check the extension. The test script runs every `.ckp` file in the
`test/` folder.

## Build and Run

### Requirements

- `flex`
- `bison`
- a C++17 compiler (`g++` on Linux, `clang++` on macOS)
- LLVM, with `llvm-config` available
- `clang`, which the test script needs to check and run the generated IR

We built and tested the project with flex 2.6.4, bison 3.8.2, g++ 16.2.1 and
LLVM and clang 23.1.3. The generated IR uses opaque pointers (`ptr`), so the
`clang` that reads it must be version 15 or newer.

On Linux the Makefile uses `g++` and the `llvm-config` found on the `PATH`. On
macOS it uses `clang++` and Homebrew's `llvm-config`, because Homebrew's LLVM is
built with libc++ and its headers must be compiled with the matching compiler.
Both can be changed on the command line:

```bash
make CXX=g++-14 LLVM_CONFIG=/usr/bin/llvm-config-20
```

The grammar uses `%error-verbose`, so it also builds with the old Bison 2.3
that ships with macOS. Newer versions of Bison accept it and print a
deprecation warning, which can be ignored.

### Building from scratch and running the tests

To rebuild everything from a clean state and run every test, use:

```bash
make clean
make test
```

- **`make clean`** deletes the `build/` folder (the generated scanner and parser
  sources, the compiled compiler and the linked test programs) and the
  `outputs/` folder (the results of the last test run).
- **`make`** (or `make all`) generates the scanner and parser in `build/` and
  compiles the compiler into `build/parser`.
- **`make test`** runs `run.sh`. The script first runs `make all`, so the
  compiler is always up to date, and then runs every test.

Other ways to run:

```bash
./run.sh                                     # the same as make test
./run.sh path/to/executable                  # test an already built compiler, without building
./build/parser test/test1_core_features.ckp  # compile a single file
```

### Test Script

`run.sh` runs every `.ckp` file in the `test/` folder, whatever its name. Files
whose names contain a number run in the order of that number, so `test2` runs
before `test10`. Files without a number run last, in alphabetical order. No test
is named inside the script, so adding a test only means adding a file.

Each run starts by deleting and recreating `outputs/` and `build/run/`, so
nothing is left over from an earlier run. Then, for every file:

1. The compiler is run on the file, and everything it prints is saved.
2. If the compiler rejects the file, the script prints `REJECTED`, the number of
   errors, and the error messages.
3. If the compiler accepts the file, `clang` checks the generated `.ll` file
   with its IR verifier. The script prints `PARSED & IR VALID` or `IR INVALID`.
4. If `test/expected/<name>.out` exists, the program is also compiled with
   `clang`, run, and its output is compared with that file. The script prints
   `output matches` or `OUTPUT DIFFERS`. If `test/inputs/<name>.in` exists, it is
   given to the program as its input.

At the end the script prints how many files were accepted and rejected, how
many IR files were valid, and how many programs printed the expected output.
It exits with a failure status if any IR file is invalid or any program prints
the wrong output.


### Output files

For every test, `run.sh` creates a folder `outputs/<name>/`. The folder holds
the original file and the intermediate representations the compiler produced
for it, one for each phase:

| File | Contents |
| --- | --- |
| `<name>.ckp` | a copy of the original source file, for reference |
| `<name>.txt` | everything the compiler printed: the phase messages and the token table for an accepted file, or the error messages for a rejected one |
| `<name>-ast.txt` | the abstract syntax tree after semantic analysis, including the implicit conversions the type checker inserted |
| `<name>-symbols.txt` | the symbol table: every scope, and every name in it with its type, its mangled name and the LLVM type it lowers to |
| `<name>.ll` | the program in LLVM IR |
| `<name>.run.txt` | what the program printed, for tests that are run |

Which files appear depends on how far the file got:

- A file with **lexical or syntax errors** has only the `.ckp` and `.txt` files,
  because no tree was built.
- A file with **semantic errors** also has the AST and the symbol table, which
  show how far the analysis got, but no `.ll` file.
- An **accepted** file has all of them.

When `clang` rejects an IR file or a program prints the wrong output, the
folder also gets a `.verify.txt`, `.link.txt` or `.diff.txt` file that explains
the problem.

An accepted file prints:

```text
Parsing successful.
Semantic analysis successful.
IR generation successful.
```

followed by the token table. Errors are printed with their position, and the
messages are sorted by line:

```text
test/test15_access_errors.ckp line: 17 col: 16 semantic error: 'shut' is a private member of 'Base'
6 semantic error(s) found.
```

## Test Coverage

There are 18 test files. Tests 1 to 10 are **positive tests**, which must be
accepted and must produce valid IR. Six of these are also compiled, run, and
checked against an expected output. Tests 11 to 18 are **negative tests**, which
must be rejected with errors.

### Positive tests

| Test | What it covers | How it is checked |
| --- | --- | --- |
| `test1_core_features` | the basic language in one program: `if`/`else`, every loop, `break`, `continue`, `goto`, all operator groups, every literal form, `switch`, structures, arrays, pointers, references, `null` and `nullptr`, recursion, function pointers, a lambda, `new` and `delete`, `argc`, `argv`, `printf` and `scanf`, classes with access specifiers, static members, constructors and destructors, single and multiple inheritance, nested classes, an empty statement, and line and block comments | valid IR |
| `test2_all_features` | one large program that uses every supported feature together | valid IR |
| `test3_brace_static_multidim` | static variables, functions, locals and members, brace initialisation without `=` (`int a{5};`, `Point p{1, 2};`), `new` with a brace list, and multi-dimensional arrays as parameters and through pointers and members | valid IR |
| `test4_pointer_types` | the type system: pointers of several levels, pointers to arrays, a pointer to a function pointer, a reference to an array, `new` with a pointer element type, pointer arithmetic, `null`, `nullptr` and `void *` | valid IR |
| `test5_ir_expressions` | every operator group, compound assignment, increment and decrement, and `char`, `float` and `double` values | valid IR and output |
| `test6_ir_control_flow` | `if`/`else`, `switch` with fallthrough, loops with `break` and `continue`, `goto`, recursion | valid IR and output |
| `test7_ir_arrays_pointers` | arrays, brace initialisers, pointers of several levels, pointer arithmetic, a pointer to an array, references, structures, `new` and `delete` | valid IR and output |
| `test8_ir_classes` | the order in which constructors and destructors run, multiple inheritance, static members, nested classes, objects destroyed at the end of their scope, `new` and `delete` with constructors and destructors | valid IR and output |
| `test9_ir_statics_lambdas` | static variables and functions, a static local that keeps its value between calls, lambdas with no captures, with copy and reference captures, with capture defaults, and capturing `this` | valid IR and output |
| `test10_ir_typedef_until` | `typedef` of builtins, pointers, arrays, function pointers and classes, a typedef inside a class and inside a block, `until` and `do`/`until` loops with `break` and `continue` | valid IR and output |

### Negative tests

| Test | Rejected by | Errors | What it covers |
| --- | --- | --- | --- |
| `test11_lexical_errors` | scanner | 16 | illegal characters, malformed numbers, an integer that is too large, bad character literals, an unterminated string, a bad escape sequence, an unterminated comment |
| `test12_unsupported_features` | parser | 9 | features we did not choose to support: `union`, `sizeof`, a variable argument list (`...`), and the assignments `<<=`, `>>=`, `&=`, `|=` and `^=` |
| `test13_syntax` | parser | 9 | one mistake in each function: a missing semicolon, a missing operand, a missing variable name, a missing closing parenthesis, tokens in the wrong order, a missing operator, a condition without parentheses, a parameter list ending in a comma, and an unclosed parameter list |
| `test14_type_errors` | semantic analysis | 25 | undeclared names, use before declaration, redefinition, incompatible initialisation and assignment, assigning to something that is not a variable, bad dereference, indexing, address-of and member access, operators that need integers, array sizes, `void` and `auto` misuse, `delete` on a non-pointer, `this` outside a method |
| `test15_access_errors` | semantic analysis | 6 | `private` and `protected` members used from outside, a private base member used in a derived class, and members reached through a privately inherited base |
| `test16_call_errors` | semantic analysis | 11 | missing or wrong return values, wrong argument counts, arguments that do not convert, no matching overload, calling something that is not a function, a mismatched function pointer, a method called without an object or used as a value |
| `test17_flow_errors` | semantic analysis | 15 | `break` and `continue` outside a loop, `goto` to a missing label, a duplicate label, a `switch` on a non-integer, duplicate and non-constant `case` values, two `default` labels, `case` and `default` outside a `switch`, a structure used as a condition, a `for` variable used after its loop |
| `test18_typedef_until_errors` | semantic analysis | 11 | a typedef declared twice, a typedef of an unknown type or of `auto`, a variable of a `void` typedef, `struct` used with a typedef name, a typedef used as a value, a typedef used after its block ends, `until` conditions that are not scalars, `break` outside a loop |

## Supported Features

**Types**

- `int`, `char`, `bool`, `float`, `double`, `void` and `auto`.
- Pointers of any depth, such as `int ***p`, and `void *`.
- References, as variables and as parameters.
- Arrays of any number of dimensions. The first dimension may be left empty
  when an initialiser gives the size, as in `int a[] = {1, 2, 3};`.
- Pointers to arrays, such as `int (*p)[3]`, and combinations such as
  `int **(*p)[10]`.
- Function pointers, as variables, as parameters, and pointers to them.
- `typedef`, at file scope, inside a block and inside a class.
- Structures and classes.

**Literals**

- Integers in decimal, hexadecimal (`0x1F`), octal (`0755`) and binary
  (`0b1010`).
- Floating-point numbers such as `3.14`, `.5`, `2.`, `1e10` and `1.5e-3`.
- Characters with escape sequences, such as `'\n'` and `'\x41'`.
- Strings with escape sequences.
- `true`, `false`, and the null pointer written as `null` or `nullptr`.

**Operators**

- Arithmetic, relational, logical, bitwise and shift operators.
- The conditional operator `?:`.
- Assignment, and the compound assignments `+=`, `-=`, `*=`, `/=` and `%=`.
- Prefix and postfix `++` and `--`.
- Unary `+`, `-`, `!`, `~`, `*` (dereference) and `&` (address of).
- Member access with `.` and `->`, indexing with `[]`, and function calls.

**Statements**

- `if` and `else`.
- `switch` with `case` and `default`, including fallthrough.
- `for`, `while`, `do`/`while`, and the extra loops `until (c)` and
  `do ... until (c);`, which repeat while the condition is false.
- `break`, `continue`, `return`, `goto` and labels.
- Blocks and the empty statement.

**Functions**

- Functions with parameters, return values and recursion.
- Overloading by parameter types.
- `static` functions.
- `int main(int argc, char *argv[])`.
- `printf` and `scanf`, which are always available. Other external functions
  can be declared in the source and then called.

**Initialisation and memory**

- `int a = 5;`, `int a{5};`, `Dog d(3);`, and nested brace lists such as
  `int g[2][2] = {{1, 2}, {3, 4}};`.
- A string literal used to fill a `char` array.
- `new T`, `new T(args)`, `new T{...}`, `new T[n]`, `new T[n]{...}` and
  `new T[n][3]`, and `delete` and `delete[]`.
- Static global variables, static local variables and static class members.

**Classes**

- `public`, `private` and `protected` members.
- Single and multiple inheritance, with or without an access specifier.
- Classes and structures nested inside other classes.
- Constructors (including overloaded ones) and destructors, defined inside or
  outside the class.
- Methods, static methods and `this`.
- Definitions outside the class with qualified names of any depth, such as
  `int Counter::total = 0;` or `void A::B::f() { ... }`.

**Lambdas**

- Capture by copy (`[x]`), by reference (`[&x]`), capture of `this`, and the
  capture defaults `[=]` and `[&]`.
- Calling a lambda through a variable, or calling it directly where it is
  written.

## Unsupported Features

- The types `short`, `long`, `unsigned` and `signed`, and the qualifiers
  `const` and `volatile`.
- `enum`, `union`, namespaces, `using`, templates and exceptions.
- Explicit casts, such as `(int)x` or `static_cast<int>(x)`.
- `sizeof`. The scanner still reads it as a keyword, but the grammar does not
  accept it.
- The comma operator, as in `a = 1, b = 2;` or `for (i = 0, j = 0; ...)`.
- The compound assignments `<<=`, `>>=`, `&=`, `|=` and `^=`.
- Declarations inside a condition, such as `if (int x = f())`.
- Default arguments and variable argument lists (`...`) in our own functions.
- Functions that return a reference, such as `int &f()`.
- Arrays of function pointers and fully nested declarators such as
  `int (*(*f)(int))[3]`.
- Structures and classes defined inside a function body.
- Using `::` inside an expression, such as `A::f()` or `x = Counter::total;`.
  Qualified names are only allowed in declarations.
- Virtual functions, virtual inheritance, operator overloading and constructor
  initialiser lists.
- In lambdas: a written return type (`-> int`), `mutable`, captures with an
  initialiser, and converting a lambda to a function pointer.
- The literal forms `1.5f` (a float suffix), hexadecimal floats, joining two
  string literals written next to each other, and integers too large for `int`.
- The C++ standard library, such as `iostream`, strings and containers. Only
  `printf` and `scanf` are available without a declaration.

## Parser Phase Notes

- The grammar has no shift/reduce or reduce/reduce conflicts.
- An `else` always belongs to the nearest `if`.
- A statement of the form `a * b;` is always read as a declaration of a pointer
  `b` of type `a`, because it has the same tokens as `Dog * p;`. If `a` is a
  variable, semantic analysis then reports `unknown type name 'a'`.
- Inside a function body, a class type followed by a parenthesised declarator
  must be written with `struct`, as in `struct Dog (*dogs)[2];`. Without it,
  `Dog (` looks like a call. At file scope `Dog (*dogs)[2];` is accepted.
- `enum Colour { RED };` is not a syntax error. It has the shape of a variable
  declaration with a brace initialiser, so it is rejected later, by semantic
  analysis, as an unknown type name.
- A `typedef` cannot have an initialiser and cannot use a qualified name. These
  two mistakes are reported as syntax errors.
- After a syntax error the parser skips ahead to the next `;` and continues, so
  one file can report several errors. A mistake that leaves a `{` or `}`
  unmatched can cause extra errors in the code that follows it, because the
  parser does not resynchronise at braces.
- Semantic analysis does not run when there are lexical or syntax errors.

## Semantic Phase Notes

These points describe where the rules differ from C++, or what is not checked.

- Names must be declared before they are used. The exception is the members of
  a class, which can be used anywhere inside that class.
- Types, variables and functions share one set of names in each scope, so
  `struct S { ... };` followed by `int S;` is a redefinition. In C++ a struct
  name and a variable name may coexist.
- Only `null` and `nullptr` are null pointers. The integer `0` cannot be
  assigned to a pointer.
- There is no implicit conversion between integers and pointers, and a
  `void *` does not convert back to another pointer type.
- Two pointers of unrelated types cannot be compared.
- Conversions that lose information, such as `double` to `int`, are allowed
  silently, even inside braces (`int a{2.5};`). C++ rejects them inside braces.
- An integer literal larger than the largest `int` is an error, because there
  is no `long`.
- Overloaded functions must differ in their parameters. When two overloads fit
  a call equally well, the call is reported as ambiguous.
- A class that declares no constructors can be default constructed and copied.
  A class that declares constructors cannot be copied with `A b(a);` unless it
  declares a matching constructor itself, because no copy constructor is
  generated for it.
- A function or constructor may be declared without a definition. This is
  accepted, and a constructor or destructor declared this way is treated as
  doing nothing beyond building or destroying the bases and members.
- A class uses `private` access by default and a structure uses `public`, for
  both members and inheritance. Access rules are also checked along the
  inheritance path, and converting a derived object to a base is not allowed
  through a `private` or `protected` base.
- There is no virtual inheritance. A member that can be reached through two
  different bases is ambiguous, even when both paths lead to the same
  grandparent class, and so is a conversion to a base class that appears twice.
- `main` must return `int`.
- The `switch` condition and every `case` value must be integers, and `case`
  values must be constants.
- Labels belong to the whole function. A `goto` cannot leave or enter a lambda.
- A function that can reach its end without a `return` is not reported.
- A lambda can only capture local variables. Its return type comes from its
  first `return`, and every other `return` must have the same type. A variable
  captured by copy can be changed inside the lambda without `mutable`.
- `auto` takes the type of its initialiser, after arrays and functions become
  pointers. It cannot be used for members, parameters or return types.
- Array sizes must be positive constants. Only the first dimension may be left
  empty, and then the size comes from the initialiser. In `new T[n]` only the
  first size may be a variable.
- `struct X` only names a structure or class, never a typedef.

## IR Generation Phase Notes

- The IR is written in the style of `clang -O0`. Every variable gets a stack
  slot (`alloca`) at the start of the function, and every read and write is a
  `load` or a `store`. No `phi` instructions are used.
- The generated file has no target triple or data layout, so `clang` compiles
  it for the machine it runs on. Sizes for `new` are computed with a
  `getelementptr` on a null pointer, which does not depend on the target.
- All pointers are written as the opaque type `ptr`.
- Floating-point constants are written as 64-bit hexadecimal values, so they
  keep their exact value.
- Methods become ordinary functions whose first parameter is the object
  (`this`). Names are mangled so that overloads and nested classes never
  produce the same name:

  | Source | Name in the IR |
  | --- | --- |
  | `int add(int, int)` | `global_3add_ii` |
  | `void Animal::setAge(int)` | `class_6Animal_6setAge_i` |
  | `Animal()` and `Animal(int)` | `class_6Animal_C_v`, `class_6Animal_C_i` |
  | `Animal::~Animal()` | `class_6Animal_D_v` |
  | `int Outer::Inner::f()` | `class_5Outer5Inner_1f_v` |
  | `static int Counter::total` | `class_7Counter_5total` |

  `main`, `printf` and `scanf` keep their own names so that the program links.
- An object is laid out with its base classes first, in the order they are
  written, followed by its own fields. There are no virtual tables.
- Constructors build the bases and members first, then run their own body.
  Destructors run their body first, then destroy the members and bases in
  reverse order.
- Local objects are destroyed at the end of their block, newest first. They are
  also destroyed when the block is left early with `return`, `break`,
  `continue` or `goto`.
- A `goto` that jumps into a block past the construction of an object is
  rejected in this phase. The message starts with `cannot generate code:`.
- Copying a class object copies its memory directly. No copy constructor or
  assignment operator is called.
- `new` calls `malloc`, and `delete` runs the destructor and then calls `free`.
  `delete[]` runs the destructor of every element.
- `&&`, `||` and `?:` are lowered with branches, so the side that is not needed
  is never evaluated.
- A function that reaches its end without a `return` returns zero (or nothing,
  for a `void` function). This is how `main` returns 0 without a `return`.
- A global variable whose initial value is not a constant is initialised by a
  function that runs before `main`. A static local is initialised the first
  time its declaration is reached, and a flag records that this has happened.
- A lambda becomes a structure that holds its captures and a separate function
  that receives a pointer to that structure. A capture by copy holds the value,
  and a capture by reference holds the address of the variable.
- Integer division and right shift are signed, and `char` is a signed type.
  Pointers are compared as unsigned addresses.
