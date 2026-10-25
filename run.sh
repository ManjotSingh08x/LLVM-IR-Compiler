
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

# start from empty output folders, so nothing is left over from an earlier run
# or from a test file that has since been removed
rm -rf "$OUT_DIR" "$RUN_DIR"
mkdir -p "$OUT_DIR" "$RUN_DIR"

if ! command -v "$CLANG" > /dev/null 2>&1; then
    echo "warning: '$CLANG' not found, generated IR will not be checked"
    CLANG=""
fi

# every .ckp file in test/, whatever its name. files are run in the order of the
# first number in their name, so test2 comes before test10; files without a
# number come last, in alphabetical order
files=()
while IFS=$'\t' read -r _ _ path; do
    files+=("$path")
done < <(
    shopt -s nullglob
    for f in "$TEST_DIR"/*.ckp; do
        name="$(basename "$f")"
        number="$(printf '%s' "$name" | sed -nE 's/^[^0-9]*([0-9]+).*/\1/p')"
        [ -z "$number" ] && number=999999999
        printf '%d\t%s\t%s\n' "$((10#$number))" "$name" "$f"
    done | sort -t $'\t' -k1,1n -k2,2
)

if [ ${#files[@]} -eq 0 ]; then
    echo "No test files found in $TEST_DIR/"
    exit 1
fi

parsed=0
rejected=0
irInvalid=0
runPassed=0
runFailed=0

for f in "${files[@]}"; do
    base="$(basename "$f" .ckp)"
    dir="$OUT_DIR/$base"

    # one folder per test, with a copy of the source file for reference
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

    # -c with the verifier on: -fsyntax-only cannot read IR at all, and release
    # builds of clang skip the verifier unless asked for it
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
