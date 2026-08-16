#!/usr/bin/env bash
# run.sh — takes the lexer executable as input and runs it over every test
#          case in test/.  Full output goes to outputs/.  Only errors are
#          shown in the terminal.
#
#   ./run.sh                    uses build/lexer
#   ./run.sh path/to/lexer      uses the executable you pass in

# set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

EXE="${1:-build/lexer}"
TEST_DIR="test"
OUT_DIR="outputs"

if [ ! -x "$EXE" ]; then
    echo "Executable '$EXE' not found. Building"
    make || exit 1
fi

mkdir -p "$OUT_DIR"

clean=0
witherr=0
for f in "$TEST_DIR"/*.c; do
    base="$(basename "$f" .c)"
    outfile="$OUT_DIR/${base}.txt"

    if "$EXE" "$f" > "$outfile" 2>&1; then
        clean=$((clean+1))
    else
        witherr=$((witherr+1))
        echo "=== $(basename "$f") ==="
        cat "$outfile"
        echo
    fi
done
echo "$((clean+witherr)) test file(s): $clean clean, $witherr with lexical errors."
echo "Output files saved to $OUT_DIR/"

