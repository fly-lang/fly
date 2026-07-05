#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# install_prerequisites.sh - fetch/configure what the self-host build needs on
# Linux: LLVM 20 (system, via apt) and the bootstrap `fly` compiler.
#
# Linux counterpart of ci/windows/install_prerequisites.ps1. The workflow calls
# this; run it yourself to reproduce CI locally.
#
# Local use: SOURCE it so FLY/PATH persist in your shell, then build:
#     . ./ci/linux/install_prerequisites.sh
#     ./ci/linux/build_compiler.sh
#
# In CI ($GITHUB_ENV set) it appends to $GITHUB_ENV / $GITHUB_PATH instead of the
# process environment - auto-detected below.
# -----------------------------------------------------------------------------

# Fail fast only when executed (not sourced): `set -e` in a sourced script would
# kill the caller's interactive shell on any error.
(return 0 2>/dev/null) && SOURCED=1 || SOURCED=0
[ "$SOURCED" -eq 0 ] && set -euo pipefail

# Resolve the project root (this script lives in ci/linux/, two levels down).
# BASH_SOURCE works whether the script is executed or sourced.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Bootstrap compiler release used to compile the std --lib archive + the self-host
# sources. Must ship the ptrsize header-gen fix (fly Frontend.cpp typeStr).
FLY_VERSION="${FLY_VERSION:-0.13.8}"

BUILD_DIR="$ROOT/build"
FLY_DIR="$BUILD_DIR/bootstrap"
FLY_BIN="$FLY_DIR/bin/fly"

# --- LLVM 20 (system, apt) — dev build + object emit -------------------------
# apt's LLVM is used only by the DEFAULT dev build and by step 3 of the release
# build (the bootstrap links the throwaway staging fly via -lLLVM-20 while emitting
# the combined object). `llvm-20-dev`+`libllvm20` give libLLVM-20.so; `lld-20` the
# linker; `clang` drives the bundle relink. The RELEASE artifact itself never links
# apt's libLLVM — that comes from the fork tree below (libxml2 OFF). Install only if
# missing and apt-get is available (Ubuntu/Debian CI); on other distros install the
# equivalent yourself.
if ! command -v llvm-config-20 >/dev/null 2>&1 && command -v apt-get >/dev/null 2>&1; then
    sudo apt-get update
    sudo apt-get install -y llvm-20-dev libllvm20 lld-20 clang-20
fi

# --- Fork LLVM (libxml2 OFF, shared libLLVM.so) — release build only ----------
# apt's libLLVM.so drags libxml2/libzstd/… as external deps; on a host whose
# libxml2 soname differs (e.g. libxml2.so.16 vs .2) the bundled release fails to
# load. The fork build (fly-lang/llvm-project: LLVM_BUILD_LLVM_DYLIB=ON +
# LLVM_ENABLE_LIBXML2=OFF) ships a libLLVM.so with NO external deps, so a release
# that bundles IT runs anywhere. Fetched only for the self-contained build
# (FLY_BUNDLE_LLVM=1); ~950 MB, so cache build/llvm in CI. build_compiler.sh's
# bundle branch links + ships this libLLVM.so (and builds fly-lld against it).
LLVM_VERSION="${LLVM_VERSION:-20.1.8}"
FORK_LLVM="$BUILD_DIR/llvm"
if [ "${FLY_BUNDLE_LLVM:-0}" = "1" ] && [ ! -f "$FORK_LLVM/lib/libLLVM.so" ]; then
    url="https://github.com/fly-lang/llvm-project/releases/download/v${LLVM_VERSION}-linux-x86_64/llvm-${LLVM_VERSION}-x86_64-linux-gnu.tar.gz"
    mkdir -p "$BUILD_DIR"
    curl -fsSL "$url" -o "$BUILD_DIR/fork-llvm.tar.gz"
    # Extract only what the bundle needs (the shared libLLVM.so, the lld static
    # archives, and the lld/LLVM headers) — ~200 MB vs ~4 GB for the full tree.
    tar -xzf "$BUILD_DIR/fork-llvm.tar.gz" -C "$BUILD_DIR" --wildcards \
        'llvm/lib/libLLVM.so*' 'llvm/lib/liblld*.a' llvm/include   # → $BUILD_DIR/llvm/
    rm -f "$BUILD_DIR/fork-llvm.tar.gz"
fi

# --- Download fly binary -----------------------------------------------------
# Skip if already present (local convenience; a fresh CI runner never has it).
if [ ! -x "$FLY_BIN" ]; then
    url="https://github.com/fly-lang/fly/releases/download/v${FLY_VERSION}/fly-${FLY_VERSION}-linux-x86_64.tar.gz"
    mkdir -p "$FLY_DIR"
    curl -fsSL "$url" -o "$BUILD_DIR/fly.tar.gz"
    tar -xzf "$BUILD_DIR/fly.tar.gz" -C "$FLY_DIR"
    rm -f "$BUILD_DIR/fly.tar.gz"
    chmod +x "$FLY_BIN"
fi

# --- Environment -------------------------------------------------------------
# Use an absolute $FLY path (not a bare `fly` on PATH): released compilers derive
# their stdlib dir from the executable path; a bare name resolved from a cwd that
# contains a `fly/`-like dir breaks that lookup. build/test scripts honour $FLY.
if [ -n "${GITHUB_ENV:-}" ]; then
    echo "FLY=$FLY_BIN"  >> "$GITHUB_ENV"
    echo "$FLY_DIR/bin"  >> "$GITHUB_PATH"
else
    export FLY="$FLY_BIN"
    export PATH="$FLY_DIR/bin:$PATH"
    echo "Prerequisites configured for this session:"
    echo "  fly $FLY_VERSION -> FLY = $FLY_BIN ; PATH += $FLY_DIR/bin"
    echo "Note: source this script (. ./ci/linux/install_prerequisites.sh) for FLY/PATH to persist before running ./ci/linux/build_compiler.sh"
fi
