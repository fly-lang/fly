#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_abi.sh - seed <-> self-host calling-convention guard.
# Mirror of ci/windows/test_abi.ps1.
#
# The shipped std archive (build/stage2/lib/fly_std_lib.a) is compiled by the
# pinned REFERENCE SEED, while user programs are compiled by the SELF-HOST that
# ships beside it. So every call a user program makes into the std crosses a
# compiler boundary, and the two compilers must lower arguments identically. The
# suites cannot see this: they compile both sides with the SAME compiler, so a
# convention both halves get wrong in the same way still passes everywhere.
#
# Each probe under ci/abi_probes/<name>/ is that boundary in miniature:
#   lib/   compiled by build/stage0 (the seed) with --lib -> archive + .fly.h
#   prog/  compiled by the compiler under test, linking that archive via -L
# prog/main.fly checks every value it gets back and `fail`s with a distinct code
# and message, so a mismatch names the clause of the convention that broke
# instead of just crashing.
#
# STAGE selects the self-host under test (default 2, the compiler that ships);
# the seed side is always build/stage0, which is what built the shipped std.
# -----------------------------------------------------------------------------
set -u
cd "$(dirname "$0")/../.."

STAGE="${STAGE:-2}"
FLY="${FLY:-build/stage$STAGE/bin/fly}"
[ -x "$FLY" ] || { echo "error: compiler '$FLY' not found."; exit 1; }
FLY="$(cd "$(dirname "$FLY")" && pwd)/$(basename "$FLY")"

# The seed: the compiler that built the std this stage links. Deliberately NOT
# overridable by $FLY - the point is that the two sides are DIFFERENT compilers.
SEED='build/stage0/bin/fly'
[ -x "$SEED" ] || { echo "error: seed compiler '$SEED' not found - run ci/linux/stage0.sh first."; exit 1; }
SEED="$(cd "$(dirname "$SEED")" && pwd)/$(basename "$SEED")"

OUT='build/test/abi'
mkdir -p "$OUT"

failed=0
passed=0
for probe in ci/abi_probes/*/; do
    name="$(basename "$probe")"
    dir="$OUT/$name"
    libDir="$dir/lib"
    rm -rf "$dir"
    mkdir -p "$libDir"

    # (1) the CALLEE side, built by the seed. --lib emits the archive next to one
    # .fly.h per public module, which is exactly what a -L package dir is.
    if ! "$SEED" --lib -o "$libDir/abi_$name" --src-dir "$probe/lib" > "$dir/seed.log" 2>&1; then
        echo "  FAIL  $name (seed --lib)"; tail -n 8 "$dir/seed.log"
        failed=$((failed + 1)); continue
    fi
    # One `ls` per pattern: `ls a.a missing.lib` fails on the missing one even
    # though the archive is right there, and only ONE of the two ever exists
    # (`.a` on gnu targets, `.lib` on msvc ones).
    archive=0
    for f in "$libDir"/*.a "$libDir"/*.lib; do if [ -f "$f" ]; then archive=1; fi; done
    header=0
    for f in "$libDir"/*.fly.h; do if [ -f "$f" ]; then header=1; fi; done
    if [ "$archive" -eq 0 ] || [ "$header" -eq 0 ]; then
        echo "  FAIL  $name (seed --lib produced no archive/header)"
        failed=$((failed + 1)); continue
    fi

    # (2) the CALLER side, built by the compiler under test, resolving the
    # imports through that .fly.h and linking that archive.
    exe="$dir/abi_$name"
    if ! "$FLY" --src-dir "$probe/prog" -o "abi_$name" --out-dir "$dir" -L "$libDir" \
            > "$dir/compile.log" 2>&1 || [ ! -x "$exe" ]; then
        echo "  FAIL  $name (self-host compile)"; tail -n 8 "$dir/compile.log"
        failed=$((failed + 1)); continue
    fi

    # (3) run it. Exit 0 = every clause agreed; anything else is the `fail` code,
    # and the probe printed which check it was.
    "$exe" > "$dir/run.log" 2>&1
    rc=$?
    if [ "$rc" -eq 0 ]; then
        echo "  ok    $name"
        passed=$((passed + 1))
    else
        echo "  FAIL  $name (exit $rc) - seed/self-host ABI mismatch"
        sed 's/^/        /' "$dir/run.log"
        failed=$((failed + 1))
    fi
done

if [ "$passed" -eq 0 ] && [ "$failed" -eq 0 ]; then echo 'error: no probes under ci/abi_probes'; exit 1; fi
if [ "$failed" -gt 0 ]; then echo "test_abi: $failed probe(s) failed"; exit 1; fi
echo "test_abi: all checks passed ($passed probe(s) agree with the seed ABI)"
exit 0
