# Assignment 1 — Lexical Analyzer (CSC-305)

Implementation language: C++ (flex, compiled with g++). Source language: C-like
(per project form — full feature checklist below).

## Layout
```
src/
  lexer.l          flex spec — rules only, no grammar logic
  token_types.h     single source of truth: TokenID enum + keyword table + category mapper
test/               6 input programs (5 clean + 1 with deliberate lexical errors)
Makefile            flex -> lex.yy.cpp -> g++ -> build/lexer
run.sh              builds if needed, runs build/lexer over every test/*, saves to test-results/
```

## Build & run
```
make        # builds build/lexer
make test   # same as ./run.sh
./build/lexer path/to/file.c   # run on one file directly
```

## Why it's structured this way (don't undo this on the next assignment)
- **`token_types.h` is the only place token IDs/keywords live.** `lexer.l` reads
  `keywordTable()` instead of hardcoding keyword strings, and prints via
  `tokenCategory()`. Adding a keyword/operator = one line in this file, not a
  rules rewrite.
- **`lexer.l` never touches grammar.** It just classifies lexemes and returns
  `TokenID`s. This means the exact same `.l` file plugs into Assignment 2's
  bison parser later — swap out `main()`'s driver loop for bison calling
  `yylex()`, done. Do not fold parsing logic into the lexer actions.
- **Errors are collected, not fatal.** `.` (catch-all rule) records unmatched
  chars and keeps scanning, so one bad character doesn't stop the run —
  matches the "report all lexical errors" requirement and will matter again
  for parser error recovery.

## Feature coverage (per the project's checklist form)
Covered by test cases: arithmetic/logical/relational/bitwise operators,
if-else, for/while/do-while, switch-case, arrays (incl. multi-dimensional),
pointers, structs, printf/scanf, function calls with arguments, goto/break/
continue, recursive functions, class/object, inheritance, public/private/
protected, dynamic memory (new/delete), lambda, function pointers, command
line input, references.
Deliberately NOT tokenized (per the unchecked boxes in the form): `static`,
`typedef`, `enum`/`union`, file I/O, `until` loop, multi-level pointers,
variadic function calls.

## Next assignments — where this plugs in
- **Parser (bison, Assignment 2):** reuse `lexer.l` + `token_types.h` as-is;
  generate a `.y` file whose `%token` list mirrors the `TokenID` enum.
- **IR generation (LLVM IR, per the project's chosen intermediate rep):**
  parser output (AST) feeds an LLVM `IRBuilder`-based emitter — token/lexeme
  layer here is unaffected.
- **Codegen (x86 target):** downstream of LLVM IR, via `llc`/LLVM backend —
  no change needed to this stage either.

## References
- Flex manual: https://westes.github.io/flex/manual/
- GNU Bison manual (for Assignment 2): https://www.gnu.org/software/bison/manual/
- LLVM IR reference (for the codegen stage): https://llvm.org/docs/LangRef.html
- LLVM "Kaleidoscope" tutorial (AST -> LLVM IR -> x86, close match to this
  project's LLVM IR / X86 choices): https://llvm.org/docs/tutorial/
- Aho, Lam, Sethi, Ullman, *Compilers: Principles, Techniques, and Tools*
  ("Dragon Book") — ch. 3 for lexer theory, directly relevant to this stage.
