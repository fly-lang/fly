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
# The bootstrap copy runs as build/bin/fly, but writes its output to a STAGING dir
# (not build/bin) — a process can't overwrite its own running executable: harmless
# on Linux (replaces the inode) but a hard "permission denied" on Windows (the
# running .exe is locked). build/bin/fly still resolves <exe>/../lib to build/lib.
cp "$FLY" "$OUT/fly"
STAGE="build/stage"
rm -rf "$STAGE"
mkdir -p "$STAGE"
"$OUT/fly" compiler/Fly.fly \
    --src-dir compiler \
    -o fly --out-dir "$STAGE"
rm -f "$OUT/fly"                 # bootstrap done (no longer running) — drop the copy
# The staged build emitted ONE combined object "$STAGE/Fly.fly.o" and linked it into
# "$STAGE/fly" (dynamically against system libLLVM via the -lLLVM-20 bridge flag).

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
    if [ -z "$CXX" ]; then
        echo "error: FLY_BUNDLE_LLVM=1 needs clang++ (install clang)." >&2
        exit 1
    fi
    OBJ="$STAGE/Fly.fly.o"
    if [ ! -f "$OBJ" ]; then
        echo "error: expected combined object '$OBJ' from the src-dir build." >&2
        exit 1
    fi

    # LLVM for the BUNDLE: the fork LLVM tree (build/llvm) that install_prerequisites.sh
    # fetched. Its libLLVM.so is built LLVM_ENABLE_LIBXML2=OFF (in fact with NO external
    # deps at all), so the shipped release loads on any host. NO apt fallback: apt's
    # libLLVM drags libxml2/libzstd and would reintroduce the portability gap. The lld
    # static archives + headers come from the same fork tree.
    FORK_LLVM="build/llvm"
    if [ ! -f "$FORK_LLVM/lib/libLLVM.so" ]; then
        echo "error: fork LLVM (libxml2-off shared libLLVM.so) not found at $FORK_LLVM/." >&2
        echo "       Run: FLY_BUNDLE_LLVM=1 ci/linux/install_prerequisites.sh" >&2
        exit 1
    fi
    LLVM_LIBDIR="$FORK_LLVM/lib"
    LLD_INC="$FORK_LLVM/include"
    LLD_LIBDIR="$FORK_LLVM/lib"
    LLVM_LINK="-lLLVM"
    LLVM_SOFILE="$FORK_LLVM/lib/libLLVM.so"
    echo "bundle: using fork LLVM tree ($FORK_LLVM, libxml2 OFF)"

    # 1) Bundle ONE libLLVM.so into lib/ under its SONAME (== the DT_NEEDED the
    #    binaries below record), so RUNPATH resolution finds this copy.
    LLVM_SO="$(readlink -f "$LLVM_SOFILE")"
    SONAME="$(readelf -d "$LLVM_SO" | sed -n 's/.*Library soname: \[\(.*\)\].*/\1/p')"
    if [ -z "$SONAME" ]; then
        echo "error: could not read libLLVM SONAME from '$LLVM_SO'." >&2
        exit 1
    fi
    echo "bundling $SONAME into $LIB ..."
    cp "$LLVM_SO" "$LIB/$SONAME"

    # lld's static archives directly reference the compression libs (deflate/…) that
    # libLLVM.so itself links, and llvm-config here can't report them cleanly (dylib
    # build). Derive exactly what the bundled .so NEEDs so fly-lld resolves them;
    # libc/libstdc++/libm/libgcc come from clang++.
    LLD_SYSLIBS=""
    for _l in z zstd tinfo edit ffi; do
        readelf -d "$LLVM_SO" | grep -q "lib${_l}\.so" && LLD_SYSLIBS="$LLD_SYSLIBS -l${_l}"
    done

    # 2) fly: dynamic link against the bundled libLLVM via rpath (clang++ supplies
    #    crt/libc). $ORIGIN is bin/, so $ORIGIN/../lib is the bundled lib dir.
    echo "linking $OUT/fly against bundled libLLVM (rpath) ..."
    "$CXX" "$OBJ" \
        "$LIB/fly_std_lib.a" "$LIB/fly_runtime_lib.a" \
        -L"$LLVM_LIBDIR" $LLVM_LINK -Wl,-rpath,'$ORIGIN/../lib' \
        -o "$OUT/fly"

    # 3) Bundle a small `fly-lld`: a tiny LLD driver (ci/linux/lld_driver.cpp, ELF
    #    flavor) with lld's own code STATIC (liblld*.a) but LLVM DYNAMIC (shared
    #    with fly via the same bundled libLLVM + rpath). ToolChain.fly prefers
    #    <exe_dir>/fly-lld, so the toolchain needs no system linker either.
    if [ ! -f "$LLD_LIBDIR/liblldELF.a" ]; then
        echo "error: liblldELF.a not found in the fork LLVM tree ($LLD_LIBDIR)." >&2
        exit 1
    fi
    echo "linking $OUT/fly-lld (lld static, LLVM dynamic, rpath) ..."
    "$CXX" ci/linux/lld_driver.cpp -std=c++17 \
        -I"$LLD_INC" \
        -L"$LLD_LIBDIR" -llldELF -llldCommon \
        -L"$LLVM_LIBDIR" $LLVM_LINK -Wl,-rpath,'$ORIGIN/../lib' \
        $LLD_SYSLIBS \
        -o "$OUT/fly-lld"
else
    # Plain build: install the staged compiler as-is (dynamic against system libLLVM).
    cp "$STAGE/fly" "$OUT/fly"
fi

# ── 4) Cleanup: bin/ ships the executable "$OUT/fly" (+ any bundled libLLVM/fly-lld);
#       drop the staging dir and any intermediate objects.
rm -rf "$STAGE"
rm -f "$OUT"/*.o

echo "fly -> $OUT/fly"
echo "std -> $LIB (fly_std_lib.a + *.fly.h + runtime)"
