#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build.sh — build the Fly compiler library without flyp / fly.toml.
#
# Single-file build: only the root entry (Parser.fly) is passed; `--src-dir`
# resolves the whole `fly.compiler` dependency graph from its `import` statements
# and lowers every reachable source into one module (so cross-file references and
# the native enums Prec/TokenKind resolve). No file list, no concatenation.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")"

FLY="${FLY:-../fly/cmake-build-relwithdebinfo/bin/fly}"
OUT=target/debug
STD=std/lib
mkdir -p "$OUT"

# Parser.fly imports `fly.compiler.*`; --src-dir compiler indexes every .fly under
# compiler/ by namespace and pulls in all of them. --lib + -o keep a stable,
# descriptive artifact name; --out-dir sends the archive, headers AND intermediate
# objects into $OUT; -L resolves the std library.
"$FLY" compiler/parser/Parser.fly \
    --lib --src-dir compiler \
    -o fly_compiler_lib.a --out-dir "$OUT" -L "$STD"

echo "fly_compiler_lib.a -> $OUT/fly_compiler_lib.a"
