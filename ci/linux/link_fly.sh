#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# link_fly.sh — link the fly executable for the current stage: the driver
# object (build_compiler.sh, compiler merged in) + std + runtime + libLLVM →
# build/stage$STAGE/bin/fly. Uses the fork's ld.lld directly (no clang/gcc
# driver). See stage1.sh for the stage map.
#
# FLY_BUNDLE_LLVM=1 makes the stage self-contained: libLLVM.so (by SONAME) in
# build/stage$STAGE/lib and fly + ld.lld resolving it via RUNPATH $ORIGIN/../lib.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

# ── Stage plumbing: everything comes from the stage dirs. ─────────────────────
STAGE="${STAGE:-1}"
OUT="build/stage$STAGE/bin"
LIB="build/stage$STAGE/lib"
CDIR=build/stage1/compiler
OBJ="build/stage$STAGE/driver/Driver.o"
mkdir -p "$OUT" "$LIB"

# FLY_LINKED=1: $OUT/fly is already there, linked by `fly build` (stage1.sh since
# 0.16); only the bundle below is left to do. Such a fly carries no RUNPATH: it
# finds libLLVM through the loader path stage0.sh exports.
if [ "${FLY_LINKED:-0}" = "1" ]; then
    [ -x "$OUT/fly" ] || { echo "error: FLY_LINKED=1 but $OUT/fly is missing." >&2; exit 1; }
else
    [ -f "$OBJ" ] || { echo "error: $OBJ missing — run build_compiler.sh first." >&2; exit 1; }
fi
# MONOLITHIC: the compiler is inside $OBJ (compiled from source by build_compiler),
# NOT a separate fly_compiler_lib.a archive — avoids the linker COMDAT-dedup of the
# compiler's generic instantiations that caused the `fly build` UAF crash.
[ -f "$LIB/fly_std_lib.a" ] && [ -f "$LIB/fly_runtime_lib.a" ] || { echo "error: std/runtime missing in $LIB — run build_runtime.sh + build_std.sh first." >&2; exit 1; }

. ci/linux/platform.sh
FORK_LLVM=build/llvm

# ── macOS: ld64.lld + libSystem; the bundle is libLLVM.dylib + the debugger ───
# The fork installs its dylibs with an @rpath install name, so copying them into
# lib/ next to bin/ (rpath @loader_path/../lib) is all the bundling there is. No
# compiler-rt is shipped: libSystem re-exports the builtins on macOS.
if is_macos; then
    LLD64="$FORK_LLVM/bin/ld64.lld"
    if [ ! -f "$FORK_LLVM/lib/libLLVM.dylib" ] || [ ! -x "$LLD64" ]; then
        echo "error: fork LLVM not found at $FORK_LLVM/ — run ci/linux/stage0.sh." >&2
        exit 1
    fi
    if [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
        cp -f "$FORK_LLVM/lib/libLLVM.dylib" "$LIB/libLLVM.dylib"
        cp -fL "$LLD64" "$OUT/ld64.lld"
        # lldb, lldb-dap, lldb-argdumper and, when the fork built it, debugserver
        # (macOS lldb launches local processes through it), under their original
        # names; liblldb goes into lib/ like libLLVM.
        for dbg in lldb lldb-dap lldb-argdumper debugserver dsymutil; do
            [ -e "$FORK_LLVM/bin/$dbg" ] && cp -fL "$FORK_LLVM/bin/$dbg" "$OUT/$dbg"
        done
        for l in "$FORK_LLVM"/lib/liblldb*.dylib; do
            [ -e "$l" ] && cp -fL "$l" "$LIB/$(basename "$l")"
        done
        RPATH="$EXE_RPATH"
    else
        RPATH="$(cd "$FORK_LLVM/lib" && pwd)"
    fi
    if [ "${FLY_LINKED:-0}" != "1" ]; then
        echo "stage$STAGE: linking $OUT/fly ..."
        FLY_LINK_RPATH="$RPATH" ci/linux/link_bin.sh "$OBJ" "$OUT/fly" --with-llvm
    fi
    echo "stage$STAGE: fly -> $OUT/fly (libs from $LIB)"
    exit 0
fi

# ── Linux: the fork's ld.lld + the host C runtime objects. ────────────────────
LLD="$FORK_LLVM/bin/ld.lld"
if [ ! -f "$FORK_LLVM/lib/libLLVM.so" ] || [ ! -x "$LLD" ]; then
    echo "error: fork LLVM not found at $FORK_LLVM/ — run ci/linux/stage0.sh." >&2
    exit 1
fi
MULTIARCH=/usr/lib/x86_64-linux-gnu
GCCDIR="$(ls -d /usr/lib/gcc/x86_64-linux-gnu/*/ 2>/dev/null | sort -V | tail -1)"
GCCDIR="${GCCDIR%/}"
if [ ! -f "$MULTIARCH/Scrt1.o" ] || [ ! -f "$GCCDIR/crtbeginS.o" ]; then
    echo "error: host C runtime objects missing (need libc6-dev + gcc)." >&2
    exit 1
fi

if [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
    # Bundle libLLVM.so under its SONAME into this stage's lib (bin/fly's
    # RUNPATH is always $ORIGIN/../lib) + ship the relocatable ld.lld.
    LLVM_SO="$(readlink -f "$FORK_LLVM/lib/libLLVM.so")"
    SONAME="$(readelf -d "$LLVM_SO" | sed -n 's/.*Library soname: \[\(.*\)\].*/\1/p')"
    [ -n "$SONAME" ] || { echo "error: cannot read libLLVM SONAME." >&2; exit 1; }
    cp -f "$LLVM_SO" "$LIB/$SONAME"
    cp -aLf "$LLD" "$OUT/ld.lld"
    # The debugger, under its ORIGINAL LLVM names: lldb (driver) + lldb-server
    # (lldb launches local processes through it; found via the liblldb-relative
    # support-exe dir = this bin/) + lldb-dap (IDE/DAP) + lldb-argdumper.
    # liblldb goes into lib/ under its SONAME (lldb's RUNPATH is $ORIGIN/../lib),
    # same dance as libLLVM above.
    for dbg in lldb lldb-server lldb-dap lldb-argdumper; do
        cp -aLf "$FORK_LLVM/bin/$dbg" "$OUT/$dbg"
    done
    LLDB_SO="$(readlink -f "$FORK_LLVM/lib/liblldb.so")"
    LLDB_SONAME="$(readelf -d "$LLDB_SO" | sed -n 's/.*Library soname: \[\(.*\)\].*/\1/p')"
    [ -n "$LLDB_SONAME" ] || { echo "error: cannot read liblldb SONAME." >&2; exit 1; }
    cp -f "$LLDB_SO" "$LIB/$LLDB_SONAME"
    RPATH='$ORIGIN/../lib'
else
    RPATH="$(cd "$FORK_LLVM/lib" && pwd)"
fi

# compiler-rt builtins: linked into EVERY program fly produces, so they ship in
# the bundle at the path the ToolChain probes first (<exe>/../lib/clang/<major>/
# lib/<triple>/, LLVM's own layout). Always copied, bundle or not: a release
# without them could link nothing on a host without a system LLVM 20.
RT_REL="lib/clang/20/lib/x86_64-unknown-linux-gnu"
RT_SRC="$FORK_LLVM/$RT_REL/libclang_rt.builtins.a"
[ -f "$RT_SRC" ] || { echo "error: $RT_SRC missing — run ci/linux/stage0.sh." >&2; exit 1; }
mkdir -p "build/stage$STAGE/$RT_REL"
cp -f "$RT_SRC" "build/stage$STAGE/$RT_REL/libclang_rt.builtins.a"

if [ "${FLY_LINKED:-0}" != "1" ]; then
    echo "stage$STAGE: linking $OUT/fly ..."
    # The link itself lives in link_bin.sh, shared with build_lsp.sh; RPATH is
    # passed through because only this script knows about the LLVM bundling mode.
    FLY_LINK_RPATH="$RPATH" ci/linux/link_bin.sh "$OBJ" "$OUT/fly" --with-llvm
fi

echo "stage$STAGE: fly -> $OUT/fly (libs from $LIB)"
