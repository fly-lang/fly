#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_tools_flyp.sh — build the `flyp` package manager (Fly port, tools/flyp).
#
# Runs AFTER build_compiler.sh, which already produced build/lib (the compiled
# std archive + headers) and build/bin/fly (the self-host compiler). flyp is a
# plain Fly program: it imports the std and the tools/flyp modules, and at run
# time shells out to the sibling `fly` compiler — so it links only the std
# archives (no libLLVM) and ships next to `fly` in bin/.
#
# The self-host `fly` cannot compile flyp itself (it does not pull generic
# modules through --src-dir), so — like the compiler build — we drive the build
# with a COPY of the bootstrap $FLY run from build/bin, so <exe>/../lib resolves
# to build/lib and flyp links the SAME std that ships in the release.
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

OUT=build/bin
LIB=build/lib

# Bootstrap compiler via $FLY (default `fly` on PATH). A slash-less name is
# resolved through PATH into an absolute path (argv[0]-based stdlib lookup). Same
# handling as build_compiler.sh.
FLY="${FLY:-fly}"
case "$FLY" in
    */*) ;;
    *)
        FLY_RESOLVED="$(command -v "$FLY" || true)"
        if [ -z "$FLY_RESOLVED" ]; then
            echo "error: bootstrap compiler '$FLY' not found on PATH." >&2
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

# The compiled std must already exist (build_compiler.sh runs first).
if [ ! -f "$LIB/fly_std_lib.a" ]; then
    echo "error: $LIB/fly_std_lib.a not found — run ci/linux/build_compiler.sh first." >&2
    exit 1
fi

# Build flyp with a bootstrap copy run from build/bin so <exe>/../lib == build/lib
# (auto-discovery links our fly_std_lib.a + fly_runtime_lib.a). A distinct copy
# name avoids clobbering the installed self-host build/bin/fly; the running copy
# is never the output file, so no staging dir is needed here.
BOOTSTRAP_COPY="$OUT/_flyp_cc"
cp "$FLY" "$BOOTSTRAP_COPY"
"$BOOTSTRAP_COPY" tools/flyp/Flyp.fly \
    --src-dir tools/flyp \
    -o flyp --out-dir "$OUT"
rm -f "$BOOTSTRAP_COPY"
rm -f "$OUT"/*.o

if [ ! -x "$OUT/flyp" ]; then
    echo "error: flyp was not built at $OUT/flyp." >&2
    exit 1
fi
echo "flyp -> $OUT/flyp"
