#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# stage0.sh — set up stage 0 of the staged bootstrap on Linux:
#
#   • the fork LLVM toolchain (fly-lang/llvm-project release) → build/llvm
#     libLLVM.so + ld.lld + llvm-config + compiler-rt builtins. NO system
#     package manager: everything the build needs comes from this tarball.
#   • the pinned bootstrap `fly` release ($FLY_VERSION below) → build/stage0
#     (bin/ + precompiled lib/)
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
# 0.13.13 brought the array subscript `k[i]`, `C[N]` declarations and
# reference-counted array buffers — the syntax the tree may use is bounded by
# what the seed can parse, since it compiles the whole tree at stage 1.
# 0.13.14 is the first STD-LESS seed: the package is just bin/fly +
# lib/{llvm.fly.h, runtime.fly.h, fly_runtime_lib.a} — the std lives in-tree
# and stage 1 builds it from source. It also carries the inherited-interface
# sema fix and the __out_N StringRef-dangle fix the interleaved suites need.
FLY_VERSION="${FLY_VERSION:-0.13.14}"

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
if [ ! -f "$FORK_LLVM/lib/libLLVM.so" ] || [ ! -f "$RT_BUILTINS" ] || [ ! -x "$FORK_LLVM/bin/lldb" ] || [ "$NEED_STATIC" = "1" ]; then
    url="https://github.com/fly-lang/llvm-project/releases/download/v${LLVM_VERSION}-linux-x86_64/llvm-${LLVM_VERSION}-x86_64-linux-gnu.tar.gz"
    mkdir -p "$BUILD_DIR"
    tarball="$BUILD_DIR/fork-llvm.tar.gz"
    # Atomic download (.part + mv, with retries): an interrupted curl must never
    # leave a truncated tarball that a rerun would mistake for the real one.
    if [ ! -f "$tarball" ]; then
        curl -fSL --retry 6 --retry-delay 15 --retry-all-errors -o "$tarball.part" "$url"
        mv -f "$tarball.part" "$tarball"
    fi
    # Always: the shared libLLVM.so (link + runtime), the lld linker, llvm-config,
    # the compiler-rt builtins (linked into every fly-produced executable) and the
    # debugger the release bundles — lldb + liblldb + lldb-server (lldb launches
    # local processes through it) + lldb-dap (IDE/DAP) + lldb-argdumper, all under
    # their ORIGINAL LLVM names. Keep this list in sync with the guard above AND
    # the fly-llvm cache key in .github/workflows/build-linux.yml.
    # Bundle also: the lld static archives + LLVM/lld headers.
    paths="llvm/lib/libLLVM.so* llvm/bin/ld.lld llvm/bin/lld llvm/bin/llvm-config llvm/lib/clang"
    paths="$paths llvm/bin/lldb llvm/bin/lldb-server llvm/bin/lldb-dap llvm/bin/lldb-argdumper llvm/lib/liblldb.so*"
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

# Both ToolChains (self-host compiler/lib/driver/ToolChain.fly and the bootstrap's C++ one)
# probe /usr/lib/llvm-20 for libclang_rt.builtins. On a host without an LLVM 20
# install, point that path at the fork tree (this replaces the old apt install).
if [ ! -e "/usr/lib/llvm-${LLVM_VERSION%%.*}" ]; then
    echo "linking /usr/lib/llvm-${LLVM_VERSION%%.*} -> $FORK_LLVM (compiler-rt builtins for the ToolChain probes)"
    sudo ln -sfn "$FORK_LLVM" "/usr/lib/llvm-${LLVM_VERSION%%.*}"
fi

# --- Download the bootstrap fly (stage 0) --------------------------------------
# Reuse an existing seed only when it is EXACTLY the pinned version (local
# convenience; a fresh CI runner never has one).
#
# Presence alone is not enough: after a pin bump, a checkout that already had the
# previous seed would keep it forever and this script would still report the
# pinned version it had not installed. The failure that follows is thoroughly
# misleading — stage1 compiles std with an old seed and a handful of suites fail
# on constructs the language does support.
have=""
if [ -x "$FLY_BIN" ]; then
    have="$("$FLY_BIN" --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
fi

if [ "$have" != "$FLY_VERSION" ]; then
    [ -n "$have" ] && echo "stage0: seed is $have, pinned is $FLY_VERSION — fetching the pinned one."
    url="https://github.com/fly-lang/fly/releases/download/v${FLY_VERSION}/fly-${FLY_VERSION}-linux-x86_64.tar.gz"
    # Staged swap: an existing seed is replaced only AFTER the new one is in
    # hand. Deleting first would strand a machine whose pinned release is not
    # published yet — which is a real state during a seed re-cut.
    rm -rf "$STAGE0_DIR.new"
    mkdir -p "$STAGE0_DIR.new"
    if curl -fsSL --retry 6 --retry-delay 15 --retry-all-errors "$url" -o "$BUILD_DIR/fly.tar.gz"; then
        tar -xzf "$BUILD_DIR/fly.tar.gz" -C "$STAGE0_DIR.new"
        rm -f "$BUILD_DIR/fly.tar.gz"
        rm -rf "$STAGE0_DIR"
        mv "$STAGE0_DIR.new" "$STAGE0_DIR"
        chmod +x "$FLY_BIN"
    else
        rm -rf "$STAGE0_DIR.new" "$BUILD_DIR/fly.tar.gz"
        echo "error: seed v$FLY_VERSION is not downloadable ($url)." >&2
        if [ -n "$have" ]; then
            echo "       the existing $have seed was left in place, but it does NOT match the pin:" >&2
            echo "       stage1 will compile std with it and can fail on newer constructs." >&2
        fi
        exit 1
    fi
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
