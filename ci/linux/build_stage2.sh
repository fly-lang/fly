#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_stage2.sh — build stage 2 (0.16): the stage-1 fly runs
#
#   fly build --profile dist[-debug] --out-dir build/stage2 --keep-objects
#
# on the root Manifest.fly: runtime, TLS archives, std, compiler, fly-lsp and
# fly-registry, plus the LLVM pieces stage 0 fetched, laid out as the release
# (bin/ and lib/). `dist-debug` when FLY_DEBUG_SYMBOLS=1. The executables'
# objects (--keep-objects) are moved to build/stage2/obj, outside what the
# release packages: stage3.sh compares obj/fly.o with its own.
#
# Shared by stage2.sh and repro_check.sh (which builds stage 2 twice). Linux
# and macOS; Windows mirror: ci/windows/build_stage2.ps1.
# Exit 0 = build/stage2 complete.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

FLY1=build/stage1/bin/fly
[ -x "$FLY1" ] || { echo "error: stage1 fly missing — run ci/linux/stage1.sh first." >&2; exit 1; }
PROFILE=dist; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && PROFILE=dist-debug

rm -rf build/stage2
echo "stage2: fly build --profile $PROFILE --out-dir build/stage2 (the stage-1 fly) ..."
start=$SECONDS
"$FLY1" build --profile "$PROFILE" --out-dir build/stage2 --keep-objects \
    || { echo "error: stage2 fly build failed" >&2; exit 1; }
echo "stage2: built in $((SECONDS - start))s"

# The stage is only meaningful if it produced ITS OWN compiler.
for need in build/stage2/bin/fly build/stage2/bin/fly-lsp build/stage2/bin/fly-registry \
            build/stage2/bin/fly.o build/stage2/lib/fly_std_lib.a; do
    [ -f "$need" ] || { echo "error: stage2 build reported success but $need is missing." >&2; exit 1; }
done
mkdir -p build/stage2/obj
mv -f build/stage2/bin/*.o build/stage2/obj/
