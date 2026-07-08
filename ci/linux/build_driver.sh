#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_driver.sh — compile the driver (driver/lib, entry Driver.fly) and link
# the fly executable: driver + fly_compiler_lib.a + std + runtime + libLLVM →
# build/bin/fly. Run with STAGE=1 (stage0 compiles the driver, links the stage-1
# libs) or STAGE=2 (the stage-1 fly recompiles it, links the self-host libs);
# see stagelib.sh. The compiler archive + headers always come from
# build/stage1/compiler (build-only, never shipped).
#
# FLY_BUNDLE_LLVM=1 ships a self-contained toolchain: one libLLVM.so in
# build/lib and fly + ld.lld resolving it via RUNPATH $ORIGIN/../lib.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
. ci/linux/stagelib.sh

OUT=build/bin
mkdir -p "$OUT" build/lib

[ -f "$CDIR/fly_compiler_lib.a" ] || { echo "error: $CDIR/fly_compiler_lib.a missing — run build_compiler.sh first." >&2; exit 1; }
[ -f "$LIB/fly_std_lib.a" ] && [ -f "$LIB/fly_runtime_lib.a" ] || { echo "error: std/runtime missing in $LIB — run build_runtime.sh + build_std.sh first." >&2; exit 1; }

# The driver is always compiled FROM SOURCE — never consumed as a header.
for h in "$CDIR"/*.fly.h "$LIB"/*.fly.h; do
    [ -e "$h" ] || continue
    if grep -q "namespace fly.driver" "$h" 2>/dev/null; then rm -f "$h"; fi
done

# ── Emit the merged driver object (-L serves the compiler headers). ─────────────
T=build/tmp_driver
rm -rf "$T"; mkdir -p "$T"
echo "stage$STAGE: compiling driver ..."
if [ "$STAGE" = "1" ]; then
    # stage0 reference: no -c — its in-process link fails on the LLVM C-API
    # symbols (resolved only by -lLLVM below) but emits the object first.
    "$FLY" driver/lib/Driver.fly "$CDIR/fly_compiler_lib.a" --src-dir driver/lib -L "$CDIR" \
        -o fly --out-dir "$T" > "$T/emit.log" 2>&1 || true
    OBJ="$T/Driver.fly.o"
else
    # self-host: -c emits a clean object, no link attempt.
    "$FLY" driver/lib/Driver.fly --src-dir driver/lib -L "$CDIR" \
        -c -o Driver --out-dir "$T" > "$T/emit.log" 2>&1 || true
    OBJ="$T/Driver"
fi
if [ ! -f "$OBJ" ]; then
    echo "error: driver object not emitted; see $T/emit.log:" >&2
    grep -m5 -E 'error:|broken|abort' "$T/emit.log" | sed 's/^/      /' >&2 || true
    exit 1
fi

# ── Link with the fork's ld.lld (no clang/gcc driver): host C runtime + driver
#    object + the three Fly archives + libLLVM. ──────────────────────────────────
FORK_LLVM=build/llvm
LLD="$FORK_LLVM/bin/ld.lld"
if [ ! -f "$FORK_LLVM/lib/libLLVM.so" ] || [ ! -x "$LLD" ]; then
    echo "error: fork LLVM not found at $FORK_LLVM/ — run ci/linux/install_prerequisites.sh." >&2
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
    # Bundle libLLVM.so under its SONAME into the SHIPPED build/lib (bin/fly's
    # RUNPATH is always $ORIGIN/../lib) + ship the relocatable ld.lld.
    LLVM_SO="$(readlink -f "$FORK_LLVM/lib/libLLVM.so")"
    SONAME="$(readelf -d "$LLVM_SO" | sed -n 's/.*Library soname: \[\(.*\)\].*/\1/p')"
    [ -n "$SONAME" ] || { echo "error: cannot read libLLVM SONAME." >&2; exit 1; }
    cp -f "$LLVM_SO" "build/lib/$SONAME"
    cp -aLf "$LLD" "$OUT/ld.lld"
    RPATH='$ORIGIN/../lib'
else
    RPATH="$(cd "$FORK_LLVM/lib" && pwd)"
fi

echo "stage$STAGE: linking $OUT/fly ..."
"$LLD" -pie --hash-style=gnu --eh-frame-hdr -m elf_x86_64 \
    -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o "$OUT/fly" \
    "$MULTIARCH/Scrt1.o" "$MULTIARCH/crti.o" "$GCCDIR/crtbeginS.o" \
    -L"$FORK_LLVM/lib" -L"$GCCDIR" -L"$MULTIARCH" \
    "$OBJ" "$CDIR/fly_compiler_lib.a" "$LIB/fly_std_lib.a" "$LIB/fly_runtime_lib.a" \
    -lLLVM -rpath "$RPATH" \
    -lstdc++ -lm -lgcc_s -lgcc -lc \
    "$GCCDIR/crtendS.o" "$MULTIARCH/crtn.o"

rm -rf "$T"
echo "stage$STAGE: fly -> $OUT/fly (libs from $LIB)"
