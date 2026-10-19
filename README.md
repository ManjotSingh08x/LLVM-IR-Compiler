# CSC-305 Assignment 2: Syntax Analyzer

Team members:

- Adityavardhan Singh (24322005)
- Garima Sidana (24113043)
- Manjot Singh (24116048)
- Naivadhya Garg (24115106)

Source language: submitted C++ subset

Implementation language: C++17 with Flex and Bison

The test files keep the `.ckp` name used in Phase 1. This is only our file
naming convention. The analyzer reads source text from any file path and does
not check its extension.

## Build

The project requires `g++`, `flex`, and `bison`.

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
either the file parsed and a token table was produced, or the analyzer reported
errors, in which case the errors are shown with a count. No test case is named
in the script and no outcome is assumed in advance, so adding a test case means
only adding the file. The full output for every file is written to `outputs/`.

## Output

A file that parses prints `Parsing successful.` followed by the token table:

```text
Parsing successful.

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
| a variable or parameter whose declarator carries `*`, `&` or `[]` | that type marked as a class specifier: `INT_CLASSSPECIFIER`, `CHAR_CLASSSPECIFIER` |
| a function, constructor, destructor, or function pointer | `PROCEDURE` |
| a structure tag | `STRUCT` |
| a class name | `CLASS` |
| a statement label, and the target of a `goto` | `LABEL` |
| never declared in the file | `IDENTIFIER` |

A declarator carrying a star, an ampersand or brackets does not have the
declared type itself, so it reports that type marked as a class specifier:
`char *argv[]` gives `CHAR_CLASSSPECIFIER` rather than `CHAR`. 


## Test coverage

There are sixteen test cases. Seven parse and produce a token table; nine are
rejected and report errors.

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

Cases that must be rejected:

- **test6** the Phase 1 lexical errors, reported by the scanner.
- **test8** constructs left unticked on the feature form, which the grammar
  deliberately does not accept.

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

`parenthesized_declarator` covers both suffixes that may follow `(*name)`: a
parameter list gives a pointer to function, and a dimension list gives a pointer
to array, which is the declarator required to hold a multi-dimensional
allocation such as `new int[3][5]`.

`parenthesized_parameter_declarator` allows the same shape in a parameter list,
so a pointer to function, a pointer to array or a reference to array can be
passed as an argument. The parameter name may be omitted there, as it may for a
plain parameter, which permits forms such as `int applyTwice(int (*)(int), int);`

The dangling `else` binds to the nearest `if` through `LOWER_THAN_ELSE`. A statement
of the form `a * b;` has the same token sequence as the declaration `Dog * p;`,
and `%nonassoc IDENTIFIER_EXPRESSION` together with `%nonassoc TOK_STAR
TOK_BITAND` resolves it toward the declaration reading. 

## Layout

```text
src/
  lexer.l
  parser.y
  token_types.h
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

`run.sh` saves one output file for each test under `outputs/` and reports the
result of each without assuming what it should be.

## Supported syntax

The grammar covers the features selected for our submission:

- arithmetic, relational, logical, bitwise, shift, conditional, assignment,
  prefix/postfix, address, dereference, member-access, and call expressions;
- `if`/`else`, `for`, `while`, `do`/`while`, and `switch` with `case` and
  `default`;
- `goto`, labels, `break`, `continue`, and `return`;
- typed functions, calls with arguments, recursive call syntax, and
  `main(int argc, char *argv[])`;
- integer and character arrays, multi-dimensional arrays, one-level pointers,
  references, function pointers as variables and as parameters, and pointers to
  arrays written as `int (*p)[3]`;
- brace initialisers, including nested ones for multi-dimensional arrays and
  structures;
- structures, classes, objects, inheritance with or without an access
  specifier and from more than one base, classes and structures defined inside
  another, `public`, `private`, `protected`, `this`, `.`/`->`, and scope
  resolution of any depth, such as `A::m` or `A::B::f`;
- constructors and destructors
- lambdas


## Deliberately unsupported syntax

This is the submitted C++ subset, not a complete ISO C++ grammar. Each of the
following was checked against the built parser and is rejected:

- `static` declarations
- `typedef`, `using`, `enum`, or `union`
- variadic `...` parameters
- `until` loops
- multi-level pointer declarators such as `int **p`
- `sizeof` expressions, although the scanner still recognizes `sizeof` as a
  Phase 1 keyword token
- `<<=`, `>>=`, `&=`, `|=`, or `^=`
- special file-manipulation grammar or built-ins

- templates, namespaces, exceptions, STL syntax, operator overloading, or other
  unselected C++ features.

The scanner reads `static`, `typedef`, `enum`, `union`, and `until` as
identifiers, since none of them appears in the Phase 1 keyword table. That keeps
Phase 1 behavior unchanged, but their unsupported C++ roles do not parse as
valid syntax. Similarly, a name such as `fopen` may only appear through the
ordinary identifier and function-call grammar.

## Token categories

| Category | Examples or meaning |
| --- | --- |
| `keyword` | the 31 Phase 1 keyword entries |
| `identifier` | letter or underscore followed by letters, digits, or underscores |
| `integer_literal` | decimal, `0x1F`, `0755`, `0b1010` |
| `float_literal` | `3.14`, `.5`, `2.`, `1e10`, `1.5e-3` |
| `char_literal` | `'a'`, `'\n'`, `'\x41'` |
| `string_literal` | double-quoted text with the submitted escapes |
| `operator` | the submitted arithmetic, logical, bitwise, access, and assignment set, and also `?` and `:` |
| `punctuator` | `( ) { } [ ] ; ,` |

Line comments and non-nesting block comments are skipped.
