#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test.sh — run every test/**/*Suite.fly against the compiler sources, without
# flyp. Single-file build: each suite is the entry, and `--src-dir compiler`
# resolves the whole `fly.compiler` dependency graph from its imports into one
# module. `--test` builds in test mode; `--out-dir` sends the executable and its
# intermediate objects into $OUT; the resulting executable is then run.
# No file list, no concatenation.
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

# Scratch for per-suite test binaries/logs; under build/ but separate from
# build/bin so it doesn't sit next to the release artifact.
OUT=build/test
STD=std/lib
mkdir -p "$OUT"

# CodeGen/target suites emit IR/objects to /tmp/cg and read them back (e.g.
# cgm.emitIR("/tmp/cg/suite.ll")). The directory must exist before they run —
# otherwise the LLVM file open fails and LLVMPrintModuleToFile/EmitToFile crash
# on the error path. Create it up front so a fresh checkout/CI runner passes.
mkdir -p /tmp/cg

# Bootstrap compiler: $FLY (default `fly`). The compiler derives its stdlib dir
# from its own executable path (argv[0]), and a bare name breaks that lookup —
# so a slash-less $FLY is resolved through PATH into an absolute path here.
FLY="${FLY:-fly}"
case "$FLY" in
    */*) ;;  # already a path — keep as given
    *)
        FLY_RESOLVED="$(command -v "$FLY" || true)"
        if [ -z "$FLY_RESOLVED" ]; then
            echo "error: bootstrap compiler '$FLY' not found on PATH." >&2
            echo "       Set FLY to the bootstrap compiler, e.g.:" >&2
            echo "       FLY=/path/to/fly/build/bin/fly $0" >&2
            exit 1
        fi
        FLY="$FLY_RESOLVED"
        ;;
esac
if [ ! -x "$FLY" ]; then
    echo "error: FLY='$FLY' is not an executable file." >&2
    exit 1
fi
if [ ! -d "$(dirname "$FLY")/../lib" ]; then
    echo "error: no lib/ directory next to '$FLY' (expected <exe_dir>/../lib" >&2
    echo "       with llvm.fly.h, runtime.fly.h, fly_runtime_lib.a)." >&2
    echo "       Point FLY at a built bootstrap compiler, e.g. fly/build/bin/fly." >&2
    exit 1
fi

pass=0
fail=0
for suite in $(find test -name '*Suite.fly' | sort); do
    name=$(basename "$suite" .fly)
    bin="$OUT/test_$name"
    if ! "$FLY" "$suite" --test --src-dir compiler -o "test_$name" --out-dir "$OUT" -L "$STD" >"$OUT/_$name.log" 2>&1; then
        echo "  COMPILE FAIL  $name"
        # match real diagnostics ('error:'), not the substring "error" inside
        # warnings like 'errorHandler'; -m3 instead of |head avoids the
        # "grep: write error: Broken pipe" noise on every failure
        grep -m3 -E 'error:|broken|abort' "$OUT/_$name.log" | sed 's/^/      /'
        fail=$((fail + 1))
        continue
    fi
    if "$bin" >"$OUT/_$name.run" 2>&1; then
        echo "  PASS          $name"
        pass=$((pass + 1))
    else
        echo "  RUN  FAIL     $name (exit $?)"
        tail -5 "$OUT/_$name.run" | sed 's/^/      /'
        fail=$((fail + 1))
    fi
done

# ── std library tests ────────────────────────────────────────────────────────
# std/test/*_test.fly are main()-style programs (mirrors of fly/std/test, run
# there by ctest). They exercise std/lib via -L; no --src-dir/--test needed.
# Without this loop they are orphans — nothing in CI ever ran them.
for t in $(find std/test -name '*_test.fly' | sort); do
    name=$(basename "$t" .fly)
    bin="$OUT/std_$name"
    if ! "$FLY" "$t" -o "std_$name" --out-dir "$OUT" -L "$STD" >"$OUT/_std_$name.log" 2>&1; then
        echo "  COMPILE FAIL  std/$name"
        grep -m3 -E 'error:|broken|abort' "$OUT/_std_$name.log" | sed 's/^/      /'
        fail=$((fail + 1))
        continue
    fi
    if "$bin" >"$OUT/_std_$name.run" 2>&1; then
        echo "  PASS          std/$name"
        pass=$((pass + 1))
    else
        echo "  RUN  FAIL     std/$name (exit $?)"
        tail -5 "$OUT/_std_$name.run" | sed 's/^/      /'
        fail=$((fail + 1))
    fi
done

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
