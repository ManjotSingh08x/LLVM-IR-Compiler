#!/usr/bin/env bash
# usage: ./run_tests.sh [target ...] [-o0] [-o1] [-o2] [-o3]
#
#   ./run_tests.sh                    every test under test/ (what make test runs)
#   ./run_tests.sh test/test_ir       every test in a folder, recursively
#   ./run_tests.sh path/to/exam.cpp   one file, with any extension, from anywhere
#   ./run_tests.sh <target> -o2 -o3   only those levels (-O2 works too), all of 0-3 without a flag
#
# a test's stdin goes in test/inputs/<stem>.in, its expected output in test/expected/<stem>.out

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXE="$ROOT/build/parser"
CLANG="${CLANG:-clang}"
FILECHECK="${FILECHECK:-FileCheck}"

usage() { sed -n '2,9p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; }

levels=""
targets=()
for arg in "$@"; do
    case "$arg" in
        -o[0-3]|-O[0-3]) case " $levels " in *" ${arg:2} "*) ;; *) levels="$levels ${arg:2}" ;; esac ;;
        -h|--help)       usage; exit 0 ;;
        -*)              echo "unknown option $arg, see $0 -h"; exit 1 ;;
        *)               [ -e "$arg" ] || { echo "no such file or folder: $arg"; exit 1; }
                         targets+=("$(cd "$(dirname "$arg")" && pwd)/$(basename "$arg")") ;;
    esac
done
LEVELS="$(printf '%s\n' $levels | sort -n | paste -sd' ')"
[ -n "$levels" ] || LEVELS="0 1 2 3"
fullRun=0
[ ${#targets[@]} -gt 0 ] || { targets=("$ROOT/test"); fullRun=1; }

[ -x "$EXE" ] || { echo "build/parser not found, run make first"; exit 1; }
command -v "$CLANG" > /dev/null 2>&1 || { echo "'$CLANG' not found, it is needed to run the tests"; exit 1; }
if ! command -v "$FILECHECK" > /dev/null 2>&1; then
    echo "warning: '$FILECHECK' not found, the FileCheck lines in the tests are not checked"
    FILECHECK=""
fi
TIMEOUT=""
command -v timeout > /dev/null 2>&1 && TIMEOUT="timeout 10"

cd "$ROOT"

# every test file under the targets, ordered by the number in its name, then by path
mapfile -t tests < <(
    for target in "${targets[@]}"; do
        find "$target" \( -name expected -o -name inputs -o -name '.*' \) -prune -o -type f -print
    done | sort -u | while IFS= read -r f; do
        n="$(basename "$f" | sed -nE 's/^test([0-9]+)_.*/\1/p')"
        printf '%s\t%s\n' "${n:-999999}" "$f"
    done | sort -t $'\t' -k1,1n -k2,2 | cut -f2-)
[ ${#tests[@]} -gt 0 ] || { echo "no test files found"; exit 1; }

# two tests with one stem would write to the same outputs/<stem>/
duplicates="$(for f in "${tests[@]}"; do b="$(basename "$f")"; echo "${b%.*}"; done | sort | uniq -d)"
if [ -n "$duplicates" ]; then
    echo "these test names are used more than once, rename one of each:"
    printf '    %s\n' $duplicates
    exit 1
fi

if [ "$fullRun" -eq 1 ]; then rm -rf outputs; fi
mkdir -p outputs

# runs program < input in its own folder, writes stdout and stderr to output, prints the exit code
run_program() {
    local program="$1" input="$2" output="$3"
    ( cd "$(dirname "$program")" && $TIMEOUT "./$(basename "$program")" < "$input" > "$output" 2>&1 )
    echo $?
}

# checks one family of FileCheck lines (IR, ASM or CG) with the prefixes that apply at one level
check_lines() {
    local family="$1" level="$2" file="$3" src="$4" log="$5"
    local prefixes="$family,$family-O$level" k
    for k in $(seq 0 "$level"); do prefixes="$prefixes,$family-GE$k"; done
    grep -qE "//[[:space:]]*$family(-O$level|-GE[0-$level])?(-[A-Z]+)?:" "$src" || return 0
    "$FILECHECK" --allow-unused-prefixes --check-prefixes="$prefixes" --input-file="$file" "$src" > "$log" 2>&1
}

passed=0
rejected=0
failed=0

run_test() {
    local src="${1#$ROOT/}" shown="${1#$ROOT/}"
    local file stem dir input expected referenceExit level ldir exitCode
    file="$(basename "$src")"
    stem="${file%.*}"
    dir="$ROOT/outputs/$stem"
    input=/dev/null
    [ -f "test/inputs/$stem.in" ] && input="$ROOT/test/inputs/$stem.in"
    local errors=()

    rm -rf "$dir"
    mkdir -p "$dir"

    # front end: a rejected file stops here
    if ! "$EXE" "$src" > "$dir/compile.txt" 2>&1; then
        rejected=$((rejected + 1))
        printf 'REJECTED %s\n' "$shown"
        sed 's/^/    /' "$dir/compile.txt"
        return
    fi

    # reference: the unoptimized .ll compiled by clang
    mkdir -p "$dir/reference"
    if ! "$CLANG" -Wno-override-module "$dir/$stem.ll" -o "$dir/reference/$stem" > "$dir/reference.txt" 2>&1; then
        errors+=("$stem.ll does not compile with clang, see $dir/reference.txt")
    else
        referenceExit="$(run_program "$dir/reference/$stem" "$input" "$dir/reference.out")"
        expected="$dir/reference.out"
        if [ -f "test/expected/$stem.out" ]; then
            expected="$ROOT/test/expected/$stem.out"
            diff -u "$expected" "$dir/reference.out" > "$dir/reference.diff" \
                || errors+=("unoptimized output differs, see $dir/reference.diff")
        fi

        for level in $LEVELS; do
            ldir="$dir/O$level"
            mkdir -p "$ldir"
            if ! "$EXE" "-O$level" --verify-each --dump-callgraph "$src" > "$ldir/compile.txt" 2>&1; then
                errors+=("-O$level: compiler failed, see $ldir/compile.txt")
                continue
            fi
            mv "$dir/$stem.opt.ll" "$dir/$stem.s" "$dir/$stem.callgraph.txt" "$ldir/"
            if ! "$CLANG" -x ir -c -fverify-intermediate-code -Wno-override-module "$ldir/$stem.opt.ll" \
                    -o /dev/null > "$ldir/verify.txt" 2>&1; then
                errors+=("-O$level: invalid $stem.opt.ll, see $ldir/verify.txt")
            fi
            if [ -n "$FILECHECK" ]; then
                check_lines IR  "$level" "$ldir/$stem.opt.ll"        "$src" "$ldir/check-ir.txt" \
                    || errors+=("-O$level: IR lines do not match, see $ldir/check-ir.txt")
                check_lines ASM "$level" "$ldir/$stem.s"             "$src" "$ldir/check-asm.txt" \
                    || errors+=("-O$level: ASM lines do not match, see $ldir/check-asm.txt")
                check_lines CG  "$level" "$ldir/$stem.callgraph.txt" "$src" "$ldir/check-cg.txt" \
                    || errors+=("-O$level: CG lines do not match, see $ldir/check-cg.txt")
            fi
            if ! "$CLANG" "$ldir/$stem.s" -o "$ldir/$stem" > "$ldir/link.txt" 2>&1; then
                errors+=("-O$level: $stem.s does not link, see $ldir/link.txt")
                continue
            fi
            exitCode="$(run_program "$ldir/$stem" "$input" "$ldir/run.out")"
            if [ -n "$TIMEOUT" ] && [ "$exitCode" = 124 ]; then
                errors+=("-O$level: no exit within 10 seconds")
            elif ! diff -u "$expected" "$ldir/run.out" > "$ldir/run.diff"; then
                errors+=("-O$level: output differs, see $ldir/run.diff")
            elif [ "$exitCode" != "$referenceExit" ]; then
                errors+=("-O$level: exit code $exitCode, expected $referenceExit")
            fi
        done
    fi

    if [ ${#errors[@]} -eq 0 ]; then
        passed=$((passed + 1))
        printf 'PASSED   %s\n' "$shown"
    else
        failed=$((failed + 1))
        printf 'FAILED   %s\n' "$shown"
        printf '    %s\n' "${errors[@]}"
    fi
}

for src in "${tests[@]}"; do
    run_test "$src"
done

echo
echo "${#tests[@]} test(s) at levels $LEVELS: $passed passed, $rejected rejected, $failed failed"
echo "files for each test are in outputs/<stem>/"
[ "$failed" -eq 0 ]
