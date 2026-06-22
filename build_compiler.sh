#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# compiler.sh — build the self-host compiler EXECUTABLE `fly`.
#
# The entry is compiler/Fly.fly (which has `void main`); without --lib the
# reference toolchain links an executable. --src-dir indexes the whole
# fly.compiler graph (driver → frontend → parser/sema/codegen) and pulls in every
# file — that's fine: each `main` is in its own namespace, so the entry file's main
# becomes the C entry without clashing with test mains. -L resolves std.
#
# This is the single build script: compiling the executable compile-checks every
# compiler source (and links), so it subsumes the old --lib-only build.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")"

FLY="${FLY:-../fly/cmake-build-relwithdebinfo/bin/fly}"
# build/bin is the release-artifact path the workflows (build-linux.yml →
# release.yml) upload and package; keep emitting the `fly` binary there.
OUT=build/bin
STD=std/lib
mkdir -p "$OUT"

"$FLY" compiler/Fly.fly \
    --src-dir compiler \
    -o fly --out-dir "$OUT" -L "$STD"

echo "fly -> $OUT/fly"

