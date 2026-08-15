# CSC-305 Assignment 1 — Lexical Analyzer

Source language: C-like (C++ subset)
Implementation language: C++ with flex

## Build

    make

Produces `build/lexer`.

## Run

    ./build/lexer test/test1_control_flow.c    # one file
    ./run.sh                                   # all test cases
    ./run.sh path/to/lexer                     # or pass the executable in
    make test                                  # same as ./run.sh

If the file contains only valid tokens, a Lexeme/Token table is printed and
the exit status is 0. If it contains lexical errors, every error is printed
with its line and column and the exit status is 1.

## Layout

    src/lexer.l         flex specification (the analyzer, plus main)
    src/token_types.h   token IDs, category names, keyword table
    test/*.c            6 test cases
    Makefile
    run.sh

## Token categories

    keyword           30 reserved words, listed in token_types.h
    identifier        letter or underscore, then letters/digits/underscores
    integer_literal   decimal, hex (0x1F), octal (0755), binary (0b1010)
    float_literal     3.14, .5, 2., 1e10, 1.5e-3
    char_literal      'a', '\n', '\x41'
    string_literal    "...", with escapes
    operator          arithmetic, relational, logical, bitwise, ?:, ., ->, ::
    punctuation       ( ) { } [ ] ; ,

Comments (`//` and `/* */`, non-nesting) are skipped, not tokenized.

## Lexical errors detected

    illegal character
    malformed numeric literal          123abc, 1.2.3, 0x, 1e
    integer literal out of range
    empty character literal            ''
    multi-character literal            'ab'
    unterminated character literal
    invalid escape sequence            '\q', "\q"
    unterminated string literal        at newline and at end of file
    unterminated block comment

Scanning continues past every error, so one run reports all of them.

## Test coverage

    test1  if-else, for, while, do-while, switch is in test5, goto,
           break, continue, all operators, ternary
    test2  int and char arrays, multi-dimensional arrays, pointers,
           references, structures, . and -> access
    test3  function calls with arguments, recursion, function pointers,
           lambda, printf and scanf, argc/argv, new and delete
    test4  classes and objects, inheritance, public/private/protected,
           this, scope resolution
    test5  switch/case/default, every literal form
    test6  every class of lexical error

## Language notes

`a+++b` is read as `a ++ + b`, because flex always takes the longest match.

`<<=`, `>>=`, `&=`, `|=`, `^=` are not in the language; `<<=` is read as
`<<` followed by `=`.

`*` and `&` each produce one token. Whether a given `*` means multiply,
dereference or a pointer declarator is decided by the parser, not here.

`static`, `typedef`, `enum`, `union` and `sizeof` are not keywords in this
language, so they are read as plain identifiers.

`printf` and `scanf` are ordinary identifiers. They become built-in function
declarations during semantic analysis, not lexical analysis.

## Note for Assignment 2

The rules section of `lexer.l` is written so a bison parser can be dropped in
without touching it. Only two changes are needed:

1. In `addToken()`, also `return` the token id to the caller.
2. Delete `main()` and `printTable()`; bison's `yyparse()` calls `yylex()`.
