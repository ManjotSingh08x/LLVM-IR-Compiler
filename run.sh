
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

TEST_DIR="test"
OUT_DIR="outputs"
DEFAULT_EXE="build/parser"

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

mkdir -p "$OUT_DIR"

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

for f in "${files[@]}"; do
    base="$(basename "$f" .ckp)"
    outfile="$OUT_DIR/${base}.txt"

    if "$EXE" "$f" > "$outfile" 2>&1; then
        parsed=$((parsed + 1))
        printf 'PARSED   %-36s token table written\n' "$(basename "$f")"
    else
        rejected=$((rejected + 1))
        count="$(grep -cE 'line: [0-9]+ col: [0-9]+' "$outfile")"
        if [ "$count" -gt 0 ]; then
            printf 'REJECTED %-36s %s error(s)\n' "$(basename "$f")" "$count"
        else
            printf 'REJECTED %-36s\n' "$(basename "$f")"
        fi
        sed 's/^/         /' "$outfile"
    fi
done

echo
echo "$((parsed + rejected)) test file(s) run: $parsed parsed, $rejected rejected."
echo "Full output for each file is in $OUT_DIR/"