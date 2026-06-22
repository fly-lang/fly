#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# compiler.sh — build the self-host compiler EXECUTABLE `flyc`.
#
# Like build.sh, but the entry is compiler/Fly.fly (which has `void main`) and we
# don't pass --lib, so the reference toolchain links an executable. --src-dir
# indexes the whole fly.compiler graph (driver → frontend → parser/sema/codegen) and
# pulls in every file — that's fine: each `main` is in its own namespace, so the
# entry file's main becomes the C entry without clashing with test mains. -L resolves
# std. (Other builds use Parser.fly --lib as the entry, so this main isn't theirs.)
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")"

FLY="${FLY:-../fly/cmake-build-relwithdebinfo/bin/fly}"
OUT=target/debug
STD=std/lib
mkdir -p "$OUT"

"$FLY" compiler/Fly.fly \
    --src-dir compiler \
    -o fly --out-dir "$OUT" -L "$STD"

echo "fly -> $OUT/fly"
