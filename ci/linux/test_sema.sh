#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_sema.sh - semantic-check guard for the self-host compiler.
# Mirror of ci/windows/test_sema.ps1.
#
# The suites only ever compile VALID programs, so they cannot see a check that
# has gone missing: a compiler that stops rejecting `int n = aLong` still passes
# every one of them. That is how the self-host came to accept programs the seed
# refuses (narrowing, private access, abstract instantiation, ...) while the
# whole pipeline stayed green - the tree itself was protected only because the
# SEED type-checks it at stage 1.
#
# Each probe under ci/sema_probes is one rule in miniature:
#   reject/<name>/main.fly    must NOT compile, and
#   reject/<name>/expect.txt  holds a fragment of the diagnostic it must get.
#   accept/<name>/main.fly    must compile, link and exit 0 - the edge cases a
#                             check must keep allowing (a literal that fits, a
#                             shadowing declaration, an overload that differs).
#
# A reject probe fails three ways, and they are told apart on purpose:
#   ACCEPTED   the compiler took the program - the check is gone;
#   INTERNAL   it died in the backend verifier instead of diagnosing - the
#              program reached codegen, which is the same hole with a crash on top;
#   WRONG MSG  it was rejected, but not for the reason the probe is about.
#
# STAGE selects the compiler under test (default 2, the one that ships).
# -----------------------------------------------------------------------------
set -u
cd "$(dirname "$0")/../.."

STAGE="${STAGE:-2}"
FLY="${FLY:-build/stage$STAGE/bin/fly}"
[ -x "$FLY" ] || { echo "error: compiler '$FLY' not found."; exit 1; }
FLY="$(cd "$(dirname "$FLY")" && pwd)/$(basename "$FLY")"

OUT="build/test/sema"
mkdir -p "$OUT"
failed=0
count=0

for probe in ci/sema_probes/reject/*/; do
    name="$(basename "$probe")"
    count=$((count + 1))
    # the expected fragment, without the trailing newline / a CR from a CRLF checkout
    expect="$(tr -d '\r' < "$probe/expect.txt" | head -n 1)"
    log="$OUT/reject_$name.log"
    "$FLY" --src-dir "$probe" --no-output > "$log" 2>&1
    rc=$?
    if grep -qF 'backend module verification failed' "$log"; then
        echo "  FAIL  reject/$name - INTERNAL error instead of a diagnostic"; failed=$((failed + 1))
    elif [ "$rc" -eq 0 ]; then
        echo "  FAIL  reject/$name - ACCEPTED (expected: $expect)"; failed=$((failed + 1))
    elif ! grep -qF -- "$expect" "$log"; then
        echo "  FAIL  reject/$name - WRONG MSG (expected: $expect)"
        head -n 3 "$log" | sed 's/^/        /'
        failed=$((failed + 1))
    else
        echo "  ok    reject/$name"
    fi
done

for probe in ci/sema_probes/accept/*/; do
    name="$(basename "$probe")"
    count=$((count + 1))
    dir="$OUT/accept_$name"
    mkdir -p "$dir"
    rm -f "$dir/$name"
    "$FLY" --src-dir "$probe" -o "$name" --out-dir "$dir" > "$dir/compile.log" 2>&1
    if [ $? -ne 0 ] || [ ! -x "$dir/$name" ]; then
        echo "  FAIL  accept/$name - REJECTED"
        head -n 5 "$dir/compile.log" | sed 's/^/        /'
        failed=$((failed + 1)); continue
    fi
    "$dir/$name" > "$dir/run.log" 2>&1
    rc=$?
    if [ "$rc" -ne 0 ]; then
        echo "  FAIL  accept/$name - exit $rc"; failed=$((failed + 1))
    else
        echo "  ok    accept/$name"
    fi
done

if [ "$failed" -gt 0 ]; then echo "test_sema: $failed of $count probe(s) failed"; exit 1; fi
echo "test_sema: all $count probes behave as specified"
exit 0
