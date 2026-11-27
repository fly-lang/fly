#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_heap.sh — object heap guard (fly-lang/fly#235).
# Mirror of ci/windows/test_heap.ps1.
#
# Every class object lives in the runtime's object heap, which knows the live
# ones: at the end of main the program reports on stderr the objects it never
# deleted, by class, then releases them (an object before what it owns, a cycle
# once). The suites cannot see this — the report is the program's own output
# after its last statement, and a suite that leaks still passes.
#
# Each probe under ci/heap_probes/<name>/ is one property in miniature:
#   main.fly     the program;
#   expect.txt   line 1: the exit code it must end with;
#   stderr.txt   what stderr must be, exactly (absent: nothing at all);
#   stdout.txt   what stdout must be, exactly (absent: nothing at all);
#   suite.txt    optional: line 1 names a suite in the probe's sources; the probe
#                is then built and run by `fly --suite=<name>` (its output: the
#                case results on stdout, the per-case leak warnings on stderr)
#                instead of as a program with a main.
# Line endings are ignored.
#
# STAGE selects the compiler under test (default 2, the one that ships).
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
cd "$(dirname "$0")/../.."

STAGE="${STAGE:-2}"
export STAGE
FLY="${FLY:-build/stage$STAGE/bin/fly}"
[ -x "$FLY" ] || { echo "error: compiler '$FLY' not found."; exit 1; }
FLY="$(cd "$(dirname "$FLY")" && pwd)/$(basename "$FLY")"

OUT=build/test/heap
mkdir -p "$OUT"
failed=0
count=0

# norm FILE — the file with CR dropped and trailing newlines trimmed (empty if absent)
norm() {
    if [ -f "$1" ]; then
        tr -d '\r' < "$1" | sed -e :a -e '/^\n*$/{$d;N;ba' -e '}'
    fi
}

for probe in ci/heap_probes/*/; do
    name="$(basename "$probe")"
    count=$((count + 1))
    want_code="$(sed -n 1p "$probe/expect.txt" | tr -d '\r')"

    dir="$OUT/$name"
    rm -rf "$dir"
    mkdir -p "$dir"
    if [ -f "$probe/suite.txt" ]; then
        suite="$(sed -n 1p "$probe/suite.txt" | tr -d '\r')"
        "$FLY" --suite="$suite" --src-dir "$probe" --out-dir "$dir" > "$dir/stdout.log" 2> "$dir/stderr.log"
        rc=$?
    else
        if ! "$FLY" --src-dir "$probe" -o "$name" --out-dir "$dir" > "$dir/compile.log" 2>&1 \
                || [ ! -x "$dir/$name" ]; then
            echo "  FAIL  $name - does not compile"
            tail -n 5 "$dir/compile.log" | sed 's/^/        /'
            failed=$((failed + 1))
            continue
        fi

        "$dir/$name" > "$dir/stdout.log" 2> "$dir/stderr.log"
        rc=$?
    fi
    if [ "$rc" != "$((want_code % 256))" ]; then
        echo "  FAIL  $name - exit $rc, expected $want_code"
        failed=$((failed + 1))
    elif [ "$(norm "$dir/stderr.log")" != "$(norm "$probe/stderr.txt")" ]; then
        echo "  FAIL  $name - stderr differs; expected:"
        norm "$probe/stderr.txt" | sed 's/^/        /'
        echo "        got:"
        norm "$dir/stderr.log" | head -n 10 | sed 's/^/        /'
        failed=$((failed + 1))
    elif [ "$(norm "$dir/stdout.log")" != "$(norm "$probe/stdout.txt")" ]; then
        echo "  FAIL  $name - stdout differs; expected:"
        norm "$probe/stdout.txt" | sed 's/^/        /'
        echo "        got:"
        norm "$dir/stdout.log" | head -n 10 | sed 's/^/        /'
        failed=$((failed + 1))
    else
        echo "  ok    $name"
    fi
done

[ "$count" -gt 0 ] || { echo "error: no probes under ci/heap_probes"; exit 1; }
if [ "$failed" -gt 0 ]; then
    echo "test_heap: $failed of $count probe(s) failed"
    exit 1
fi
echo "test_heap: all checks passed ($count probes)"
exit 0
