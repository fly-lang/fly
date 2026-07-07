#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_driver.sh — build the driver (driver/lib + the entry driver/lib/Driver.fly,
# which carries `main`) and LINK it against the compiler library from
# build_compiler.sh (fly_compiler_lib.a) + std + runtime + libLLVM, producing the
# final self-host executable build/bin/fly.
#
# Layered self-host build (mirrors fly/'s CMake target graph):
#     build_runtime.sh → fly_runtime_lib.a         (+ runtime.fly.h, llvm.fly.h)
#     build_std.sh      → fly_std_lib.a            (+ *.fly.h)
#     build_compiler.sh → fly_compiler_lib.a       (+ *.fly.h)      [pure library]
#     build_driver.sh   → bin/fly                   (links the three archives)   ← HERE
#
# The entry Driver.fly imports `fly.compiler.*` (resolved from the *.fly.h headers in
# build/lib) and `fly.driver.*` (its OWN namespace — pulls the sibling driver files via
# --src-dir); the passed fly_compiler_lib.a supplies the compiler symbols. The compiler
# archive references the LLVM C-API (LLVMInitialize*), so the final link adds -lLLVM.
# NOTE: `compiler/` is now a pure library (no entry); `main` lives in driver/lib.
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

OUT=build/bin
LIB=build/lib
mkdir -p "$OUT" "$LIB"

# Invoke the bootstrap compiler via $FLY (default: `fly` on PATH). The compiler
# derives its stdlib dir from its own executable path (argv[0]); a bare name breaks
# that lookup, so a slash-less $FLY is resolved through PATH into an absolute path.
FLY="${FLY:-fly}"
case "$FLY" in
    */*) ;;  # already a path — keep as given
    *)
        FLY_RESOLVED="$(command -v "$FLY" || true)"
        if [ -z "$FLY_RESOLVED" ]; then
            echo "error: bootstrap compiler '$FLY' not found on PATH." >&2
            echo "       Set FLY to the bootstrap compiler, e.g.:" >&2
            echo "       FLY=/path/to/fly/build/bin/fly $0" >&2
            exit 1
        fi
        FLY="$FLY_RESOLVED"
        ;;
esac
if [ ! -x "$FLY" ]; then
    echo "error: FLY='$FLY' is not an executable file." >&2
    exit 1
fi

# ── Prerequisites: the compiler library + std + runtime archives must be in build/lib
#    (build_compiler.sh, which itself seeds runtime + std). ─────────────────────────
for f in fly_compiler_lib.a fly_std_lib.a fly_runtime_lib.a; do
    if [ ! -f "$LIB/$f" ]; then
        echo "error: $LIB/$f missing — run ci/linux/build_compiler.sh first." >&2
        exit 1
    fi
done

# ── Strip any stale fly.driver headers from build/lib. A fly.driver *.fly.h would make
#    the driver namespace "header-served", so the frontend would link the driver from an
#    archive stub and SKIP the driver SOURCE (→ "undefined symbol Driver.init_ctor").
#    The driver is compiled FROM SOURCE here, never consumed as a header. ─────────────
for h in "$LIB"/*.fly.h; do
    [ -e "$h" ] || continue
    if grep -q "namespace fly.driver" "$h" 2>/dev/null; then rm -f "$h"; fi
done

# ── Emit the merged driver object. Run a bootstrap COPY from build/bin so <exe>/../lib
#    == build/lib. The in-process staged link FAILS on the LLVM C-API symbols the
#    compiler archive references (they resolve only against libLLVM, added in the relink
#    below), but the object is emitted BEFORE that link — so tolerate the failure and
#    reuse the object. Mirrors the old build_compiler FLY_BUNDLE staged-object flow. ──
cp "$FLY" "$OUT/fly"
STAGE=build/stage_fly
rm -rf "$STAGE"; mkdir -p "$STAGE"
echo "compiling driver + entry into $STAGE/Driver.fly.o ..."
"$OUT/fly" driver/lib/Driver.fly "$LIB/fly_compiler_lib.a" --src-dir driver/lib \
    -o fly --out-dir "$STAGE" > "$STAGE/emit.log" 2>&1 || \
    echo "note: staged in-process link failed (expected — libLLVM is added in the relink below); reusing the emitted object."
rm -f "$OUT/fly"                 # drop the bootstrap copy
OBJ="$STAGE/Driver.fly.o"
if [ ! -f "$OBJ" ]; then
    echo "error: expected driver object '$OBJ' was not emitted; see $STAGE/emit.log:" >&2
    grep -m5 -E 'error:|broken|abort' "$STAGE/emit.log" | sed 's/^/      /' >&2 || true
    exit 1
fi

# ── Final link with the fork's ld.lld — NO clang++/g++. Hand it the host C runtime
#    (glibc Scrt1/crti/crtn, gcc crtbeginS/crtendS + libgcc) + the three Fly archives +
#    libLLVM. libLLVM comes from the fork tree; RUNPATH resolves it: $ORIGIN/../lib in
#    bundle mode (libLLVM.so copied into lib/ under its SONAME), or the fork tree
#    otherwise. These crt objects ship with build-essential on any Linux build host. ──
FORK_LLVM=build/llvm
LLD="$FORK_LLVM/bin/ld.lld"
if [ ! -f "$FORK_LLVM/lib/libLLVM.so" ] || [ ! -x "$LLD" ]; then
    echo "error: fork LLVM tree not found at $FORK_LLVM/ (need lib/libLLVM.so + bin/ld.lld)." >&2
    echo "       Run: ci/linux/install_prerequisites.sh" >&2
    exit 1
fi
MULTIARCH=/usr/lib/x86_64-linux-gnu
GCCDIR="$(ls -d /usr/lib/gcc/x86_64-linux-gnu/*/ 2>/dev/null | sort -V | tail -1)"
GCCDIR="${GCCDIR%/}"
if [ ! -f "$MULTIARCH/Scrt1.o" ] || [ ! -f "$GCCDIR/crtbeginS.o" ]; then
    echo "error: C runtime objects not found (need libc6-dev + gcc: $MULTIARCH/Scrt1.o, $GCCDIR/crtbeginS.o)." >&2
    exit 1
fi

if [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
    # Bundle ONE libLLVM.so into lib/ under its SONAME (== the DT_NEEDED fly records),
    # so RUNPATH $ORIGIN/../lib resolves this copy. Ship the fork's relocatable ld.lld
    # under its native name (ToolChain prefers <exe_dir>/ld.lld; LLD picks the ELF
    # flavor from argv[0]).
    LLVM_SO="$(readlink -f "$FORK_LLVM/lib/libLLVM.so")"
    SONAME="$(readelf -d "$LLVM_SO" | sed -n 's/.*Library soname: \[\(.*\)\].*/\1/p')"
    if [ -z "$SONAME" ]; then
        echo "error: could not read libLLVM SONAME from '$LLVM_SO'." >&2
        exit 1
    fi
    echo "bundling $SONAME into $LIB + shipping $OUT/ld.lld ..."
    cp "$LLVM_SO" "$LIB/$SONAME"
    cp -aL "$LLD" "$OUT/ld.lld"
    RPATH='$ORIGIN/../lib'
else
    RPATH="$(cd "$FORK_LLVM/lib" && pwd)"
fi

echo "linking $OUT/fly (driver + fly_compiler_lib.a + std + runtime + libLLVM) ..."
"$LLD" -pie --hash-style=gnu --eh-frame-hdr -m elf_x86_64 \
    -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o "$OUT/fly" \
    "$MULTIARCH/Scrt1.o" "$MULTIARCH/crti.o" "$GCCDIR/crtbeginS.o" \
    -L"$FORK_LLVM/lib" -L"$GCCDIR" -L"$MULTIARCH" \
    "$OBJ" "$LIB/fly_compiler_lib.a" "$LIB/fly_std_lib.a" "$LIB/fly_runtime_lib.a" \
    -lLLVM -rpath "$RPATH" \
    -lstdc++ -lm -lgcc_s -lgcc -lc \
    "$GCCDIR/crtendS.o" "$MULTIARCH/crtn.o"

# ── Release lib/ cleanup (bundle mode): the compiled compiler now lives INSIDE
#    bin/fly, so the shipped lib/ only needs what USER compilations require — std +
#    runtime headers/archives + libLLVM. Drop the build-time-only compiler headers
#    (namespace fly.compiler.*) and fly_compiler_lib.a. (Skipped in dev mode so a bare
#    re-run of build_driver.sh still finds its prerequisites.) ─────────────────────────
if [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
    rm -f "$LIB/fly_compiler_lib.a"
    for h in "$LIB"/*.fly.h; do
        [ -e "$h" ] || continue
        if grep -q "namespace fly.compiler" "$h" 2>/dev/null; then rm -f "$h"; fi
    done
fi

# Cleanup staging + intermediate objects.
rm -rf "$STAGE"
rm -f "$OUT"/*.o

echo "fly -> $OUT/fly (driver + compiler lib + std + runtime + libLLVM)"
