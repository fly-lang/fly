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

# --- LLVM 20 (apt) -------------------------------------------------------------
# The fly toolchain links the compiler-rt builtins (libclang_rt.builtins) from the
# SYSTEM LLVM 20 into every executable it produces (ToolChain probes
# /usr/lib/llvm-20 — see GetCompilerRTBuiltinsPath). Dev machines with apt.llvm.org
# LLVM 20 already have it; fresh CI runners don't, so install it here (this is the
# "Install LLVM 20 (apt, …)" the workflow step documents).
LLVM_MAJOR=20
if [ ! -d "/usr/lib/llvm-${LLVM_MAJOR}" ]; then
    echo "installing LLVM ${LLVM_MAJOR} via apt.llvm.org (compiler-rt builtins for linking) ..."
    curl -fsSL https://apt.llvm.org/llvm.sh -o /tmp/llvm.sh
    chmod +x /tmp/llvm.sh
    sudo /tmp/llvm.sh "$LLVM_MAJOR"
    sudo apt-get install -y "libclang-rt-${LLVM_MAJOR}-dev"
fi

# --- LLVM 20 (fly-lang/llvm-project fork, downloaded — NO system package manager) --
# Mirrors ci/windows/install_prerequisites.ps1: the LLVM the self-host compiler links
# against comes from the project's OWN LLVM build (fly-lang/llvm-project release), NOT
# apt. apt's libLLVM.so drags libxml2/libzstd/… as external deps; on a host whose
# libxml2 soname differs (e.g. libxml2.so.16 vs .2) the bundled release fails to load.
# The fork build (LLVM_BUILD_LLVM_DYLIB=ON + LLVM_ENABLE_LIBXML2=OFF) ships a libLLVM.so
# with NO external deps, plus lld (ld.lld) and llvm-config. It does NOT ship clang, so
# the FLY_BUNDLE_LLVM relink uses the host C++ driver (clang++ or g++, see build_compiler.sh).
# The tarball is ~1 GB; cache build/llvm in CI. Always fetched (dev + release use the fork
# now); the bundle build additionally needs the lld static archives + LLVM/lld headers.
LLVM_VERSION="${LLVM_VERSION:-20.1.8}"
FORK_LLVM="$BUILD_DIR/llvm"
NEED_STATIC=0
[ "${FLY_BUNDLE_LLVM:-0}" = "1" ] && [ ! -f "$FORK_LLVM/lib/liblldELF.a" ] && NEED_STATIC=1
if [ ! -f "$FORK_LLVM/lib/libLLVM.so" ] || [ "$NEED_STATIC" = "1" ]; then
    url="https://github.com/fly-lang/llvm-project/releases/download/v${LLVM_VERSION}-linux-x86_64/llvm-${LLVM_VERSION}-x86_64-linux-gnu.tar.gz"
    mkdir -p "$BUILD_DIR"
    tarball="$BUILD_DIR/fork-llvm.tar.gz"
    [ -f "$tarball" ] || curl -fsSL "$url" -o "$tarball"
    # Always: the shared libLLVM.so (link + runtime), the lld linker and llvm-config.
    # Bundle also: the lld static archives + LLVM/lld headers (build_compiler.sh links
    # a small fly-lld against them).
    paths="llvm/lib/libLLVM.so* llvm/bin/ld.lld llvm/bin/lld llvm/bin/llvm-config"
    [ "${FLY_BUNDLE_LLVM:-0}" = "1" ] && paths="$paths llvm/lib/liblld*.a llvm/include"
    # shellcheck disable=SC2086
    tar -xzf "$tarball" -C "$BUILD_DIR" --wildcards $paths   # → $BUILD_DIR/llvm/
    rm -f "$tarball"
fi
# The codegen bridge emits `-lLLVM-20` (see compiler/lib/codegen/LLVMApi.fly); the fork
# ships the shared lib as `libLLVM.so`, so alias it to the `libLLVM-20.so` link name.
[ -e "$FORK_LLVM/lib/libLLVM-20.so" ] || ln -sf libLLVM.so "$FORK_LLVM/lib/libLLVM-20.so"

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
#
# The fork LLVM replaces apt: put its bin on PATH (ld.lld, llvm-config) and its lib
# dir on LIBRARY_PATH (so the `-lLLVM-20` link finds libLLVM-20.so) and LD_LIBRARY_PATH
# (so the dynamically-linked staged `fly` loads libLLVM.so.20.1 at run time). The
# self-contained release re-links against the fork tree with an explicit rpath, so this
# only matters for the plain dev build and for the intermediate staged link.
if [ -n "${GITHUB_ENV:-}" ]; then
    echo "FLY=$FLY_BIN"                                >> "$GITHUB_ENV"
    echo "$FLY_DIR/bin"                                >> "$GITHUB_PATH"
    echo "$FORK_LLVM/bin"                              >> "$GITHUB_PATH"
    echo "LIBRARY_PATH=$FORK_LLVM/lib:${LIBRARY_PATH:-}"       >> "$GITHUB_ENV"
    echo "LD_LIBRARY_PATH=$FORK_LLVM/lib:${LD_LIBRARY_PATH:-}" >> "$GITHUB_ENV"
else
    export FLY="$FLY_BIN"
    export PATH="$FLY_DIR/bin:$FORK_LLVM/bin:$PATH"
    export LIBRARY_PATH="$FORK_LLVM/lib:${LIBRARY_PATH:-}"
    export LD_LIBRARY_PATH="$FORK_LLVM/lib:${LD_LIBRARY_PATH:-}"
    echo "Prerequisites configured for this session:"
    echo "  fly $FLY_VERSION -> FLY = $FLY_BIN ; PATH += $FLY_DIR/bin"
    echo "  LLVM $LLVM_VERSION (fork) -> PATH += $FORK_LLVM/bin ; LIBRARY_PATH/LD_LIBRARY_PATH += $FORK_LLVM/lib"
    echo "Note: source this script (. ./ci/linux/install_prerequisites.sh) for FLY/PATH to persist before running ./ci/linux/build_compiler.sh"
fi
