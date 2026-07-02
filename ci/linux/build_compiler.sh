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

# Invoke the bootstrap compiler via $FLY (default: `fly` on PATH). CI sets FLY to
# an absolute path: released binaries derive their stdlib dir from the executable
# path, and a bare `fly` resolved from a cwd containing a `fly/` directory fails
# that lookup. A slash-containing path sidesteps it. See ../fly Driver.cpp.
FLY="${FLY:-fly}"

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
cp "$FLY" "$OUT/flyc"
"$OUT/flyc" compiler/Fly.fly \
    --src-dir compiler \
    -o fly --out-dir "$OUT"

# ── 4) Cleanup: bin/ ships only the executable (drop the bootstrap copy + objects).
rm -f "$OUT/flyc" "$OUT"/*.o

echo "fly -> $OUT/fly"
echo "std -> $LIB (fly_std_lib.a + *.fly.h + runtime)"
