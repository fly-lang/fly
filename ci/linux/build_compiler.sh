#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_compiler.sh — build the compiler library from compiler/lib sources into
# build/stage1/compiler (fly_compiler_lib.a + one .fly.h per module; build-only,
# never shipped). STAGE 1 ONLY for now: the self-host cannot yet compile its own
# sources (a future stage-2 rebuild would make the shipped compiler fully
# self-hosted — like Rust's beta-built rustc). Requires build_runtime.sh +
# build_std.sh (stage 1) first — the compiler is compiled against the IN-TREE
# std/runtime headers so its symbol references match the std the final binary
# links. See stage1.sh for the stage map.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

# ── Stage plumbing (STAGE forced to 1 — see header). ──────────────────────────
STAGE=1
LIB=build/stage1/lib
CDIR=build/stage1/compiler
FLY=build/stage1/bin/fly0               # stage0 hardlink: <exe>/../lib → build/stage1/lib
[ -x build/stage0/bin/fly ] || { echo "error: stage0 compiler missing — run ci/linux/stage0.sh first." >&2; exit 1; }
mkdir -p build/stage1/bin
ln -f build/stage0/bin/fly "$FLY" 2>/dev/null || cp -f build/stage0/bin/fly "$FLY"

[ -f "$LIB/fly_std_lib.a" ] || { echo "error: std missing in $LIB — run build_runtime.sh + build_std.sh (stage 1) first." >&2; exit 1; }
mkdir -p "$CDIR"

mapfile -t FILES < <(find compiler/lib -name '*.fly' | sort)
echo "stage1: compiling ${#FILES[@]} compiler/lib files ..."
"$FLY" --lib -o "$CDIR/fly_compiler_lib" "${FILES[@]}"

# headers (nested `>>` spaced so re-reads lex them; idempotent)
for h in "$CDIR"/*.fly.h; do
    sed -i -E ':a;s/>>/> >/;ta' "$h"
done

echo "stage1: compiler -> $CDIR/fly_compiler_lib.a (+ $(ls "$CDIR"/*.fly.h | wc -l) *.fly.h)"
