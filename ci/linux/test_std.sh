#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_std.sh — run every std/test/*_test.fly. These are main()-style programs (not
# `suite`/`--test` blocks) that exercise the standard library: fly.str, fly.math,
# fly.data.* containers, fly.os.* (env/fs/io/path/time), fly.mem, generics, enums,
# and the inheritance/override/super paths. Each is a standalone program compiled
# against the std archive via -L (no --test / --src-dir); the resulting executable
# is run and must exit 0 (fly.assert.* exit non-zero with the failing code). Mirror
# of fly/std/test (run there by ctest).
#
# Scope: ONLY std/test. Compiler, driver, and flyp suites run in their own scripts.
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

# Scratch for per-test binaries/logs; under build/ but separate from build/bin.
OUT=build/test
STD=std/lib
mkdir -p "$OUT"

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
for t in $(find std/test -name '*_test.fly' | sort); do
    name=$(basename "$t" .fly)
    bin="$OUT/std_$name"
    if ! "$FLY" "$t" -o "std_$name" --out-dir "$OUT" -L "$STD" >"$OUT/_std_$name.log" 2>&1; then
        echo "  COMPILE FAIL  $name"
        # match real diagnostics ('error:'), not the substring "error" inside
        # warnings like 'errorHandler'; -m3 avoids the "Broken pipe" noise.
        grep -m3 -E 'error:|broken|abort' "$OUT/_std_$name.log" | sed 's/^/      /'
        fail=$((fail + 1))
        continue
    fi
    if "$bin" >"$OUT/_std_$name.run" 2>&1; then
        echo "  PASS          $name"
        pass=$((pass + 1))
    else
        echo "  RUN  FAIL     $name (exit $?)"
        tail -5 "$OUT/_std_$name.run" | sed 's/^/      /'
        fail=$((fail + 1))
    fi
done

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
