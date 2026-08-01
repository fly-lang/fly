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
#           Build and test are INTERLEAVED: the seed (fly0) runs the runtime
#           and std suites against the freshly built build/stage1/lib BEFORE
#           the compiler is built, so a library defect is caught before a
#           compiler build is spent on it. Both suite sets run after build_std:
#           the runtime suites import fly.assert/fly.mem, so the std archive
#           must exist for their link (-L std/lib alone is declarations-only).
#   stage2  stage2.sh: build/stage1/bin/fly rebuilds the shipped artifacts
#           into build/stage2/ (the self-hosting fixpoint check).
#
# Each build_*.sh derives its compiler and dirs from $STAGE. At stage 1 the
# stage0 binary is hardlinked to build/stage1/bin/fly0 so its /proc/self/exe
# based <exe>/../lib discovery serves build/stage1/lib, never the bootstrap's
# own lib. The test steps must run that same fly0 — build/stage1/bin/fly does
# not exist until link_fly. FLY_STAGE1_TESTS=0 skips them (fast local builds).
# FLY_BUNDLE_LLVM=1 is honoured by link_fly.sh (self-contained stage).
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
export STAGE=1

FLY0=build/stage1/bin/fly0        # seed hardlink, created by build_runtime.sh
STAGE1_TESTS="${FLY_STAGE1_TESTS:-1}"

./ci/linux/build_runtime.sh
./ci/linux/build_std.sh
[ "$STAGE1_TESTS" = 0 ] || FLY="$FLY0" ./ci/linux/test_runtime.sh
# Per-suite, not one-shot: the bare `--suite` all-in-one-binary run is a
# SELF-HOST semantic — the reference seed builds it but reports only the
# first suite (exit 0, everything else unreported).
[ "$STAGE1_TESTS" = 0 ] || FLY="$FLY0" FLY_TEST_PER_SUITE=1 ./ci/linux/test_std.sh
./ci/linux/build_compiler.sh
./ci/linux/link_fly.sh

# The tools (fly-lsp, fly-registry) are built and tested at STAGE 2, not here —
# see the tail of stage2.sh. They are PRODUCTS of the toolchain, so the compiler
# that builds them should be the one that ships: stage2's fly, the self-hosting
# fixpoint. Building them here would use the stage1 binary, which the seed
# produced.

echo "stage1: done — build/stage1/bin/fly"
