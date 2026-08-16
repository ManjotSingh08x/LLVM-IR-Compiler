# CSC-305 Assignment 1 — Lexical Analyzer

Source language: C-like (C++ subset)
Implementation language: C++ with flex

## Build

    make

Produces `build/lexer`.

## Run

    ./build/lexer test/test1_control_flow.c    # one file
    ./run.sh                                   # all test cases
    make test                                  # same thing

Clean input → prints a Lexeme/Token table (exit 0).
Errors → prints every error with line:col (exit 1).

## Layout

    src/lexer.l         flex spec + main()
    src/token_types.h   token IDs, keyword table
    test/*.c            test cases
    Makefile
    run.sh

## Token categories

    keyword           30 reserved words (see token_types.h)
    identifier        starts with letter/underscore
    integer_literal   decimal, hex (0x1F), octal (0755), binary (0b1010)
    float_literal     3.14, .5, 2., 1e10, 1.5e-3
    char_literal      'a', '\n', '\x41'
    string_literal    "...", with escapes
    operator          arithmetic, relational, logical, bitwise, ?:, ., ->, ::
    punctuation       ( ) { } [ ] ; ,

Comments (`//` and `/* */`) are skipped.

## Errors detected

    illegal character
    malformed numeric literal          123abc, 1.2.3, 0x, 1e
    integer literal out of range
    empty character literal            ''
    multi-character literal            'ab'
    unterminated character literal
    invalid escape sequence            '\q', "\q"
    unterminated string literal
    unterminated block comment

All errors in one file are reported in a single run.

## Test coverage

    test1  control flow: if/else, for, while, do-while, goto, break, continue
    test2  arrays, pointers, structs, . and ->
    test3  functions, recursion, function pointers, lambdas, new/delete
    test4  classes, inheritance, access specifiers, this, ::
    test5  switch/case/default, every literal form
    test6  every kind of lexical error
    test7  everything together in one file
    test8  stuff we deliberately don't support

## Language notes

`a+++b` → `a ++ + b` (longest match).

`<<=`, `>>=`, `&=`, `|=`, `^=` aren't in the language.

`*` and `&` are single tokens — the parser figures out what they mean.

`static`, `typedef`, `enum`, `union` aren't keywords (just identifiers).
`sizeof` is a keyword.

`printf` and `scanf` are identifiers (not keywords).

## For Assignment 2

The lexer rules don't need to change. Just:
1. Add `return TOK_X;` after each `addToken()` call
2. Delete `main()` and `printTable()` — bison's `yyparse()` takes over
