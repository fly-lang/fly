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

#PATH="$PATH:/home/marco/Projects/flylang/fly/build/bin"

set -euo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

# build/bin is the release-artifact path the workflows (build-linux.yml →
# release.yml) upload and package; keep emitting the `fly` binary there.
OUT=build/bin
STD=std/lib
mkdir -p "$OUT"

# Invoke the bootstrap compiler via $FLY (default: `fly` on PATH). CI sets FLY to
# an absolute path: released binaries derive their stdlib dir from the executable
# path, and a bare `fly` resolved from a cwd containing a `fly/` directory fails
# that lookup. A slash-containing path sidesteps it. See ../fly Driver.cpp.
FLY="${FLY:-fly}"

"$FLY" compiler/Fly.fly \
    --src-dir compiler \
    -o fly --out-dir "$OUT" -L "$STD"

echo "fly -> $OUT/fly"

