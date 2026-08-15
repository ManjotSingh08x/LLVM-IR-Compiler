#!/usr/bin/env bash
# run.sh — takes the lexer executable as input and runs it over every test
#          case in test/.
#
#   ./run.sh                    uses build/lexer
#   ./run.sh path/to/lexer      uses the executable you pass in

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

EXE="${1:-build/lexer}"
TEST_DIR="test"

if [ ! -x "$EXE" ]; then
    echo "Executable '$EXE' not found. Building..."
    make || exit 1
fi

clean=0
witherr=0
for f in "$TEST_DIR"/*.c; do
    echo "=== $(basename "$f") ==="
    if "$EXE" "$f"; then
        clean=$((clean+1))
    else
        witherr=$((witherr+1))
    fi
    echo
done

echo "-----------------------------------------"
echo "$((clean+witherr)) test file(s): $clean clean, $witherr with lexical errors."
