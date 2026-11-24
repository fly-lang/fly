#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# stage2.sh — stage 2 of the staged bootstrap (see the stage map in stage1.sh):
# the fly built by stage1 (build/stage1/bin/fly) produces the SHIPPED artifacts
# into build/stage2/ — this is what CI uploads and the release packages.
#
# From 0.16 the whole stage is one `fly build` on the root Manifest.fly
# (build_stage2.sh, shared with repro_check.sh):
#
#   build/stage1/bin/fly build --profile dist[-debug] --out-dir build/stage2 --keep-objects
#
# runtime, TLS archives, std, compiler, fly-lsp and fly-registry, all compiled
# by the stage-1 fly, plus the LLVM pieces stage 0 fetched (libLLVM, ld.lld,
# lldb, compiler-rt builtins, llvm.fly.h), laid out as the release: bin/ and
# lib/, the executables finding lib/ through $ORIGIN/../lib. Up to 0.15 the
# stage ran build_runtime, build_std, build_compiler, link_fly, build_lsp and
# build_registry; the Manifest now says what they said.
#
# A failed stage-2 build is FATAL: there is no fallback to the stage-1
# artifacts. build/stage2 is what the release packages verbatim.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
export STAGE=2

./ci/linux/build_stage2.sh

# ── the guards: properties of the artifact no suite can see ──────────────────
#
# The tools are PRODUCTS of the toolchain and ship from build/stage2/bin; a tool
# failure here is a real failure of the artifact.
# test_dbg: the debugger is PROVISIONED (copied by the Manifest's copy targets
# from the fork LLVM), not compiled — but it ships from build/stage2/bin like
# the tools, so it is verified here with them.
# test_abi: the calling conventions of the compiler that ships must match the
# seed's (a library and a program compiled by different releases link together).
# test_sema: the compiler must still REJECT what the language forbids.
# test_traps: a run-time error must stop the program with its diagnostic.
# test_leaks: programs that free their temporaries stay under a memory ceiling.
# test_fly_build: build/stage2 is a toolchain of its own (bare environment, no
# option), and a project built with `fly build` reads its own manifest.
./ci/linux/test_lsp.sh
./ci/linux/test_registry.sh
./ci/linux/test_tools.sh
./ci/linux/test_abi.sh
./ci/linux/test_sema.sh
./ci/linux/test_traps.sh
./ci/linux/test_leaks.sh
./ci/linux/test_dbg.sh
bash ./ci/linux/test_fly_build.sh

echo "stage2: done — build/stage2/bin/fly"
