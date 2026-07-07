#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_driver.sh — run every driver/test/**/*Suite.fly (the driver + package
# manager unit suites: CLI parsing, Manifest/toml, lockfile, semver/MVS resolver,
# registry, ToolChain, cache/checksum/json). Single-file build: each suite is the
# entry; source discovery is implicit (a fly project compiles from the CURRENT
# directory — the repo root here), so the import graph pulls fly.driver.* AND
# fly.compiler.* source into one module while std namespaces stay archive-linked
# (the -L pass registers them first). `--test` builds in test mode; `--out-dir`
# sends the executable + intermediate objects into $OUT; the executable is then run.
#
# The suites live in namespace fly.driver.test(.cli) — a DIFFERENT namespace from
# driver/lib/Driver.fly (fly.driver), so the suite's generated `--test` main is the
# C entry and Driver.fly's own `main` is pulled in as an ordinary (mangled, unused)
# function — no symbol clash.
#
# Scope: ONLY driver/test. The compiler, std, and flyp suites run in their own
# scripts (test_compiler.sh / test_std.sh / test_tools_flyp.sh).
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

# Scratch for per-suite test binaries/logs; under build/ but separate from
# build/bin so it doesn't sit next to the release artifact.
OUT=build/test
STD=std/lib
mkdir -p "$OUT"

# CodeGen/target paths (pulled transitively via fly.compiler.*) may emit IR/objects
# to /tmp/cg; create it up front so the file open never fails on a fresh runner.
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
for suite in $(find driver/test -name '*Suite.fly' | sort); do
    name=$(basename "$suite" .fly)
    bin="$OUT/test_$name"
    if ! "$FLY" "$suite" --test -o "test_$name" --out-dir "$OUT" -L "$STD" >"$OUT/_$name.log" 2>&1; then
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

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
