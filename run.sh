set -u

# usage: bash run.sh [--levels=0,1,2,3] [--bless] [executable]
#   part 1 checks the front end on every test/*.ckp: parse or reject, .ll written and valid,
#          and for a test with test/expected/<name>.out, the .ll program's output
#   part 2 is the regression matrix: every test that parsed plus every test/regression/*.ckp is
#          compiled at each optimization level and checked (see the comment above part 2)
#   --levels  which of -O0..-O3 part 2 runs, default all four
#   --bless   writes test/regression/expected/<name>.out from the unoptimized .ll program and
#             rewrites test/regression/ll.sha256. review both before committing them

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

TEST_DIR="test"
OUT_DIR="outputs"
RUN_DIR="build/run"            # linked test programs, kept out of outputs/
EXPECTED_DIR="$TEST_DIR/expected"
INPUT_DIR="$TEST_DIR/inputs"
REG_DIR="$TEST_DIR/regression"
REG_EXPECTED_DIR="$REG_DIR/expected"
REG_INPUT_DIR="$REG_DIR/inputs"
MANIFEST="$REG_DIR/ll.sha256"
NODIFF="$REG_DIR/nodiff.txt"
DEFAULT_EXE="build/parser"
CLANG="${CLANG:-clang}"
FILECHECK="${FILECHECK:-FileCheck}"

LEVELS="0 1 2 3"
BLESS=0
EXE=""
for arg in "$@"; do
    case "$arg" in
        --levels=*) LEVELS="$(printf '%s' "${arg#--levels=}" | tr ',' ' ')" ;;
        --bless)    BLESS=1 ;;
        -h|--help)  sed -n '3,11p' "$0"; exit 0 ;;
        -*)         echo "unknown option $arg"; exit 1 ;;
        *)          EXE="$arg" ;;
    esac
done
for level in $LEVELS; do
    case "$level" in
        0|1|2|3) ;;
        *) echo "unknown level '$level' in --levels, use 0 to 3"; exit 1 ;;
    esac
done

if [ -n "$EXE" ]; then
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

# every .ckp file in a folder, whatever its name. files are listed in the order of the
# first number in their name, so test2 comes before test10; files without a
# number come last, in alphabetical order
list_ckp() {
    shopt -s nullglob
    for f in "$1"/*.ckp; do
        name="$(basename "$f")"
        number="$(printf '%s' "$name" | sed -nE 's/^[^0-9]*([0-9]+).*/\1/p')"
        [ -z "$number" ] && number=999999999
        printf '%d\t%s\t%s\n' "$((10#$number))" "$name" "$f"
    done | sort -t $'\t' -k1,1n -k2,2
}

files=()
while IFS=$'\t' read -r _ _ path; do
    files+=("$path")
done < <(list_ckp "$TEST_DIR")

if [ ${#files[@]} -eq 0 ]; then
    echo "No test files found in $TEST_DIR/"
    exit 1
fi

parsed=0
rejected=0
irInvalid=0
runPassed=0
runFailed=0
parsedFiles=()

echo "== front end: test/*.ckp"
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
    parsedFiles+=("$f")

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
frontEndOk=1
[ "$irInvalid" -eq 0 ] && [ "$runFailed" -eq 0 ] || frontEndOk=0

# part 2: the regression matrix
# for every program (each test/*.ckp that parsed, then each test/regression/*.ckp):
#   reference  the unoptimized <name>.ll from irgen, compiled by clang and run. its checksum must
#              match test/regression/ll.sha256, so irgen output never changes unnoticed
#   per level  outputs/<name>/O<n>/ holds what "parser -O<n> --verify-each --dump-callgraph" wrote
#     COMPILER FAIL   the compiler failed or --verify-each found broken ir after a pass
#     LL CHANGED      <name>.ll differs from the reference: passes must never touch it
#     OPT IR INVALID  <name>.opt.ll fails clang's ir verifier
#     LINK FAILED     <name>.s does not assemble or link
#     RUN MISMATCH    stdout or exit code differs from the expected output (test/expected,
#                     test/regression/expected, else the reference run), HANG on a timeout.
#                     programs listed in test/regression/nodiff.txt are only linked
#     CHECK FAILED    the FileCheck lines in the source do not match (see check_family)
echo
echo "== regression matrix: levels $LEVELS"
if [ -z "$CLANG" ]; then
    echo "skipped: clang is needed to build and run the programs"
    [ "$frontEndOk" -eq 1 ]
    exit
fi

TIMEOUT=""
if command -v timeout > /dev/null 2>&1; then
    TIMEOUT="timeout"
elif command -v gtimeout > /dev/null 2>&1; then
    TIMEOUT="gtimeout"
else
    echo "warning: no timeout command, a program that hangs will stop the run"
fi
if ! command -v "$FILECHECK" > /dev/null 2>&1; then
    echo "warning: '$FILECHECK' not found, the FileCheck lines in the sources are not checked"
    FILECHECK=""
fi

checksum() {
    if command -v sha256sum > /dev/null 2>&1; then
        sha256sum "$1" | cut -d' ' -f1
    else
        shasum -a 256 "$1" | cut -d' ' -f1
    fi
}

# runs a program with a time limit, writes its stdout and stderr to $3, prints its exit code
# every program is run as ./<name> from its own folder, so argv[0] is the same at every level
run_program() {
    local program="$1" input="$2" output="$3"
    case "$input" in /*) ;; *) input="$SCRIPT_DIR/$input" ;; esac
    case "$output" in /*) ;; *) output="$SCRIPT_DIR/$output" ;; esac
    (
        cd "$(dirname "$program")" || exit 1
        if [ -n "$TIMEOUT" ]; then
            "$TIMEOUT" 10 "./$(basename "$program")" < "$input" > "$output" 2>&1
        else
            "./$(basename "$program")" < "$input" > "$output" 2>&1
        fi
    )
    echo $?
}

# checks one family of FileCheck lines at one level. at level n the prefixes are
#   F       every level
#   F-O<n>  exactly level n
#   F-GE<k> level k and above, for every k <= n
# a source with no line for these prefixes has nothing to check
check_family() {
    local family="$1" level="$2" input="$3" source="$4" log="$5"
    local prefixes="$family,$family-O$level" k
    for k in $(seq 0 "$level"); do prefixes="$prefixes,$family-GE$k"; done
    grep -qE "//[[:space:]]*$family(-O$level|-GE[0-$level])?(-[A-Z]+)?:" "$source" || return 0
    [ -f "$input" ] || { echo "no $input to check" > "$log"; return 1; }
    "$FILECHECK" --allow-unused-prefixes --check-prefixes="$prefixes" --input-file="$input" "$source" \
        > "$log" 2>&1
}

nodiff() {
    [ -f "$NODIFF" ] && grep -qxF "$1" "$NODIFF"
}

programs=("${parsedFiles[@]}")
while IFS=$'\t' read -r _ _ path; do
    programs+=("$path")
done < <(list_ckp "$REG_DIR")

declare -A manifest
if [ -f "$MANIFEST" ]; then
    while read -r sum name; do
        [ -n "$sum" ] && manifest["$name"]="$sum"
    done < "$MANIFEST"
fi
newManifest=()
blessed=()

declare -A passCount
for level in $LEVELS; do passCount[$level]=0; done
failures=0
programCount=0
STATS="$OUT_DIR/stats.txt"
printf '%-34s %5s %7s %7s %6s\n' "program" "level" "instrs" "stack" "calls" > "$STATS"
declare -A statInstr statStack statCalls
for level in $LEVELS; do statInstr[$level]=0; statStack[$level]=0; statCalls[$level]=0; done

fail() {
    failures=$((failures + 1))
    printf '%-15s %-34s %s\n' "$1" "$2" "$3"
}

for src in "${programs[@]}"; do
    base="$(basename "$src" .ckp)"
    dir="$OUT_DIR/$base"
    mkdir -p "$dir"
    programCount=$((programCount + 1))
    isRegression=0
    case "$src" in "$REG_DIR"/*) isRegression=1 ;; esac

    # reference: the unoptimized .ll, built by clang
    if ! "$EXE" "$src" > "$dir/$base.reference.txt" 2>&1 || [ ! -f "$dir/$base.ll" ]; then
        fail "COMPILER FAIL" "$base" "reference compile failed, see $dir/$base.reference.txt"
        continue
    fi
    referenceSum="$(checksum "$dir/$base.ll")"
    newManifest+=("$referenceSum $base.ll")
    if [ "$BLESS" -eq 0 ]; then
        if [ -z "${manifest[$base.ll]:-}" ]; then
            fail "NOT BLESSED" "$base" "no checksum in $MANIFEST, run with --bless"
        elif [ "${manifest[$base.ll]}" != "$referenceSum" ]; then
            fail "LL CHANGED" "$base" "irgen output differs from $MANIFEST"
        fi
    fi

    input="/dev/null"
    [ -f "$INPUT_DIR/$base.in" ] && input="$INPUT_DIR/$base.in"
    [ -f "$REG_INPUT_DIR/$base.in" ] && input="$REG_INPUT_DIR/$base.in"

    mkdir -p "$RUN_DIR/reference"
    reference="$RUN_DIR/reference/$base"
    if ! "$CLANG" -Wno-override-module "$dir/$base.ll" -o "$reference" > "$dir/$base.reference.link.txt" 2>&1; then
        fail "LINK FAILED" "$base" "the reference .ll does not link, see $dir/$base.reference.link.txt"
        continue
    fi
    referenceExit="$(run_program "$reference" "$input" "$dir/$base.reference.out")"

    expected=""
    if [ -f "$EXPECTED_DIR/$base.out" ]; then
        expected="$EXPECTED_DIR/$base.out"
    elif [ "$isRegression" -eq 1 ]; then
        expected="$REG_EXPECTED_DIR/$base.out"
        if [ "$BLESS" -eq 1 ] && ! cmp -s "$dir/$base.reference.out" "$expected" 2>/dev/null; then
            cp "$dir/$base.reference.out" "$expected"
            blessed+=("$expected")
        elif [ ! -f "$expected" ]; then
            fail "NOT BLESSED" "$base" "no $expected, run with --bless"
            expected=""
        fi
    else
        expected="$dir/$base.reference.out"
    fi

    for level in $LEVELS; do
        ldir="$dir/O$level"
        rm -rf "$ldir"
        mkdir -p "$ldir"
        rm -f "$dir/$base.opt.ll" "$dir/$base.s" "$dir/$base.callgraph.txt"
        what="$base -O$level"

        compileCmd=("$EXE" "-O$level" --verify-each --dump-callgraph "$src")
        if [ -n "$TIMEOUT" ]; then compileCmd=("$TIMEOUT" 120 "${compileCmd[@]}"); fi
        if ! "${compileCmd[@]}" > "$ldir/compile.txt" 2>&1; then
            fail "COMPILER FAIL" "$what" "$(grep -m1 -E 'verifier error|codegen error|error' "$ldir/compile.txt" || echo "exit status, see $ldir/compile.txt")"
            continue
        fi
        for suffix in .opt.ll .s .callgraph.txt; do
            [ -f "$dir/$base$suffix" ] && mv "$dir/$base$suffix" "$ldir/"
        done
        ok=1

        if [ "$(checksum "$dir/$base.ll")" != "$referenceSum" ]; then
            fail "LL CHANGED" "$what" "$base.ll differs from the reference written at -O0"
            ok=0
        fi

        if ! "$CLANG" -x ir -c -fverify-intermediate-code -Wno-override-module "$ldir/$base.opt.ll" \
                -o /dev/null > "$ldir/verify.txt" 2>&1; then
            fail "OPT IR INVALID" "$what" "see $ldir/verify.txt"
            ok=0
        fi

        mkdir -p "$RUN_DIR/O$level"
        program="$RUN_DIR/O$level/$base"
        if ! "$CLANG" "$ldir/$base.s" -o "$program" > "$ldir/link.txt" 2>&1; then
            fail "LINK FAILED" "$what" "see $ldir/link.txt"
            continue
        fi

        if ! nodiff "$base" && [ -n "$expected" ]; then
            exitCode="$(run_program "$program" "$input" "$ldir/run.txt")"
            if [ "$exitCode" = "124" ] && [ -n "$TIMEOUT" ]; then
                fail "HANG" "$what" "no exit within 10 seconds"
                ok=0
            elif ! diff -u "$expected" "$ldir/run.txt" > "$ldir/diff.txt"; then
                fail "RUN MISMATCH" "$what" "output differs, see $ldir/diff.txt"
                ok=0
            elif [ "$exitCode" != "$referenceExit" ]; then
                fail "RUN MISMATCH" "$what" "exit code $exitCode, expected $referenceExit"
                ok=0
            else
                rm -f "$ldir/diff.txt"
            fi
        fi

        if [ -n "$FILECHECK" ]; then
            check_family IR "$level" "$ldir/$base.opt.ll" "$src" "$ldir/check-ir.txt" \
                || { fail "CHECK FAILED" "$what" "IR lines, see $ldir/check-ir.txt"; ok=0; }
            check_family ASM "$level" "$ldir/$base.s" "$src" "$ldir/check-asm.txt" \
                || { fail "CHECK FAILED" "$what" "ASM lines, see $ldir/check-asm.txt"; ok=0; }
            check_family CG "$level" "$ldir/$base.callgraph.txt" "$src" "$ldir/check-cg.txt" \
                || { fail "CHECK FAILED" "$what" "CG lines, see $ldir/check-cg.txt"; ok=0; }
        fi

        # statistics, for information only
        instrs="$(grep -cE $'^\t[a-z]' "$ldir/$base.s")"
        stack="$(grep -o '(%rbp)' "$ldir/$base.s" | wc -l | tr -d ' ')"
        calls="$(grep -cE $'^\tcall' "$ldir/$base.s")"
        printf '%-34s %5s %7s %7s %6s\n' "$base" "O$level" "$instrs" "$stack" "$calls" >> "$STATS"
        statInstr[$level]=$((statInstr[$level] + instrs))
        statStack[$level]=$((statStack[$level] + stack))
        statCalls[$level]=$((statCalls[$level] + calls))

        [ "$ok" -eq 1 ] && passCount[$level]=$((passCount[$level] + 1))
    done
done

if [ "$BLESS" -eq 1 ]; then
    printf '%s\n' "${newManifest[@]}" | sort -k2 > "$MANIFEST"
    echo "blessed: wrote $MANIFEST (${#newManifest[@]} checksums)"
    for file in "${blessed[@]}"; do echo "blessed: wrote $file"; done
    echo "review these files before committing them"
fi

echo
echo "$programCount program(s) at levels $LEVELS"
printf '%-6s %8s %10s %10s %8s\n' "level" "passed" "instrs" "stack" "calls"
for level in $LEVELS; do
    printf '%-6s %4s/%-3s %10s %10s %8s\n' "-O$level" "${passCount[$level]}" "$programCount" \
        "${statInstr[$level]}" "${statStack[$level]}" "${statCalls[$level]}"
done
echo "per-program statistics: $STATS, per-level files: $OUT_DIR/<name>/O<n>/"
if [ "$failures" -eq 0 ]; then
    echo "regression matrix: all checks passed"
else
    echo "regression matrix: $failures failure(s)"
fi

[ "$frontEndOk" -eq 1 ] && [ "$failures" -eq 0 ]
