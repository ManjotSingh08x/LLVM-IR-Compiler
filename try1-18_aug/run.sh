#!/usr/bin/env bash
# run.sh — run the lexer executable over every test case in test/
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

EXE="build/lexer"
TEST_DIR="test"
RESULTS_DIR="test-results"

if [ ! -x "$EXE" ]; then
    echo "Building lexer..."
    make
fi

mkdir -p "$RESULTS_DIR"

shopt -s nullglob
files=("$TEST_DIR"/*.c "$TEST_DIR"/*.txt)
shopt -u nullglob

if [ ${#files[@]} -eq 0 ]; then
    echo "No test files found in $TEST_DIR/"
    exit 1
fi

pass=0
fail=0
for f in "${files[@]}"; do
    name="$(basename "$f")"
    out="$RESULTS_DIR/${name}.out"
    echo "=== $name ==="
    if "$EXE" "$f" > "$out" 2>&1; then
        status="OK"
        pass=$((pass+1))
    else
        status="LEXICAL ERROR(S)"
        fail=$((fail+1))
    fi
    echo "  -> $status  (full output: $out)"
    sed 's/^/    /' "$out"
    echo
done

echo "-----------------------------------------"
echo "Ran ${#files[@]} test file(s): $pass clean, $fail with lexical errors."
