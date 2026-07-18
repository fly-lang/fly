#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_runtime.sh — run every runtime/test/*Suite.fly. These are `suite`/`case`
# programs (compiled with --test) exercising the Fly runtime (fly.runtime: the
# libc/libm FFI backend + the fly.os wrappers over it). Each is compiled against
# the std + runtime archives via -L; the executable is run and must exit 0
# (fly.assert.* exit non-zero with the failing code).
#
# The runtime is platform-specific (fly.runtime.* links the HOST runtime), so this
# runs ONLY the host suite: it SKIPS foreign-platform suites (RuntimeWindows* /
# RuntimeMacos*) so RuntimeLinuxSuite is what runs on Linux.
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

# Scratch for per-test binaries/logs; under build/ but outside the stage dirs.
OUT=build/test
STD=std/lib
mkdir -p "$OUT"

# Compiler under test: $FLY (default: the stage2 self-host fly — the artifact
# that ships). The compiler derives its stdlib dir from its own executable path
# (argv[0]), so the value is resolved to an absolute path here (a slash-less
# override is resolved through PATH).
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
found=0
for t in $(find runtime/test -name '*Suite.fly' 2>/dev/null | sort); do
    name=$(basename "$t" .fly)
    # Skip foreign-platform suites (their osname/arch assertions target another OS).
    case "$name" in *Windows*|*Macos*) continue ;; esac
    found=$((found + 1))
    bin="$OUT/rt_$name"
    if ! "$FLY" "$t" --test -o "rt_$name" --out-dir "$OUT" -L "$STD" >"$OUT/_rt_$name.log" 2>&1; then
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
    echo "  no runtime/test/*Suite.fly found for this platform"
fi
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
