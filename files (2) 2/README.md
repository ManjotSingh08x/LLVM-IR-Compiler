# CSC-305 Assignment 1: Lexical Analyzer

Team members:

- Adityavardhan Singh (24322005)
- Garima Sidana (24113043)
- Manjot Singh (24116048)
- Naivadhya Garg (24115106)

Source language: C-like (C++ subset)

Implementation language: C++ with lex

## Build

```bash
make
```

This produces `build/lexer`.

## Run

```bash
./build/lexer test/test1_control_flow.c    # one file
./run.sh                                   # all test cases
./run.sh path/to/lexer                     # or pass the executable in
make test                                  # same as ./run.sh
```

If the file contains only valid tokens, a Lexeme/Token table is printed and
the exit status is 0. If it contains lexical errors, every error is printed
with its line and column and the exit status is 1.

## Layout

```
src/lexer.l
src/token_types.h
test/*.c
Makefile
run.sh
```

after running make or run.sh, `build/lexer` and `build/lex.yy.cpp` are also generated.

## Token categories

| Category        | Description                                             |
| --------------- | ------------------------------------------------------- |
| keyword         | 30 reserved words, listed in token_types.h              |
| identifier      | letter or underscore, then letters/digits/underscores   |
| integer_literal | decimal, hex (0x1F), octal (0755), binary (0b1010)      |
| float_literal   | 3.14, .5, 2., 1e10, 1.5e-3                              |
| char_literal    | 'a', '\n', '\x41'                                       |
| string_literal  | "...", with escapes                                     |
| operator        | arithmetic, relational, logical, bitwise, ?:, ., ->, :: |
| punctuation     | ( ) { } [ ] ; ,                                         |

Line comments `//` and non-nesting block comments `/* */` are recognized but they are directly skipped.

## Lexical errors detected

| Error                          | Examples/Notes                |
| ------------------------------ | ----------------------------- |
| illegal character              | $@                            |
| malformed numeric literal      | 123abc, 1.2.3, 0x, 1e         |
| integer literal out of range   | 99...9999                     |
| empty character literal        | ''                            |
| multi-character literal        | 'ab'                          |
| unterminated character literal | 'a                            |
| invalid escape sequence        | '\q', "\q"                    |
| unterminated string literal    | at newline and at end of file |
| unterminated block comment     |                               |

Scanning continues past every error, so one run reports all of them.

## Test coverage

- **test1**: `if-else`, `for`, `while`, `do-while`, `goto`, `break`, `continue`, all operators, and the ternary operator. (Note: `switch` is in test5).
- **test2**: `int` and `char` arrays, multi-dimensional arrays, pointers, references, structures, `.` and `->` access.
- **test3**: Function calls with arguments, recursion, function pointers, lambdas, `printf` and `scanf`, `argc`/`argv`, `new` and `delete`.
- **test4**: Classes and objects, inheritance, `public`/`private`/`protected`, `this`, and scope resolution.
- **test5**: `switch`/`case`/`default`, and every literal form.
- **test6**: Every class of lexical error.
- **test7**: Contains

## Language notes

- `a+++b` is read as `a ++ + b`, because flex always takes the longest match.

- `<<=`, `>>=`, `&=`, `|=`, `^=` are not in the language; `<<=` is read as
  `<<` followed by `=`.

- `*` and `&` each produce one token. Whether a given `*` means multiply, dereference or a pointer declarator is decided by the parser, not here.

- `static`, `typedef`, `enum`, `union` are not keywords in this language, so they are read as plain identifiers. `sizeof` is a keyword.

- `printf` and `scanf` are ordinary identifiers. They become built-in function declarations during semantic analysis, not lexical analysis.
