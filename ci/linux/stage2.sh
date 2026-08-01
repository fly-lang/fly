#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# stage2.sh — stage 2 of the staged bootstrap (see the stage map in stage1.sh):
# the fly built by stage1 (build/stage1/bin/fly) produces the SHIPPED artifacts
# into build/stage2/ — this is what CI uploads and the release packages.
#
#   runtime   recompiled by the stage1 fly (self-hosting fixpoint check)
#   std       COPIED from stage1: the shipped std must keep the reference class
#             ABI — the self-host lays out based classes flat, which would break
#             every header consumer of a class with an interface base. Build it
#             with STAGE=2 build_std.sh once the self-host adopts that ABI.
#   compiler  stays the stage1 (stage0-built) archive, like Rust's beta-built
#             rustc — linked in, never shipped as an artifact.
#   driver    recompiled by the stage1 fly and linked into build/stage2/bin/fly.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
export STAGE=2

# std: ship the stage1 (reference-ABI) build — archive + generated headers.
# Copied BEFORE the runtime build so the stage-2 runtime.fly.h/llvm.fly.h win.
mkdir -p build/stage2/lib
cp -f build/stage1/lib/fly_std_lib.a build/stage2/lib/
cp -f build/stage1/lib/*.fly.h build/stage2/lib/
echo "stage2: std -> build/stage2/lib (copied from stage1, reference ABI)"

./ci/linux/build_runtime.sh
./ci/linux/build_compiler.sh
./ci/linux/link_fly.sh

# ── the tools: built and tested with the compiler that just finished ─────────
#
# fly-lsp and fly-registry are PRODUCTS of the toolchain, not part of the
# bootstrap: nothing downstream compiles against them, and --entry is a
# self-host option the pinned seed rejects outright. They belong HERE rather
# than in stage1 because the compiler that builds them should be the one that
# ships — stage2's fly, built by stage1's self-host, i.e. the self-hosting
# fixpoint. A tool built at stage1 would carry the seed-built compiler's
# codegen, which is not what a user gets.
#
# They are part of the stage, not an opt-in extra: build/stage2/bin is what the
# release packages verbatim, so a tool that is not built here does not ship, and
# a tool failure here is a real failure of the artifact.
./ci/linux/build_lsp.sh
./ci/linux/test_lsp.sh
./ci/linux/build_registry.sh
./ci/linux/test_registry.sh
./ci/linux/test_tools.sh
# The debugger is PROVISIONED (bundled by link_fly.sh from the fork LLVM), not
# compiled — but it ships from build/stage2/bin like the tools, so verify it here.
./ci/linux/test_dbg.sh

echo "stage2: done — build/stage2/bin/fly"
