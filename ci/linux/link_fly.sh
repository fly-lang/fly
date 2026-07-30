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

[ -f "$OBJ" ] || { echo "error: $OBJ missing — run build_compiler.sh first." >&2; exit 1; }
# MONOLITHIC: the compiler is inside $OBJ (compiled from source by build_compiler),
# NOT a separate fly_compiler_lib.a archive — avoids the linker COMDAT-dedup of the
# compiler's generic instantiations that caused the `fly build` UAF crash.
[ -f "$LIB/fly_std_lib.a" ] && [ -f "$LIB/fly_runtime_lib.a" ] || { echo "error: std/runtime missing in $LIB — run build_runtime.sh + build_std.sh first." >&2; exit 1; }

# ── Toolchain: the fork's ld.lld + the host C runtime objects. ────────────────
FORK_LLVM=build/llvm
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
    RPATH='$ORIGIN/../lib'
else
    RPATH="$(cd "$FORK_LLVM/lib" && pwd)"
fi

echo "stage$STAGE: linking $OUT/fly ..."
# The link itself lives in link_bin.sh, shared with build_lsp.sh; RPATH is
# passed through because only this script knows about the LLVM bundling mode.
FLY_LINK_RPATH="$RPATH" ci/linux/link_bin.sh "$OBJ" "$OUT/fly" --with-llvm

echo "stage$STAGE: fly -> $OUT/fly (libs from $LIB)"
