#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_leaks.sh - memory-leak guard for the self-host codegen.
# Mirror of ci/windows/test_leaks.ps1.
#
# Every program under ci/leak_probes/<name>/ loops millions of times over one
# ownership pattern (a string call result bound to a local, a callee local left
# through `return`, a call result passed as an argument, locals left through
# break/continue). Each is compiled by the compiler under test and run while its
# peak RSS (VmHWM) is sampled; a probe that goes over the ceiling FAILS.
#
# One leaked buffer per iteration costs tens of MB over the run, a clean probe
# stays at the process baseline: the ceiling sits well between. The suites
# cannot see this — a leak changes no value, it only grows the heap.
# -----------------------------------------------------------------------------
set -u
cd "$(dirname "$0")/../.."

STAGE="${STAGE:-2}"
FLY="${FLY:-build/stage$STAGE/bin/fly}"
[ -x "$FLY" ] || { echo "error: compiler '$FLY' not found."; exit 1; }
FLY="$(cd "$(dirname "$FLY")" && pwd)/$(basename "$FLY")"

CEILING_KB=$((20 * 1024))
OUT="build/test/leaks"
mkdir -p "$OUT"

failed=0
for probe in ci/leak_probes/*/; do
    name="$(basename "$probe")"
    dir="$OUT/$name"
    mkdir -p "$dir"
    exe="$dir/$name"
    rm -f "$exe"

    if ! "$FLY" --src-dir "$probe" -o "$name" --out-dir "$dir" > "$dir/compile.log" 2>&1 || [ ! -x "$exe" ]; then
        echo "  FAIL  $name (compile)"; tail -n 10 "$dir/compile.log"
        failed=$((failed + 1)); continue
    fi

    # VmHWM is the kernel's running maximum of the RSS: sampling it until the
    # process is gone misses at most the last few ms.
    "$exe" &
    pid=$!
    peak=0
    while kill -0 "$pid" 2>/dev/null; do
        hwm="$(awk '/^VmHWM:/ { print $2 }' "/proc/$pid/status" 2>/dev/null)"
        if [ -n "$hwm" ] && [ "$hwm" -gt "$peak" ]; then peak="$hwm"; fi
        sleep 0.01
    done
    wait "$pid"
    rc=$?
    mb="$(awk -v k="$peak" 'BEGIN { printf "%.1f", k / 1024 }')"
    if [ "$rc" -ne 0 ]; then
        echo "  FAIL  $name (exit $rc, peak $mb MB)"; failed=$((failed + 1))
    elif [ "$peak" -gt "$CEILING_KB" ]; then
        echo "  FAIL  $name peak $mb MB > $((CEILING_KB / 1024)) MB - memory leaked"; failed=$((failed + 1))
    else
        echo "  ok    $name peak $mb MB"
    fi
done

if [ "$failed" -gt 0 ]; then echo "test_leaks: $failed probe(s) failed"; exit 1; fi
echo "test_leaks: all probes under the ceiling"
exit 0
