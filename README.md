# CSC-305 Assignment 3: Semantic Analyzer

Team members:

- Adityavardhan Singh (24322005)
- Garima Sidana (24113043)
- Manjot Singh (24116048)
- Naivadhya Garg (24115106)

Source language: submitted C++ subset

Implementation language: C++17 with Flex, Bison and the LLVM type API

The test files keep the `.ckp` name used in Phase 1. This is only our file
naming convention. The analyzer reads source text from any file path and does
not check its extension.

## Build

The project requires `flex`, `bison`, a C++17 compiler, and LLVM 18 or newer.

LLVM is found through `llvm-config`. On macOS the Makefile looks for Homebrew's
copy, which is not on the `PATH`, and uses `clang++`, because Homebrew's LLVM is
built against libc++ and its headers must be compiled by the matching compiler.
On Linux it uses `llvm-config` from the `PATH` and `g++`. Both can be overridden:

```bash
make CXX=g++-14 LLVM_CONFIG=/usr/bin/llvm-config-20
```

Only LLVM support ADTs are linked (`llvm-config --libs support`). No LLVM IR
library or types are used.

```bash
make clean && make
```

This generates the scanner and parser files under `build/` and creates the
executable `build/parser`.

The grammar is written with `%error-verbose` rather than
`%define parse.error verbose`, so that it also builds with Bison 2.3, which is
the version shipped with macOS. Newer Bison accepts it with a deprecation
warning.

## Run

```bash
make test                          # builds, then runs every test case
./run.sh                           # the same thing
./run.sh path/to/executable        # run an already built executable instead
./build/parser test/test1_control_flow.ckp
```

`make test` builds the analyzer before running, so the binary is never stale.
Passing an executable path skips the build and uses the executable given.

`run.sh` runs every `.ckp` file in `test/` and reports what happened to each:
either the file was accepted and a token table was produced, or the analyzer
reported errors, in which case the errors are shown with a count. No test case is named
in the script and no outcome is assumed in advance, so adding a test case means
only adding the file. The full output for every file is written to `outputs/`.

## Output

A file that is accepted prints both phases, then the token table:

```text
Parsing successful.
Semantic analysis successful.

Token                | Token_Type
---------------------+---------------
int - keyword        | KEYWORD
min - identifier     | PROCEDURE
( - punctuator       | PUNCTUATOR
int - keyword        | KEYWORD
a - identifier       | INT
, - punctuator       | PUNCTUATOR
int - keyword        | KEYWORD
b - identifier       | INT
) - punctuator       | PUNCTUATOR
{ - punctuator       | PUNCTUATOR
int - keyword        | KEYWORD
result - identifier  | INT
```

The first column gives the lexeme with the lexical category the scanner
assigned it. The second column gives the token type. For a keyword, operator,
punctuator or literal this is the category in upper case. For an identifier it
is what the declaration said about it, which only the parser can know:

| Declared as | Token_Type |
| --- | --- |
| a variable or parameter | the type specifier of its declaration, upper cased: `INT`, `CHAR`, `DOUBLE`, or `DOG` for a class type |
| a function, constructor, destructor, or function pointer | `PROCEDURE` |
| a structure tag | `STRUCT` |
| a class name | `CLASS` |
| a statement label, and the target of a `goto` | `LABEL` |
| never declared in the file | `IDENTIFIER` |

The token table reports the written type specifier, so `int **p` and
`char *argv[]` report `INT` and `CHAR`. The full type of every name, pointers,
arrays and all, is in the symbol table dump described below.

### Semantic output

Every accepted file also produces two files under `outputs/`:

- `<name>-ast.txt`, the syntax tree, which after semantic analysis also shows
  the `ImplicitCastExpr` nodes the type checker inserted;
- `<name>-symbols.txt`, the scope tree with every symbol, its full type, its
  mangled name, and the LLVM type it lowers to.

A file with a semantic error reports it in the same form as a syntax error and
exits non-zero:

```text
test/test18_type_errors.ckp line: 33 col: 15 semantic error: cannot dereference a value of type 'int'
25 semantic error(s) found.
```


## Test coverage

There are twenty-two test cases. Nine are accepted and produce a token table;
thirteen are rejected and report errors.

Feature coverage:

- **test1** control flow, jumps, calls, and the submitted operators.
- **test2** arrays, pointers, references, structures, member access, brace
  initialisers, and array `delete`.
- **test3** calls, recursion, command-line parameters, `printf` and `scanf`,
  function pointers including as parameters, lambdas, and dynamic allocation.
- **test4** classes, objects, inheritance, access specifiers, `this`, scope
  resolution, constructors and destructors both inside and outside the class
  body, and allocation with constructor arguments.
- **test5** switch labels and every submitted literal form.
- **test7** a comprehensive regression over every accepted feature.
- **test17** `static` variables, functions and members, list initialisation
  without `=` (`int a{5};`, `Point p{1, 2};`, `new int[3]{1, 2, 3}`), and
  multi-dimensional element access through arrays, pointers and members.
- **test22** the recursive type system: pointers of several levels, pointers to
  arrays, pointers to function pointers, references to arrays, `new` with a
  pointer element type, pointer arithmetic, and `null` and `void *` conversions.

Cases that must be rejected:

- **test6** the Phase 1 lexical errors, reported by the scanner.
- **test8** constructs left unticked on the feature form, which the grammar
  deliberately does not accept.

Semantic errors, one class of error per file:

- **test18** undeclared and redeclared names, use before declaration,
  incompatible initialisation and assignment, assignment to something that is
  not a variable, bad dereference, index, member access and `delete`, array
  sizes that are not positive constants, and `this` outside a member function.
- **test19** access control: `private` and `protected` members from outside the
  class, a private base member from a derived class, and members reached
  through a privately inherited base.
- **test20** calls: wrong argument counts, arguments that do not convert,
  overload resolution with no match, calling a value that is not a function, a
  function pointer that does not match, a member function without an object,
  and a member function used as a value instead of being called.
- **test21** control flow: `break` and `continue` outside a loop, `goto` to a
  label that does not exist, a duplicate label, a `switch` on a non-integer,
  duplicate and non-constant `case` values, two `default` labels, `case` and
  `default` outside a `switch`, conditions that are not scalars, and a `for`
  variable used after its loop.

One malformed construct per file, so that it is clear which construct produced
which message:

- **test9** a missing semicolon.
- **test10** an expression with a missing operand.
- **test11** a declaration with a missing variable name.
- **test13** an unbalanced bracket.
- **test14** jumbled tokens.
- **test15** a missing operator.
- **test16** missing brackets around a condition and in a parameter list.

**test12** is named for an extra semicolon but parses successfully, because a
stray `;` is an empty statement and is well formed. It is kept as a case that
confirms the analyzer does not reject valid input.

## Grammar notes

The grammar has no shift/reduce or reduce/reduce conflicts.

`pointer` is one or more stars, so a declarator may carry any number of
pointer levels. `paren_suffix` covers both suffixes that may follow `(*name)`:
a parameter list gives a pointer to function, and a dimension list gives a
pointer to array, which is the declarator required to hold a multi-dimensional
allocation such as `new int[3][5]`. Stars may appear on both sides of the
parentheses, which is what `int **(*p)[10]` needs.

A class type followed directly by a parenthesised declarator is offered only at
file scope, through `named_parenthesized_declaration`. Inside a class `Dog (`
would begin a constructor, and inside a block it has the shape of a call, so
`struct Dog (*dogs)[2]` is written there instead.

`parenthesized_parameter_declarator` allows the same shape in a parameter list,
so a pointer to function, a pointer to array or a reference to array can be
passed as an argument. The parameter name may be omitted there, as it may for a
plain parameter, which permits forms such as `int applyTwice(int (*)(int), int);`

The dangling `else` binds to the nearest `if` through `LOWER_THAN_ELSE`. A statement
of the form `a * b;` has the same token sequence as the declaration `Dog * p;`,
and `%nonassoc IDENTIFIER_EXPRESSION` together with `%nonassoc TOK_STAR
TOK_BITAND` resolves it toward the declaration reading. 

## Semantic analysis

After a successful parse the analyzer builds a symbol table and type checks the
tree. Both passes live in `src/semantic/` and share the visitor in
`src/semantic/visitor.h`:

- `types.h` holds the recursive type system. A compound type points at the type
  it is built from, so `int **(*p)[10]` is a pointer to an array of ten
  pointers to a pointer to `int`. It also holds the helpers the type checker
  needs, such as `isSameType`, `decay` and `findConversion`, and `toLLVM`,
  which lowers a type to an `llvm::Type`.
- `symbols.h` holds the symbols and the scope tree. Every scope is kept after
  it is built, and the tree nodes that open a scope point at theirs, so later
  passes walk back into the same scopes. The `SymbolTable` at the root owns
  both the scopes and every compound type, because types are shared: the type
  of `&x` points at the type of `x`.
- `symbolbuilder.h` declares every name and resolves every written type. It
  runs in two passes over the file, because a class body sees all of its
  members at once: first every record with its members and every function
  signature and global, then the bodies with their parameters and locals.
- `typecheck.h` works out the type of every expression and checks each use. It
  writes its results into the tree: the type and value category of every
  expression, the symbol every name resolves to, an `ImplicitCastExpr` around
  every silent conversion, and the constructor each declaration runs.

Methods become global functions that take the object as a `this` parameter.
Each function symbol keeps both signatures: the one callers are checked
against, and the lowered one with `this` in front.

`docs/semantic-phase.md` explains all of this in full: the type system and why
it is not `llvm::Type`, who owns the types, the declarator rewrite, the scope
tree, the mangling scheme, and the edge cases behind each semantic field on the
tree.

Names are mangled with each part written as its length followed by the name,
so overloads and names containing underscores cannot collide:

| Source | Mangled |
| --- | --- |
| `int add(int, int)` | `global_3add_ii` |
| `void Animal::setAge(int)` | `class_6Animal_6setAge_i` |
| `Animal()` and `Animal(int)` | `class_6Animal_C_v`, `class_6Animal_C_i` |
| `Animal::~Animal()` | `class_6Animal_D_v` |
| `int Outer::Inner::f()` | `class_5Outer5Inner_1f_v` |
| `static int Counter::total` | `class_7Counter_5total` |

`main`, `printf` and `scanf` keep their own names so that the program links.
Parameter types are written as `v b c i f d` for the builtins, `P` for a
pointer, `R` for a reference, `A<n>_` for an array, `F..E` for a function, and
a length-prefixed name for a record.

## Layout

```text
docs/
  semantic-phase.md
src/
  lexer.l
  parser.y
  token_types.h
  declarator.h
  ast/
    ast.h base.h statements.h declarations.h expressions.h
    ast_printer.h ast_printer.cpp
  semantic/
    visitor.h
    types.h types.cpp
    symbols.h symbols.cpp
    symbolbuilder.h symbolbuilder.cpp
    typecheck.h typecheck.cpp
    constants.h constants.cpp
    diagnostics.h
test/
  test1_control_flow.ckp
  test2_arrays_pointers_struct.ckp
  test3_functions_recursion.ckp
  test4_oop_features.ckp
  test5_switch_literals.ckp
  test6_lexical_errors.ckp
  test7_all_features.ckp
  test8_unticked_features.ckp
  test9_missing_symbol.ckp
  test10_missing_operand.ckp
  test11_missing_varname.ckp
  test12_extra_semicolon.ckp
  test13_imbalanced_bracket.ckp
  test14_jumbled.ckp
  test15_missing_operator.ckp
  test16_brackets.ckp
  test17_brace_static_multidim.ckp
  test18_type_errors.ckp
  test19_access_errors.ckp
  test20_call_errors.ckp
  test21_flow_errors.ckp
  test22_pointer_types.ckp
Makefile
run.sh
README.md
```

The generated files are kept out of `src/`:

```text
build/parser.tab.cpp
build/parser.tab.h
build/parser.output
build/parser.stamp
build/lex.yy.cpp
build/parser
```

`run.sh` saves the token table, the syntax tree and the symbol table dump for
each test under `outputs/` and reports the result of each without assuming what
it should be.

## Supported syntax

The grammar covers the features selected for our submission:

- arithmetic, relational, logical, bitwise, shift, conditional, assignment,
  prefix/postfix, address, dereference, member-access, and call expressions;
- `if`/`else`, `for`, `while`, `do`/`while`, and `switch` with `case` and
  `default`;
- `goto`, labels, `break`, `continue`, and `return`;
- typed functions, calls with arguments, recursive call syntax, and
  `main(int argc, char *argv[])`;
- integer and character arrays, multi-dimensional arrays, pointers of any
  number of levels such as `int ***p`, references, function pointers as
  variables and as parameters, pointers to arrays written as `int (*p)[3]`,
  and the two combined as `int **(*p)[10]`;
- brace initialisers, both `= { ... }` and the list form without `=` such as
  `int a{5};`, `int g[2][2]{{1, 2}, {3, 4}};` or `Point p{1, 2};`, including
  nested ones for multi-dimensional arrays and structures, and after `new`
  as in `new int[3]{1, 2, 3}` or `new Point{1, 2}`;
- `static` variables and functions at file scope, `static` locals, and
  `static` class members;
- multi-dimensional element access of any depth, such as `cube[i][j][k]`,
  `p->cells[1][2]` or `m[i][j]++`;
- structures, classes, objects, inheritance with or without an access
  specifier and from more than one base, classes and structures defined inside
  another, `public`, `private`, `protected`, `this`, `.`/`->`, and scope
  resolution of any depth, such as `A::m` or `A::B::f`;
- constructors and destructors
- lambdas


## Deliberately unsupported syntax

This is the submitted C++ subset, not a complete ISO C++ grammar. Each of the
following was checked against the built parser and is rejected:

- `typedef`, `using`, `enum`, or `union`
- variadic `...` parameters
- `until` loops
- `sizeof` expressions, although the scanner still recognizes `sizeof` as a
  Phase 1 keyword token
- `<<=`, `>>=`, `&=`, `|=`, or `^=`
- special file-manipulation grammar or built-ins

- templates, namespaces, exceptions, STL syntax, operator overloading, or other
  unselected C++ features.

The scanner reads `typedef`, `enum`, `union`, and `until` as
identifiers, since none of them appears in the Phase 1 keyword table. That keeps
Phase 1 behavior unchanged, but their unsupported C++ roles do not parse as
valid syntax. Similarly, a name such as `fopen` may only appear through the
ordinary identifier and function-call grammar.

## Token categories

| Category | Examples or meaning |
| --- | --- |
| `keyword` | the 31 Phase 1 keyword entries, plus `static` |
| `identifier` | letter or underscore followed by letters, digits, or underscores |
| `integer_literal` | decimal, `0x1F`, `0755`, `0b1010` |
| `float_literal` | `3.14`, `.5`, `2.`, `1e10`, `1.5e-3` |
| `char_literal` | `'a'`, `'\n'`, `'\x41'` |
| `string_literal` | double-quoted text with the submitted escapes |
| `operator` | the submitted arithmetic, logical, bitwise, access, and assignment set, and also `?` and `:` |
| `punctuator` | `( ) { } [ ] ; ,` |

Line comments and non-nesting block comments are skipped.
