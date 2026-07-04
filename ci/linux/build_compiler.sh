#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_compiler.sh — build the COMPILED std library, then the self-host `fly`.
#
# Mirrors fly/'s CMake model (std/CMakeLists.txt): the standard library is first
# compiled into ONE archive `fly_std_lib.a` plus flat `*.fly.h` headers via the
# bootstrap compiler `--lib`; the compiler executable is then built LINKING that
# archive. The release ships `bin/fly` + a sibling `lib/` holding the compiled
# std (fly_std_lib.a + *.fly.h + the runtime archive + bridge stubs), NOT source.
#
# A released fly resolves std at <exe_dir>/../lib. The reference compiler has no
# flag to point its stdlib/runtime dir elsewhere, so to link OUR archive we run a
# copy of the bootstrap FROM build/bin: <exe>/../lib then resolves to build/lib
# and auto-discovery loads our headers + links fly_std_lib.a (+ fly_runtime_lib.a).
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

# build/bin + build/lib are the release-artifact paths the workflows
# (build-linux.yml → release.yml) upload and package.
OUT=build/bin
LIB=build/lib
STD=std/lib
mkdir -p "$OUT" "$LIB"

# Invoke the bootstrap compiler via $FLY (default: `fly` on PATH). The compiler
# derives its stdlib dir from its own executable path (argv[0]), and a bare name
# breaks that lookup — so a slash-less $FLY is resolved through PATH into an
# absolute path here. See ../fly Driver.cpp.
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
if [ ! -d "$(dirname "$FLY")/../lib" ]; then
    echo "error: no lib/ directory next to '$FLY' (expected <exe_dir>/../lib" >&2
    echo "       with llvm.fly.h, runtime.fly.h, fly_runtime_lib.a)." >&2
    echo "       Point FLY at a built bootstrap compiler, e.g. fly/build/bin/fly." >&2
    exit 1
fi

# ── 1) Seed the runtime bridge stubs + runtime archive from the bootstrap's own
#       lib. The `--lib` std build needs llvm.fly.h/runtime.fly.h to resolve
#       fly.runtime/fly.llvm; the release+link need fly_runtime_lib.a. ─────────
BLIB="$(cd "$(dirname "$FLY")/../lib" && pwd)"
cp "$BLIB/llvm.fly.h" "$BLIB/runtime.fly.h" "$BLIB/fly_runtime_lib.a" "$LIB/"

# ── 2) Compile the std into one archive + flat *.fly.h headers. Source order
#       mirrors fly/std/CMakeLists.txt (namespace/header dependency order). The
#       compiler appends the .a extension to the -o stem itself. ───────────────
"$FLY" --lib -o "$LIB/fly_std_lib" \
    "$STD/assert.fly" "$STD/str.fly" "$STD/math.fly" \
    "$STD/os/time.fly" "$STD/os/env.fly" "$STD/os/path.fly" "$STD/os/io.fly" "$STD/os/fs.fly" \
    "$STD/sync.fly" "$STD/mem.fly" "$STD/bridge/clang.fly" \
    "$STD/data/list.fly" "$STD/data/stack.fly" "$STD/data/queue.fly" "$STD/data/deque.fly" \
    "$STD/data/map.fly" "$STD/data/set.fly" "$STD/data/tree.fly" "$STD/data/wrapper.fly" \
    "$STD/os/proc.fly"

# ── 3) Build the compiler executable LINKING our fly_std_lib.a. Run a copy of the
#       bootstrap from build/bin so <exe>/../lib == build/lib: auto-discovery then
#       loads our generated headers and links fly_std_lib.a + fly_runtime_lib.a.
#       --src-dir indexes the whole fly.compiler graph from the entry's imports;
#       each `main` is in its own namespace so only the entry's becomes the C entry.
cp "$FLY" "$OUT/fly"
"$OUT/fly" compiler/Fly.fly \
    --src-dir compiler \
    -o fly --out-dir "$OUT"
# The src-dir build emits ONE combined object "$OUT/Fly.fly.o" then links it into
# "$OUT/fly" (dynamically against system libLLVM via the -lLLVM-20 bridge flag).

# ── 3b) Optional SELF-CONTAINED bundle (Rust-style). With FLY_BUNDLE_LLVM=1 the
#       release ships ONE shared libLLVM.so in lib/, and both `fly` and a small
#       `fly-lld` link DYNAMICALLY against it via RUNPATH $ORIGIN/../lib — so the
#       host needs neither system libLLVM nor a system linker, while LLVM is stored
#       once (tarball ~half of a full-static build). The src-dir mode can't emit an
#       object directly (emit modes are gated off), so we reuse the "$OUT/Fly.fly.o"
#       the link step already produced and hand it to clang++.
#       Gated so the default local dev build keeps the plain dynamic link against
#       system libLLVM; the release workflow sets FLY_BUNDLE_LLVM=1.
if [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
    CXX="$(command -v clang++-20 || command -v clang++ || true)"
    LLVM_CONFIG="$(command -v llvm-config-20 || command -v llvm-config || true)"
    if [ -z "$CXX" ] || [ -z "$LLVM_CONFIG" ]; then
        echo "error: FLY_BUNDLE_LLVM=1 needs clang++ and llvm-config (install llvm-20-dev + clang)." >&2
        exit 1
    fi
    OBJ="$OUT/Fly.fly.o"
    if [ ! -f "$OBJ" ]; then
        echo "error: expected combined object '$OBJ' from the src-dir build." >&2
        exit 1
    fi
    LLVM_LIBDIR="$("$LLVM_CONFIG" --libdir)"

    # 1) Bundle ONE libLLVM.so into lib/ under its SONAME (== the DT_NEEDED the
    #    binaries below record), so RUNPATH resolution finds this copy.
    LLVM_SO="$(readlink -f "$LLVM_LIBDIR/libLLVM-20.so")"
    SONAME="$(readelf -d "$LLVM_SO" | sed -n 's/.*Library soname: \[\(.*\)\].*/\1/p')"
    if [ -z "$SONAME" ]; then
        echo "error: could not read libLLVM SONAME from '$LLVM_SO'." >&2
        exit 1
    fi
    echo "bundling $SONAME into $LIB ..."
    cp "$LLVM_SO" "$LIB/$SONAME"

    # 2) fly: dynamic link against the bundled libLLVM via rpath (clang++ supplies
    #    crt/libc). $ORIGIN is bin/, so $ORIGIN/../lib is the bundled lib dir.
    echo "linking $OUT/fly against bundled libLLVM (rpath) ..."
    "$CXX" "$OBJ" \
        "$LIB/fly_std_lib.a" "$LIB/fly_runtime_lib.a" \
        -L"$LLVM_LIBDIR" -lLLVM-20 -Wl,-rpath,'$ORIGIN/../lib' \
        -o "$OUT/fly"

    # 3) Bundle a small `fly-lld` beside the binary so the toolchain needs no
    #    system linker either: a tiny LLD driver (ci/linux/lld_driver.cpp, ELF
    #    flavor here) with lld's own code STATIC (liblld*.a) but LLVM DYNAMIC
    #    (shared with fly via the same bundled libLLVM + rpath). ToolChain.fly
    #    prefers <exe_dir>/fly-lld.
    #    lld headers/archives come from liblld-20-dev (CI); local dev without that
    #    package falls back to the bootstrap's own LLVM build tree (<bootstrap>/../llvm).
    LLD_INC="$("$LLVM_CONFIG" --includedir)"
    LLD_LIBDIR="$LLVM_LIBDIR"
    if [ ! -f "$LLD_LIBDIR/liblldELF.a" ]; then
        BOOT_LLVM="$(cd "$(dirname "$FLY")/../llvm" 2>/dev/null && pwd || true)"
        if [ -n "$BOOT_LLVM" ] && [ -f "$BOOT_LLVM/lib/liblldELF.a" ]; then
            LLD_INC="$BOOT_LLVM/include"
            LLD_LIBDIR="$BOOT_LLVM/lib"
        fi
    fi
    if [ ! -f "$LLD_LIBDIR/liblldELF.a" ]; then
        echo "error: liblldELF.a not found; install liblld-20-dev (or point FLY at a" >&2
        echo "       bootstrap whose ../llvm tree holds the lld static archives)." >&2
        exit 1
    fi
    echo "linking $OUT/fly-lld (lld static, LLVM dynamic, rpath) ..."
    "$CXX" ci/linux/lld_driver.cpp -std=c++17 \
        -I"$LLD_INC" \
        -L"$LLD_LIBDIR" -llldELF -llldCommon \
        -L"$LLVM_LIBDIR" -lLLVM-20 -Wl,-rpath,'$ORIGIN/../lib' \
        $("$LLVM_CONFIG" --link-static --system-libs) \
        -o "$OUT/fly-lld"
fi

# ── 4) Cleanup: bin/ ships only the executable "$OUT/fly" (drop intermediate
#       objects; the bootstrap copy was overwritten in place by the -o output).
rm -f "$OUT"/*.o

echo "fly -> $OUT/fly"
echo "std -> $LIB (fly_std_lib.a + *.fly.h + runtime)"
