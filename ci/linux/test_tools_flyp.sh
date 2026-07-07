#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_tools_flyp.sh — run every tools/flyp/test/*Suite.fly, the unit tests of the
# flyp package manager (Fly port). Each suite is the entry; `--src-dir tools/flyp`
# pulls the flyp modules (flyp.toml, flyp.manifest, flyp.resolver, …) into one
# module while std namespaces stay archive-linked (the -L pass registers them).
# `--test` builds in test mode; the resulting executable is then run.
#
# Kept separate from test_compiler.sh (which tests the compiler suites) and wired
# as its own workflow step, mirroring the compiler test job.
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

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
for suite in $(find tools/flyp/test -name '*Suite.fly' 2>/dev/null | sort); do
    name=$(basename "$suite" .fly)
    bin="$OUT/flyp_$name"
    if ! "$FLY" "$suite" --test --src-dir tools/flyp -o "flyp_$name" --out-dir "$OUT" -L "$STD" >"$OUT/_flyp_$name.log" 2>&1; then
        echo "  COMPILE FAIL  flyp/$name"
        grep -m3 -E 'error:|broken|abort' "$OUT/_flyp_$name.log" | sed 's/^/      /'
        fail=$((fail + 1))
        continue
    fi
    if "$bin" >"$OUT/_flyp_$name.run" 2>&1; then
        echo "  PASS          flyp/$name"
        pass=$((pass + 1))
    else
        echo "  RUN  FAIL     flyp/$name (exit $?)"
        tail -5 "$OUT/_flyp_$name.run" | sed 's/^/      /'
        fail=$((fail + 1))
    fi
done

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
