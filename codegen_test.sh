#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# codegen_test.sh — compile + run each CodeGen test (test/codegen/*_main.fly).
# CodeGen tests are plain main() programs (the suite/case harness chokes on
# llvm.api imports); each builds IR, verifies it, and exits 0 on success. The
# binary dynamically loads libLLVM-20.so via the CLang bridge at runtime.
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
cd "$(dirname "$0")"

FLY="${FLY:-../fly/cmake-build-relwithdebinfo/bin/fly}"
OUT=/tmp/cgtest
STD=std/lib
rm -rf "$OUT"; mkdir -p "$OUT"; mkdir -p /tmp/cg

pass=0
fail=0
for t in $(find test/codegen -name '*_main.fly' | sort); do
    name=$(basename "$t" _main.fly)
    if ! "$FLY" "$t" --src-dir compiler -o "$name" --out-dir "$OUT" -L "$STD" >"$OUT/_$name.log" 2>&1; then
        echo "  COMPILE FAIL  $name"
        grep -iE 'error|broken' "$OUT/_$name.log" | head -3 | sed 's/^/      /'
        fail=$((fail + 1))
        continue
    fi
    if "$OUT/$name" >"$OUT/_$name.run" 2>&1; then
        echo "  PASS          $name"
        pass=$((pass + 1))
    else
        echo "  RUN  FAIL     $name (exit $?)"
        tail -3 "$OUT/_$name.run" | sed 's/^/      /'
        fail=$((fail + 1))
    fi
done

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
