
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

TEST_DIR="test"
OUT_DIR="outputs"
RUN_DIR="build/run"            # linked test programs, kept out of outputs/
EXPECTED_DIR="$TEST_DIR/expected"
INPUT_DIR="$TEST_DIR/inputs"
DEFAULT_EXE="build/parser"
CLANG="${CLANG:-clang}"

if [ $# -ge 1 ]; then
    EXE="$1"
    if [ ! -x "$EXE" ]; then
        echo "Executable '$EXE' not found or not executable."
        exit 1
    fi
else
    EXE="$DEFAULT_EXE"
    echo "Building..."
    if ! make all; then
        echo "Build failed."
        exit 1
    fi
    echo
fi

mkdir -p "$OUT_DIR" "$RUN_DIR"

if ! command -v "$CLANG" > /dev/null 2>&1; then
    echo "warning: '$CLANG' not found, generated IR will not be checked"
    CLANG=""
fi

shopt -s nullglob
files=("$TEST_DIR"/*.ckp)
shopt -u nullglob

if [ ${#files[@]} -eq 0 ]; then
    echo "No test files found in $TEST_DIR/"
    exit 1
fi


IFS=$'\n' files=($(printf '%s\n' "${files[@]}" \
    | sed -E 's|.*/test([0-9]+)_|\1 &|' \
    | sort -n -k1,1 \
    | cut -d' ' -f2-))
unset IFS

parsed=0
rejected=0
irInvalid=0
runPassed=0
runFailed=0

for f in "${files[@]}"; do
    base="$(basename "$f" .ckp)"
    dir="$OUT_DIR/$base"

    # a fresh folder per test, so a stale .ll cannot pass
    rm -rf "$dir"
    mkdir -p "$dir"
    cp "$f" "$dir/$base.ckp"
    outfile="$dir/$base.txt"

    if ! "$EXE" "$f" > "$outfile" 2>&1; then
        rejected=$((rejected + 1))
        count="$(grep -cE 'line: [0-9]+ col: [0-9]+' "$outfile")"
        if [ "$count" -gt 0 ]; then
            printf 'REJECTED %-36s %s error(s)\n' "$(basename "$f")" "$count"
        else
            printf 'REJECTED %-36s\n' "$(basename "$f")"
        fi
        sed 's/^/         /' "$outfile"
        if [ -f "$dir/$base.ll" ]; then
            echo "         error: a rejected file must not produce $dir/$base.ll"
        fi
        continue
    fi
    parsed=$((parsed + 1))

    ll="$dir/$base.ll"
    if [ ! -f "$ll" ]; then
        irInvalid=$((irInvalid + 1))
        printf 'IR MISSING %-34s no %s was written\n' "$(basename "$f")" "$ll"
        continue
    fi
    if [ -z "$CLANG" ]; then
        printf 'PARSED   %-36s IR written (not checked)\n' "$(basename "$f")"
        continue
    fi

    # -c with the verifier on, release clang skips it otherwise
    if ! "$CLANG" -x ir -c -fverify-intermediate-code -Wno-override-module "$ll" -o /dev/null \
            > "$dir/$base.verify.txt" 2>&1; then
        irInvalid=$((irInvalid + 1))
        printf 'IR INVALID %-34s\n' "$(basename "$f")"
        sed 's/^/         /' "$dir/$base.verify.txt"
        continue
    fi
    rm -f "$dir/$base.verify.txt"

    # a test with an expected output is also linked, run and compared
    expected="$EXPECTED_DIR/$base.out"
    if [ ! -f "$expected" ]; then
        printf 'PARSED & IR VALID %-27s\n' "$(basename "$f")"
        continue
    fi
    input="/dev/null"
    [ -f "$INPUT_DIR/$base.in" ] && input="$INPUT_DIR/$base.in"
    program="$RUN_DIR/$base"
    if ! "$CLANG" -Wno-override-module "$ll" -o "$program" > "$dir/$base.link.txt" 2>&1; then
        runFailed=$((runFailed + 1))
        printf 'LINK FAILED %-33s\n' "$(basename "$f")"
        sed 's/^/         /' "$dir/$base.link.txt"
        continue
    fi
    rm -f "$dir/$base.link.txt"
    "$program" < "$input" > "$dir/$base.run.txt" 2>&1
    if diff -u "$expected" "$dir/$base.run.txt" > "$dir/$base.diff.txt"; then
        runPassed=$((runPassed + 1))
        rm -f "$dir/$base.diff.txt"
        printf 'PARSED & IR VALID %-27s output matches\n' "$(basename "$f")"
    else
        runFailed=$((runFailed + 1))
        printf 'OUTPUT DIFFERS %-30s see %s\n' "$(basename "$f")" "$dir/$base.diff.txt"
    fi
done

echo
echo "$((parsed + rejected)) test file(s) run: $parsed parsed, $rejected rejected."
if [ -n "$CLANG" ]; then
    echo "IR: $((parsed - irInvalid)) valid, $irInvalid invalid or missing."
    echo "Programs run: $runPassed matched the expected output, $runFailed did not."
fi
echo "Full output for each file is in $OUT_DIR/<name>/"

[ "$irInvalid" -eq 0 ] && [ "$runFailed" -eq 0 ]
