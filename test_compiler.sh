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
cd "$(dirname "$0")"

FLY="${FLY:-../fly/cmake-build-relwithdebinfo/bin/fly}"
# Scratch for per-suite test binaries/logs; under build/ but separate from
# build/bin so it doesn't sit next to the release artifact.
OUT=build/test
STD=std/lib
mkdir -p "$OUT"

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
