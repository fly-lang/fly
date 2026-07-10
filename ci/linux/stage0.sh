#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# stage0.sh — set up stage 0 of the staged bootstrap on Linux:
#
#   • the fork LLVM toolchain (fly-lang/llvm-project release) → build/llvm
#     libLLVM.so + ld.lld + llvm-config + compiler-rt builtins. NO system
#     package manager: everything the build needs comes from this tarball.
#   • the bootstrap `fly` 0.13.8 release → build/stage0 (bin/ + precompiled lib/)
#
# stage1.sh / stage2.sh build on top of these (see the stage map in stage1.sh).
#
# Local use: SOURCE it so PATH/env persist in your shell:
#     . ./ci/linux/stage0.sh
#     ./ci/linux/stage1.sh && ./ci/linux/stage2.sh
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
STAGE0_DIR="$BUILD_DIR/stage0"
FLY_BIN="$STAGE0_DIR/bin/fly"

# --- LLVM 20 (fly-lang/llvm-project fork, downloaded — NO package manager) ------
# Everything comes from the project's OWN LLVM build (LLVM_BUILD_LLVM_DYLIB=ON +
# LLVM_ENABLE_LIBXML2=OFF + compiler-rt): a libLLVM.so with no external deps, the
# lld linker, and libclang_rt.builtins.a — which the fly ToolChain links into every
# executable it produces (this used to come from an apt LLVM; see the symlink below).
# The tarball is ~1 GB; cache build/llvm in CI. The bundle build additionally needs
# the lld static archives + LLVM/lld headers.
LLVM_VERSION="${LLVM_VERSION:-20.1.8}"
FORK_LLVM="$BUILD_DIR/llvm"
RT_BUILTINS="$FORK_LLVM/lib/clang/${LLVM_VERSION%%.*}/lib/x86_64-unknown-linux-gnu/libclang_rt.builtins.a"
NEED_STATIC=0
[ "${FLY_BUNDLE_LLVM:-0}" = "1" ] && [ ! -f "$FORK_LLVM/lib/liblldELF.a" ] && NEED_STATIC=1
if [ ! -f "$FORK_LLVM/lib/libLLVM.so" ] || [ ! -f "$RT_BUILTINS" ] || [ "$NEED_STATIC" = "1" ]; then
    url="https://github.com/fly-lang/llvm-project/releases/download/v${LLVM_VERSION}-linux-x86_64/llvm-${LLVM_VERSION}-x86_64-linux-gnu.tar.gz"
    mkdir -p "$BUILD_DIR"
    tarball="$BUILD_DIR/fork-llvm.tar.gz"
    # Atomic download (.part + mv, with retries): an interrupted curl must never
    # leave a truncated tarball that a rerun would mistake for the real one.
    if [ ! -f "$tarball" ]; then
        curl -fSL --retry 3 --retry-all-errors -o "$tarball.part" "$url"
        mv -f "$tarball.part" "$tarball"
    fi
    # Always: the shared libLLVM.so (link + runtime), the lld linker, llvm-config
    # and the compiler-rt builtins (linked into every fly-produced executable).
    # Bundle also: the lld static archives + LLVM/lld headers.
    paths="llvm/lib/libLLVM.so* llvm/bin/ld.lld llvm/bin/lld llvm/bin/llvm-config llvm/lib/clang"
    [ "${FLY_BUNDLE_LLVM:-0}" = "1" ] && paths="$paths llvm/lib/liblld*.a llvm/include"
    # shellcheck disable=SC2086
    tar -xzf "$tarball" -C "$BUILD_DIR" --wildcards $paths   # → $BUILD_DIR/llvm/
    rm -f "$tarball"
fi
# The codegen bridge emits `-lLLVM-20` (see compiler/lib/codegen/LLVMApi.fly); the fork
# ships the shared lib as `libLLVM.so`, so alias it to the `libLLVM-20.so` link name.
[ -e "$FORK_LLVM/lib/libLLVM-20.so" ] || ln -sf libLLVM.so "$FORK_LLVM/lib/libLLVM-20.so"

# compiler-rt builtins are a hard link-time requirement (GetCompilerRTBuiltinsPath
# errors out without them) — fail loudly here rather than at the first test link.
if [ ! -f "$RT_BUILTINS" ]; then
    echo "error: $RT_BUILTINS missing — the fork LLVM tarball did not provide compiler-rt." >&2
    echo "       (delete build/llvm and rerun; if it persists the fork release lacks compiler-rt)" >&2
    [ "$SOURCED" -eq 0 ] && exit 1 || return 1
fi

# Both ToolChains (self-host driver/lib/ToolChain.fly and the bootstrap's C++ one)
# probe /usr/lib/llvm-20 for libclang_rt.builtins. On a host without an LLVM 20
# install, point that path at the fork tree (this replaces the old apt install).
if [ ! -e "/usr/lib/llvm-${LLVM_VERSION%%.*}" ]; then
    echo "linking /usr/lib/llvm-${LLVM_VERSION%%.*} -> $FORK_LLVM (compiler-rt builtins for the ToolChain probes)"
    sudo ln -sfn "$FORK_LLVM" "/usr/lib/llvm-${LLVM_VERSION%%.*}"
fi

# --- Download the bootstrap fly (stage 0) --------------------------------------
# Skip if already present (local convenience; a fresh CI runner never has it).
if [ ! -x "$FLY_BIN" ]; then
    url="https://github.com/fly-lang/fly/releases/download/v${FLY_VERSION}/fly-${FLY_VERSION}-linux-x86_64.tar.gz"
    mkdir -p "$STAGE0_DIR"
    curl -fsSL --retry 3 --retry-all-errors "$url" -o "$BUILD_DIR/fly.tar.gz"
    tar -xzf "$BUILD_DIR/fly.tar.gz" -C "$STAGE0_DIR"
    rm -f "$BUILD_DIR/fly.tar.gz"
    chmod +x "$FLY_BIN"
fi

# --- Environment -------------------------------------------------------------
# The build/test scripts derive their compiler from $STAGE (see stage1.sh /
# stage2.sh) — no $FLY export needed. Only the fork LLVM is wired up: its bin on
# PATH (ld.lld, llvm-config), its lib on LIBRARY_PATH (the `-lLLVM-20` link) and
# LD_LIBRARY_PATH (the non-bundled staged `fly` loads libLLVM.so at run time).
if [ -n "${GITHUB_ENV:-}" ]; then
    echo "$FORK_LLVM/bin"                              >> "$GITHUB_PATH"
    echo "LIBRARY_PATH=$FORK_LLVM/lib:${LIBRARY_PATH:-}"       >> "$GITHUB_ENV"
    echo "LD_LIBRARY_PATH=$FORK_LLVM/lib:${LD_LIBRARY_PATH:-}" >> "$GITHUB_ENV"
else
    export PATH="$FORK_LLVM/bin:$PATH"
    export LIBRARY_PATH="$FORK_LLVM/lib:${LIBRARY_PATH:-}"
    export LD_LIBRARY_PATH="$FORK_LLVM/lib:${LD_LIBRARY_PATH:-}"
    echo "stage0 ready:"
    echo "  fly $FLY_VERSION  -> $FLY_BIN"
    echo "  LLVM $LLVM_VERSION (fork) -> PATH += $FORK_LLVM/bin ; LIBRARY_PATH/LD_LIBRARY_PATH += $FORK_LLVM/lib"
fi
