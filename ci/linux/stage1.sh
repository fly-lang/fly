#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# stage1.sh — stage 1 of the Rust-style staged bootstrap:
#
#   stage0  downloaded bootstrap compiler + precompiled lib (build/stage0,
#           set up by stage0.sh — which also fetches the fork LLVM).
#   stage1  THIS SCRIPT: stage0 builds runtime → std → compiler → driver FROM
#           IN-TREE SOURCES and links the first self-host fly, all into
#           build/stage1/. The compiler is built against the in-tree std
#           headers so its symbol references match the std the binary links.
#   stage2  stage2.sh: build/stage1/bin/fly rebuilds the shipped artifacts
#           into build/stage2/ (the self-hosting fixpoint check).
#
# Each build_*.sh derives its compiler and dirs from $STAGE. At stage 1 the
# stage0 binary is hardlinked to build/stage1/bin/fly0 so its /proc/self/exe
# based <exe>/../lib discovery serves build/stage1/lib, never the bootstrap's
# own lib. FLY_BUNDLE_LLVM=1 is honoured by link_fly.sh (self-contained stage).
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
export STAGE=1

./ci/linux/build_runtime.sh
./ci/linux/build_std.sh
./ci/linux/build_compiler.sh
./ci/linux/link_fly.sh

echo "stage1: done — build/stage1/bin/fly"
