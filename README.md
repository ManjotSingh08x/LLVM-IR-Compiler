# LLVM-IR-Compiler

A compiler for a subset of C++, written for CSC-305. It turns a `.ckp` source file into LLVM IR. It can then optimize the IR at four levels (`-O0` to `-O3`) and generate x86-64 assembly that `gcc` or `clang` can link.

The compiler is written in C++17. It uses Flex and Bison, and links only LLVM's Support library. The optimizations are documented pass by pass in [reference/optimizations.md](reference/optimizations.md).

## Commands

You need `flex`, `bison`, `g++`, `llvm-config` and `clang`.

```bash
make                          # build the compiler: build/parser
make test                     # build, then run the full test suite at -O0, -O1, -O2 and -O3
bash run.sh --levels=2        # run the full suite at one level only
bash run.sh --bless           # rewrite the expected outputs (review the diff before committing)
make clean                    # delete build/ and outputs/
```

To compile one file:

```bash
./build/parser -O2 file.ckp                   # writes outputs/file/file.ll, file.opt.ll and file.s
gcc outputs/file/file.s -o file && ./file     # link and run the generated program
```

To debug the optimizer:

```bash
./build/parser -O2 --verify-each --trace-passes --dump-callgraph file.ckp
```

- `--verify-each` checks the IR after every pass, and reports the pass that broke it.
- `--trace-passes` writes the IR after each pass to `file.passes.ll`.
- `--dump-callgraph` writes `file.callgraph.txt`.

## Architecture

```text
  file.ckp
     │
     ▼
┌───────────────┐  tokens
│ Lexer (flex)  │────────────┐
└───────────────┘            ▼
                    ┌─────────────────┐
                    │ Parser (bison)  │  builds the AST
                    └────────┬────────┘
                             ▼
                    ┌─────────────────┐   declares names    ┌───────────────────────┐
                    │ Semantic        │────────────────────►│ Symbol table          │
                    │ 1. SymbolBuilder│◄────────────────────│ scope tree: globals,  │
                    │ 2. TypeChecker  │   looks up names    │ classes, functions,   │
                    └────────┬────────┘                     │ blocks, lambdas       │
                             │ AST with types, implicit     └───────────┬───────────┘
                             │ casts and resolved symbols               │ mangled names, types,
                             ▼                                          │ record layouts,
                    ┌─────────────────┐◄────────────────────────────────┘ lambda captures
                    │ IR generation   │  CFG per function  ──►  file.ll
                    └────────┬────────┘
                             ▼
                    ┌─────────────────┐  -O0  simplify_cfg, loop_rotate
                    │ Optimizer       │  -O1  + inline, local DAG, constant propagation
                    │ (pass managers) │  -O2  + SSA: mem2reg, sccp, licm, unswitch, cse, dce, sink
                    └────────┬────────┘  -O3  + induction-variable strength reduction ──► file.opt.ll
                             ▼
                    ┌─────────────────┐  instruction selection → register allocation
                    │ x86-64 codegen  │  (-O0..-O2: getReg, -O3: graph colouring)
                    └────────┬────────┘  → peephole (-O1+)  ──►  file.s
                             ▼
                      gcc / clang link  ──►  executable
```

Semantic analysis builds the symbol table, a tree of scopes, and keeps it. Every AST node points into the tree. IR generation does not look names up again: it reads the resolved symbols, and takes from them the mangled function names, the LLVM types, record layouts and lambda captures.

## Limitations

- **Types:**
  - not supported: `short`, `long`, `unsigned`, `const`, `enum`, `union`;
  - not supported: number suffixes such as `10L`, `1u`, `1.5f`.
- **Expressions:**
  - not supported: explicit casts, `sizeof`, the comma operator;
  - not supported: `<<=`, `>>=`, `&=`, `|=`, `^=`;
  - `::` is allowed only in declarations, not in expressions.
- **C++ features not supported:**
  - templates, namespaces, exceptions;
  - virtual functions, operator overloading, constructor initialiser lists;
  - default arguments;
  - user-defined variadic functions;
  - functions returning references.
- **Libraries:** only `printf` and `scanf` are built in. Other functions are name-mangled, so the compiler cannot call C library functions or link with separately compiled C code.
- **Code generation:**
  - x86-64 Linux (System V ABI) only;
  - structs passed to or returned from functions by value are not supported. Use pointers or references.
- **Not implemented in the optimizer:**
  - partial-redundancy elimination;
  - loop unrolling, fusion and fission;
  - instruction scheduling.

## Members

- Adityavardhan Singh (24322005)
- Garima Sidana (24113043)
- Manjot Singh (24116048)
- Naivadhya Garg (24115106)
