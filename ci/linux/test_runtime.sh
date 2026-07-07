#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_runtime.sh — run every runtime/test/*_test.fly. These are main()-style
# programs exercising the Fly runtime (fly.runtime: the libc/libm FFI backend +
# the fly.os wrappers over it). Each is a standalone program compiled against the
# std + runtime archives via -L (no --test / --src-dir); the executable is run and
# must exit 0 (fly.assert.* exit non-zero with the failing code).
#
# runtime/test is currently empty (the runtime is exercised indirectly by the
# std/os suites in test_std.sh); this script runs 0 tests today but picks up any
# *_test.fly added under runtime/test. Scope: ONLY runtime/test.
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
found=0
for t in $(find runtime/test -name '*_test.fly' 2>/dev/null | sort); do
    found=$((found + 1))
    name=$(basename "$t" .fly)
    bin="$OUT/rt_$name"
    if ! "$FLY" "$t" -o "rt_$name" --out-dir "$OUT" -L "$STD" >"$OUT/_rt_$name.log" 2>&1; then
        echo "  COMPILE FAIL  $name"
        grep -m3 -E 'error:|broken|abort' "$OUT/_rt_$name.log" | sed 's/^/      /'
        fail=$((fail + 1))
        continue
    fi
    if "$bin" >"$OUT/_rt_$name.run" 2>&1; then
        echo "  PASS          $name"
        pass=$((pass + 1))
    else
        echo "  RUN  FAIL     $name (exit $?)"
        tail -5 "$OUT/_rt_$name.run" | sed 's/^/      /'
        fail=$((fail + 1))
    fi
done

echo "─────────────────────────────────────────────"
if [ "$found" -eq 0 ]; then
    echo "  no runtime/test/*_test.fly found (runtime exercised via std/os suites)"
fi
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
