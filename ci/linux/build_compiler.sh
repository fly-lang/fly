#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_compiler.sh — compile compiler/lib into ONE archive `fly_compiler_lib.a`
# plus flat `*.fly.h` headers, ready to be LINKED by build_fly.sh (which builds the
# driver + entry into the final `fly` executable).
#
# Layered self-host build (mirrors fly/'s CMake target graph):
#     build_runtime.sh → fly_runtime_lib.a         (+ runtime.fly.h, llvm.fly.h)
#     build_std.sh      → fly_std_lib.a            (+ *.fly.h)      [needs runtime]
#     build_compiler.sh → fly_compiler_lib.a       (+ *.fly.h)      [needs std]   ← HERE
#     build_fly.sh      → bin/fly                   (links the three archives)
#
# Everything lands in build/lib (the release-artifact staging the workflows upload).
# The compiler library is built with the bootstrap's `--lib` (archive + headers);
# std/runtime are consumed as headers from build/lib via <exe>/../lib auto-discovery
# (we run a bootstrap COPY from build/bin so <exe>/../lib resolves to build/lib).
#
# The generated compiler headers are post-processed to SPACE nested generic closers
# (`>>` → `> >`): the reference/bootstrap lexer greedily fuses `>>`, and its parser
# needs the spaced form to re-read a header like `Map<string, List<Symbol> >`.
#
# NOTE: this script builds only the COMPILER library. std + runtime must already be
# in build/lib — run build_runtime.sh and build_std.sh first (or the full CI chain).
# For local convenience it will seed the runtime stubs + (re)build std if missing.
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

OUT=build/bin
LIB=build/lib
STD=std/lib
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
BLIB="$(cd "$(dirname "$FLY")/../lib" 2>/dev/null && pwd || true)"
if [ -z "$BLIB" ] || [ ! -d "$BLIB" ]; then
    echo "error: no lib/ directory next to '$FLY' (expected <exe_dir>/../lib" >&2
    echo "       with llvm.fly.h, runtime.fly.h, fly_runtime_lib.a)." >&2
    echo "       Point FLY at a built bootstrap compiler, e.g. fly/build/bin/fly." >&2
    exit 1
fi

# ── Prerequisites: runtime stubs + compiled std must be in build/lib. Until the
#    dedicated build_runtime.sh / build_std.sh exist in the chain, self-heal here so
#    build_compiler.sh is runnable standalone: seed the runtime bridge headers +
#    archive from the bootstrap's own lib, then (re)build std if it's missing. ─────
if [ ! -f "$LIB/fly_runtime_lib.a" ] || [ ! -f "$LIB/llvm.fly.h" ] || [ ! -f "$LIB/runtime.fly.h" ]; then
    echo "seeding runtime stubs (llvm.fly.h, runtime.fly.h, fly_runtime_lib.a) from $BLIB ..."
    cp "$BLIB/llvm.fly.h" "$BLIB/runtime.fly.h" "$BLIB/fly_runtime_lib.a" "$LIB/"
fi
if [ ! -f "$LIB/fly_std_lib.a" ]; then
    echo "building std into $LIB/fly_std_lib.a (prerequisite) ..."
    "$FLY" --lib -o "$LIB/fly_std_lib" \
        "$STD/assert.fly" "$STD/str.fly" "$STD/math.fly" \
        "$STD/os/time.fly" "$STD/os/env.fly" "$STD/os/path.fly" "$STD/os/io.fly" "$STD/os/fs.fly" \
        "$STD/sync.fly" "$STD/mem.fly" "$STD/bridge/clang.fly" \
        "$STD/data/list.fly" "$STD/data/stack.fly" "$STD/data/queue.fly" "$STD/data/deque.fly" \
        "$STD/data/map.fly" "$STD/data/set.fly" "$STD/data/tree.fly" "$STD/data/wrapper.fly" \
        "$STD/os/proc.fly"
fi

# ── Build the compiler library. Run a bootstrap COPY from build/bin so <exe>/../lib
#    resolves to build/lib: `import fly.data.*`, `fly.str`, `fly.llvm`, `fly.runtime`
#    in the compiler sources resolve from our staged std + runtime headers, and the
#    std/runtime namespaces stay archive-linked (not redefined) in the output. Every
#    file under compiler/lib is an input; the compiler emits ONE archive + a *.fly.h
#    per public type. compiler/test/** is NOT included; compiler/ is a pure library
#    (no entry) — the `main` + driver live in driver/lib and belong to build_driver.sh.
cp "$FLY" "$OUT/fly"
mapfile -t FILES < <(find compiler/lib -name '*.fly' | sort)
echo "compiling ${#FILES[@]} compiler/lib files into $LIB/fly_compiler_lib.a ..."
"$OUT/fly" --lib -o "$LIB/fly_compiler_lib" "${FILES[@]}"
rm -f "$OUT/fly"                 # drop the bootstrap copy

# ── Space nested generic closers in the generated headers (`>>` → `> >`) so the
#    reference/bootstrap parser can re-read them (its lexer fuses `>>`; the loop
#    `:a;s/>>/> >/;ta` handles `>>>` and deeper too). Only the compiler headers nest,
#    but the pass is idempotent and safe on every *.fly.h. ─────────────────────────
for h in "$LIB"/*.fly.h; do
    sed -i -E ':a;s/>>/> >/;ta' "$h"
done

echo "compiler -> $LIB/fly_compiler_lib.a (+ $(ls "$LIB"/*.fly.h | wc -l) *.fly.h headers)"
