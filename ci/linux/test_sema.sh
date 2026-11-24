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
#   warn/<name>/main.fly      must compile with the warnings its expect.txt lists.
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

# -- seed verdicts ------------------------------------------------------------
# The same reject probes, judged by the SEED (build/stage0). The seed compiles
# the whole tree at stage 1, so what it accepts and refuses is part of the
# language's effective definition - and every difference from the self-host must
# be a recorded decision, not an accident. A probe the seed is known to accept
# carries a seed-accepts.txt saying why; anything else must be refused by both.
# A seed that starts accepting or refusing something else fails here, which is
# exactly the moment a seed re-cut changed the language. From 0.16 the seed is
# the 0.15.2 self-host release, which enforces every rule the tree checks: no
# probe carries a seed-accepts.txt (release/0.15.X recorded the rules 0.14.9 did
# not have yet).
SEED="build/stage0/bin/fly"
SEEDLIB="build/stage1/lib"
if [ -x "$SEED" ] && [ -d "$SEEDLIB" ]; then
    SEED="$(cd "$(dirname "$SEED")" && pwd)/$(basename "$SEED")"
    SEEDLIB="$(cd "$SEEDLIB" && pwd)"
    for probe in ci/sema_probes/reject/*/; do
        name="$(basename "$probe")"
        count=$((count + 1))
        known=0; [ -f "$probe/seed-accepts.txt" ] && known=1
        "$SEED" --src-dir "$probe" --no-output -L "$SEEDLIB" > "$OUT/seed_$name.log" 2>&1
        rc=$?
        if [ "$rc" -eq 0 ] && [ "$known" -eq 0 ]; then
            echo "  FAIL  seed/$name - the seed ACCEPTS it and no seed-accepts.txt records why"; failed=$((failed + 1))
        elif [ "$rc" -ne 0 ] && [ "$known" -eq 1 ]; then
            echo "  FAIL  seed/$name - seed-accepts.txt is stale: the seed now REJECTS it"; failed=$((failed + 1))
        elif [ "$known" -eq 1 ]; then
            echo "  ok    seed/$name (recorded divergence: seed accepts)"
        else
            echo "  ok    seed/$name"
        fi
    done
else
    echo "  skip  seed verdicts - build/stage0 or build/stage1/lib not present"
fi

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

# warn/<name>/main.fly must compile, link and exit 0, and its compile log must
# hold every line of expect.txt; a line starting with '!' must NOT appear (the
# cases a warning must stay quiet about). Warnings never stop a build, so like a
# missing check, a missing warning is invisible to every suite.
for probe in ci/sema_probes/warn/*/; do
    [ -d "$probe" ] || continue
    name="$(basename "$probe")"
    count=$((count + 1))
    dir="$OUT/warn_$name"
    mkdir -p "$dir"
    rm -f "$dir/$name"
    "$FLY" --src-dir "$probe" -o "$name" --out-dir "$dir" > "$dir/compile.log" 2>&1
    if [ $? -ne 0 ] || [ ! -x "$dir/$name" ]; then
        echo "  FAIL  warn/$name - REJECTED (a warning must not stop the build)"
        head -n 5 "$dir/compile.log" | sed 's/^/        /'
        failed=$((failed + 1)); continue
    fi
    bad=""
    while IFS= read -r line || [ -n "$line" ]; do
        line="${line%$'\r'}"
        [ -z "$line" ] && continue
        case "$line" in
            '!'*)
                if grep -qF -- "${line#!}" "$dir/compile.log"; then bad="unexpected: ${line#!}"; break; fi ;;
            *)
                if ! grep -qF -- "$line" "$dir/compile.log"; then bad="missing: $line"; break; fi ;;
        esac
    done < "$probe/expect.txt"
    if [ -n "$bad" ]; then
        echo "  FAIL  warn/$name - $bad"; failed=$((failed + 1))
    else
        echo "  ok    warn/$name"
    fi
done
# Seed-compiled sources: std/lib, std/test, runtime/lib and compiler/lib are
# compiled by the seed (string literals RAW) as well as by the self-host (escapes
# decoded, from 0.15). A literal holding a backslash, or a raw literal between
# three double quotes, would mean one thing to each, and nothing else would
# notice: the stage 3 fixpoint compares two self-host builds. Build such bytes
# from their value instead (Escapes.byteString, mem.writeByte).
count=$((count + 1))
seedlit="$(grep -rnE '^[^/]*("[^"]*[\]|""")' --include='*.fly' std/lib std/test runtime/lib compiler/lib || true)"
if [ -n "$seedlit" ]; then
    echo "  FAIL  seed-literals - a literal the seed reads differently:"
    echo "$seedlit" | head -n 5 | sed 's/^/        /'
    failed=$((failed + 1))
else
    echo "  ok    seed-literals"
fi
if [ "$failed" -gt 0 ]; then echo "test_sema: $failed of $count probe(s) failed"; exit 1; fi
echo "test_sema: all $count probes behave as specified"
exit 0
