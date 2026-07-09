#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_compiler.sh — run every compiler/test/**/*Suite.fly against the compiler
# sources, without flyp. Single-file build: each suite is the entry; source
# discovery is implicit (a fly project compiles from the CURRENT directory — the
# repo root here), so the import graph pulls fly.compiler.*, fly.test.util, … into
# one module while std namespaces stay archive-linked (the -L pass registers them
# first). `--test` builds in test mode; `--out-dir` sends the executable and its
# intermediate objects into $OUT; the resulting executable is then run.
#
# Scope: ONLY compiler/test (the compiler's own unit suites). The std, driver, and
# flyp suites run in their own scripts — see the note at the bottom of this file.
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

# Scratch for per-suite test binaries/logs; under build/ but separate from
# the stage dirs so it doesn't sit next to the release artifact.
OUT=build/test
STD=std/lib
mkdir -p "$OUT"

# CodeGen/target suites emit IR/objects to /tmp/cg and read them back (e.g.
# cgm.emitIR("/tmp/cg/suite.ll")). The directory must exist before they run —
# otherwise the LLVM file open fails and LLVMPrintModuleToFile/EmitToFile crash
# on the error path. Create it up front so a fresh checkout/CI runner passes.
mkdir -p /tmp/cg

# Compiler under test: $FLY (default: the stage2 self-host fly — the artifact
# that ships; its --test system was ported from the reference and all suites
# are green under it). The compiler derives its stdlib dir from its own
# executable path (argv[0]), so the value is resolved to an absolute path here
# (a slash-less override is resolved through PATH).
FLY="${FLY:-build/stage2/bin/fly}"
case "$FLY" in
    /*) ;;                    # absolute — keep as given
    */*) FLY="$PWD/$FLY" ;;   # relative — anchor to the repo root
    *)
        FLY_RESOLVED="$(command -v "$FLY" || true)"
        if [ -z "$FLY_RESOLVED" ]; then
            echo "error: compiler '$FLY' not found on PATH." >&2
            exit 1
        fi
        FLY="$FLY_RESOLVED"
        ;;
esac
if [ ! -x "$FLY" ]; then
    echo "error: FLY='$FLY' is not an executable file — run ci/linux/stage2.sh first (or set FLY)." >&2
    exit 1
fi
if [ ! -d "$(dirname "$FLY")/../lib" ]; then
    echo "error: no lib/ directory next to '$FLY' (expected <exe_dir>/../lib" >&2
    echo "       with llvm.fly.h, runtime.fly.h, fly_runtime_lib.a)." >&2
    exit 1
fi

pass=0
fail=0
for suite in $(find compiler/test -name '*Suite.fly' | sort); do
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

# Sibling test scripts run the other targets as separate workflow steps:
#   test_std.sh        — std/test/*_test.fly (std library)
#   test_driver.sh     — driver/test/*Suite.fly (driver + package manager)
#   test_tools_flyp.sh — tools/flyp/test/*Suite.fly

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
