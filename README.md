# C++ Compiler - Lexical Analyzer

This repository contains the lexical analyzer component for a C++ compiler.

## Project Structure

- `build/` - Directory for out-of-source builds.
- `include/` - Header files.
- `src/` - Source files (including `.l` lex files and `.cpp` files).
- `tests/` - CTest based tests.

## Building the Project

```bash
cd build
cmake ..
make
```

## Running Tests

```bash
cd build
ctest
```

## Work TODO

Regex writing: vardhan, garima
Lex structure and parsing input/output : manjot
testcases and error handling: naivadhya
