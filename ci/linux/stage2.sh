#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# stage2.sh — stage 2 of the staged bootstrap (see the stage map in stage1.sh):
# the fly built by stage1 (build/stage1/bin/fly) produces the SHIPPED artifacts
# into build/stage2/ — this is what CI uploads and the release packages.
#
#   runtime   recompiled by the stage1 fly (self-hosting fixpoint check)
#   std       recompiled by the stage1 fly — the std that ships
#   compiler  recompiled by the stage1 fly (compiler + driver, one object) and
#   + driver  linked into build/stage2/bin/fly.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
export STAGE=2

mkdir -p build/stage2/lib
./ci/linux/build_runtime.sh
# The std that ships, built by the stage-1 fly like every other stage-2 artifact
# (it used to be the seed's stage-1 build, copied over).
./ci/linux/build_std.sh
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
# ABI guard: the shipped pair is a SELF-HOST compiler over a SEED-built std, so
# the two compilers' calling conventions must match. No suite can see this — a
# suite compiles both sides with one compiler.
./ci/linux/test_abi.sh
# Semantic guard: the compiler must still REJECT what the language forbids. No
# suite can see a check that went missing - suites only compile valid programs.
./ci/linux/test_sema.sh
# Trap guard: a run-time error (a null reference) must stop the program with its
# diagnostic. No suite can see a trap that went missing - suites run valid programs.
./ci/linux/test_traps.sh
# Leak guard: the probes are compiled by the stage2 fly, the one that ships.
./ci/linux/test_leaks.sh
# The debugger is PROVISIONED (bundled by link_fly.sh from the fork LLVM), not
# compiled — but it ships from build/stage2/bin like the tools, so verify it here.
./ci/linux/test_dbg.sh

echo "stage2: done — build/stage2/bin/fly"
