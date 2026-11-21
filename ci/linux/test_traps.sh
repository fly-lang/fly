#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_traps.sh - run-time trap guard for the self-host compiler.
# Mirror of ci/windows/test_traps.ps1.
#
# Some errors can only be caught while the program runs. Fly stops it there with
# a diagnostic - "error 134: <what> at <file>:<line>:<col>" and exit code 134 -
# instead of letting it crash or read garbage. The suites cannot see a trap that
# has gone missing: they run valid programs, so a compiler that stops emitting
# the check still passes every one of them.
#
# Each probe under ci/trap_probes/<name>/ is one trap in miniature:
#   main.fly     the program;
#   expect.txt   line 1 the exit code it must end with; line 2 a fragment of the
#                message it must print; line 3 the position (file:line:col) that
#                must follow. A probe that must run normally expects 0 and empty
#                lines.
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

OUT=build/test/traps
mkdir -p "$OUT"
failed=0
count=0

for probe in ci/trap_probes/*/; do
    name="$(basename "$probe")"
    count=$((count + 1))
    want_code="$(sed -n 1p "$probe/expect.txt" | tr -d '\r')"
    want_msg="$(sed -n 2p "$probe/expect.txt" | tr -d '\r')"
    want_pos="$(sed -n 3p "$probe/expect.txt" | tr -d '\r')"

    dir="$OUT/$name"
    rm -rf "$dir"
    mkdir -p "$dir"
    if ! "$FLY" --src-dir "$probe" -o "$name" --out-dir "$dir" > "$dir/compile.log" 2>&1 || [ ! -x "$dir/$name" ]; then
        echo "  FAIL  $name - does not compile"
        tail -n 5 "$dir/compile.log" | sed 's/^/        /'
        failed=$((failed + 1))
        continue
    fi

    "$dir/$name" > "$dir/run.log" 2>&1
    rc=$?
    if [ "$rc" != "$want_code" ]; then
        echo "  FAIL  $name - exit $rc, expected $want_code"
        head -n 3 "$dir/run.log" | sed 's/^/        /'
        failed=$((failed + 1))
    elif [ -n "$want_msg" ] && ! grep -qF -- "$want_msg" "$dir/run.log"; then
        echo "  FAIL  $name - message missing: $want_msg"
        head -n 3 "$dir/run.log" | sed 's/^/        /'
        failed=$((failed + 1))
    elif [ -n "$want_pos" ] && ! grep -qF -- "$want_pos" "$dir/run.log"; then
        echo "  FAIL  $name - position missing: $want_pos"
        head -n 3 "$dir/run.log" | sed 's/^/        /'
        failed=$((failed + 1))
    else
        echo "  ok    $name"
    fi
done

[ "$count" -gt 0 ] || { echo "error: no probes under ci/trap_probes"; exit 1; }
if [ "$failed" -gt 0 ]; then
    echo "test_traps: $failed of $count probe(s) failed"
    exit 1
fi
echo "test_traps: all checks passed ($count probes)"
exit 0
