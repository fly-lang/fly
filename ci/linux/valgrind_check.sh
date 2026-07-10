#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# valgrind_check.sh — run the self-host fly under Valgrind (memcheck) on
# crash-prone compiler suites to catch the DOUBLE-FREE / invalid-free that the
# Windows heap aborts on (0xC0000374 in Frontend.resolveSourceDeps) but which
# glibc silently tolerates on Linux. Valgrind flags the invalid free regardless
# and prints, all symbolized, the three stacks that pin the bug:
#   * where it is being freed the SECOND time,
#   * where it was freed the FIRST time,
#   * where the block was allocated.
# If Valgrind is CLEAN, the double-free is Windows-codegen-specific (not a source
# ownership bug). Build with FLY_DEBUG_SYMBOLS=1 for File.fly:line frames.
#
# Runnable locally (./ci/linux/valgrind_check.sh) and from build-linux.yml.
# Env: FLY=<compiler> (default build/stage1/bin/fly), VG_SUITES="a.fly b.fly …".
# Exit: 0 = clean, 1 = a memory error was found (the interesting case).
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
cd "$(dirname "$0")/../.."

FLY="${FLY:-build/stage1/bin/fly}"
if [ ! -x "$FLY" ]; then
    echo "error: '$FLY' not found — run ci/linux/stage1.sh first" >&2
    echo "       (FLY_DEBUG_SYMBOLS=1 in the build gives source-line frames)." >&2
    exit 1
fi
if ! command -v valgrind >/dev/null 2>&1; then
    echo "error: valgrind not installed — 'sudo apt-get install -y valgrind'." >&2
    exit 1
fi

OUT=build/vgcheck; mkdir -p "$OUT"
STD=std/lib
# Suites seen faulting on Windows; the first that trips memcheck is enough.
SUITES="${VG_SUITES:-compiler/test/sema/SemaNodeSuite.fly compiler/test/sema/SymbolTableSuite.fly compiler/test/sema/SemaTypeSuite.fly compiler/test/parser/LexerEdgeSuite.fly}"

found=0
for suite in $SUITES; do
    [ -f "$suite" ] || continue
    name="$(basename "$suite" .fly)"
    log="$OUT/vg_${name}.txt"
    echo "== valgrind memcheck: $name =="
    valgrind --tool=memcheck --error-exitcode=42 --num-callers=40 \
             --track-origins=yes --read-inline-info=yes --read-var-info=yes \
             "$FLY" "$suite" --test -o "t_${name}" --out-dir "$OUT" -L "$STD" \
             > "$OUT/_${name}.out" 2> "$log" || true

    if grep -qE 'Invalid free|Mismatched free|Invalid read|Invalid write' "$log"; then
        echo "  >>> MEMORY ERROR in ${name} — the double-free the Windows heap aborts on:"
        # Print the invalid-free block: the second-free, first-free ('free'd')
        # and allocation ('alloc'd') stacks are all in the ~40 lines that follow.
        awk '/Invalid free|Mismatched free|Invalid read|Invalid write/{p=1} p{print} /ERROR SUMMARY/{exit}' "$log" \
            | sed 's/^/    /' | head -90
        found=1
        break
    fi
    grep -E 'ERROR SUMMARY' "$log" | sed 's/^/  /'
done

if [ "$found" -eq 0 ]; then
    echo "valgrind: no invalid/double free detected on these suites."
    echo "          → the double-free is NOT reproducible on Linux; it is Windows-codegen-specific."
    exit 0
fi
exit 1
