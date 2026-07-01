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

# Bootstrap compiler: $FLY (default `fly`). CI passes an absolute path — see the
# note in build_compiler.sh (executable-path stdlib discovery / bare-name trap).
FLY="${FLY:-fly}"

pass=0
fail=0
for suite in $(find test -name '*Suite.fly' | sort); do
    name=$(basename "$suite" .fly)
    bin="$OUT/test_$name"
    if ! "$FLY" "$suite" --test --src-dir compiler -o "test_$name" --out-dir "$OUT" -L "$STD" >"$OUT/_$name.log" 2>&1; then
        echo "  COMPILE FAIL  $name"
        grep -iE 'error|broken|abort' "$OUT/_$name.log" | head -3 | sed 's/^/      /'
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

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
