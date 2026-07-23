#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# valgrind_check.sh — run the self-host fly under Valgrind (memcheck) over the
# whole test corpus to catch double-free / invalid-free / use-after-free bugs
# (the owned-string aliasing class the Windows heap aborts on with 0xC0000374
# but glibc silently tolerates on Linux). Valgrind flags each one and prints,
# all symbolized (needs FLY_DEBUG_SYMBOLS=1), the offending stacks — for a
# double-free: the second free, the first free, and the allocation site.
#
# By default it sweeps EVERY test suite for a full safety net — all three trees
# are now `suite`/`case` programs compiled with --test:
#   compiler/test/**/*Suite.fly (incl. driver)
#   std/test/**/*Suite.fly        runtime/test/*Suite.fly
# The memory bug is in the COMPILER while it compiles each test (e.g. the
# resolveSourceDeps string double-free), so Valgrind wraps the fly COMPILE.
#
# Runnable locally (./ci/linux/valgrind_check.sh) and from build-linux.yml.
# Env: FLY=<compiler> (default build/stage1/bin/fly); VG_SUITES="a.fly b.fly …"
#      to scope it; VG_LIMIT=N to cap how many suites run (0 = all).
# Exit: 0 = clean, 1 = at least one suite tripped memcheck. NOTE: a full sweep
#       under Valgrind is slow (the compiler runs libLLVM) — expect minutes.
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
LIMIT="${VG_LIMIT:-0}"

# Collect every test file (or honour a VG_SUITES override).
if [ -n "${VG_SUITES:-}" ]; then
    SUITES="$VG_SUITES"
else
    SUITES="$(
        { find compiler/test -name '*Suite.fly'
          find std/test      -name '*Suite.fly' 2>/dev/null
          find runtime/test  -name '*Suite.fly' 2>/dev/null
        } | sort )"
fi

count=$(printf '%s\n' $SUITES | grep -c . || true)
echo "valgrind: sweeping ${count} test suites under memcheck (FLY=$FLY)"

total=0
bad=0
badlist=""
for suite in $SUITES; do
    [ -f "$suite" ] || continue
    if [ "$LIMIT" -gt 0 ] && [ "$total" -ge "$LIMIT" ]; then break; fi
    total=$((total + 1))
    name="$(basename "$suite" .fly)"
    log="$OUT/vg_${name}.txt"
    # DIRECTORY CLI (the self-host fly): no positional — a *Suite.fly is selected
    # by name via --suite=<Name> (fly also RUNS the built suite once, natively:
    # valgrind has no --trace-children, so only the COMPILE is memchecked, as
    # before); a plain main()-style VG_SUITES override compiles its directory.
    # The source root is the suite's OWN tree: suite names repeat across trees
    # (e.g. ManifestSuite exists in both compiler/test and std/test).
    case "$suite" in
        compiler/*) SRCROOT=compiler ;;
        std/*)      SRCROOT=std ;;
        runtime/*)  SRCROOT=runtime/test ;;
        *)          SRCROOT="$(dirname "$suite")" ;;
    esac
    if case "$suite" in *Suite.fly) true ;; *) false ;; esac; then
        FLY_ARGS=(--suite="$name" --src-dir "$SRCROOT" -o "t_${name}" --out-dir "$OUT" -L "$STD")
    else
        FLY_ARGS=(--src-dir "$(dirname "$suite")" -o "t_${name}" --out-dir "$OUT" -L "$STD")
    fi

    printf '  [%d/%d] %s ... ' "$total" "$count" "$name"
    valgrind --tool=memcheck --error-exitcode=42 --leak-check=no --num-callers=40 \
             --track-origins=yes --read-inline-info=yes \
             "$FLY" "${FLY_ARGS[@]}" \
             > "$OUT/_${name}.out" 2> "$log" || true

    if grep -qE 'Invalid free|Mismatched free|Invalid read|Invalid write' "$log"; then
        bad=$((bad + 1))
        badlist="${badlist} ${name}"
        echo "MEMORY ERROR"
        echo "  ┌─ $suite ─────────────────────────────────────────────"
        # The double-free's three stacks (2nd free / 1st free / alloc) live in the
        # ~60 lines from the first error to the ERROR SUMMARY.
        awk '/Invalid free|Mismatched free|Invalid read|Invalid write/{p=1} p{print} /ERROR SUMMARY/{exit}' "$log" \
            | sed 's/^/  │ /' | head -70
        echo "  └──────────────────────────────────────────────────────"
    else
        echo "ok"
    fi
done

echo "─────────────────────────────────────────────"
echo "valgrind: ${total} suites checked, ${bad} with memory errors${badlist:+ →${badlist}}"
[ "$bad" -eq 0 ] && exit 0 || exit 1
