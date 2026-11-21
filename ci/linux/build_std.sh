#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_std.sh — build the standard library from std/lib sources into
# build/stage$STAGE/lib. Requires build_runtime.sh first (std imports
# fly.runtime/fly.llvm). Output: $LIB/fly_std_lib.a + one .fly.h per module.
#
# Who compiles it:
#   STAGE=1                      the SEED (a self-host release since 0.15). This
#                                build only serves to link the stage-1 compiler,
#                                which the seed also compiles.
#   STAGE=1 FLY_STD_BUILDER=self the stage-1 fly, right after link_fly: it
#                                replaces the seed's std in build/stage1/lib, so
#                                the stage-1 suites and stage 2 use a std built
#                                by the compiler of THIS tree.
#   STAGE=2                      the stage-1 fly, like every stage-2 artifact.
#                                This is the std that ships.
# Every builder is a self-host compiler, so the invocation is the same; only
# the binary differs.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

# ── Stage plumbing: pick the compiler and the in/out dirs from $STAGE. ────────
STAGE="${STAGE:-1}"
LIB="build/stage$STAGE/lib"; mkdir -p "$LIB"
BY_SEED=0
if [ "$STAGE" = "1" ] && [ "${FLY_STD_BUILDER:-seed}" != "self" ]; then
    BY_SEED=1
    FLY=build/stage1/bin/fly0           # seed hardlink: <exe>/../lib → build/stage1/lib
    [ -x build/stage0/bin/fly ] || { echo "error: stage0 compiler missing — run ci/linux/stage0.sh first." >&2; exit 1; }
    mkdir -p build/stage1/bin
    ln -f build/stage0/bin/fly "$FLY" 2>/dev/null || cp -f build/stage0/bin/fly "$FLY"
else
    FLY=build/stage1/bin/fly            # the fly linked by stage1
    [ -x "$FLY" ] || { echo "error: stage1 fly '$FLY' not found — run ci/linux/stage1.sh first." >&2; exit 1; }
fi

[ -f "$LIB/fly_runtime_lib.a" ] && [ -f "$LIB/runtime.fly.h" ] || { echo "error: runtime missing in $LIB — run build_runtime.sh first." >&2; exit 1; }

T=build/tmp_std
rm -rf "$T"; mkdir -p "$T"
STD=std/lib

# DIRECTORY CLI: --lib compiles the whole --src-dir — std/lib IS the library.
# FLY_DEBUG_SYMBOLS=1 → emit DWARF so a self-host crash symbolizes to a source line.
DBG=""; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && DBG="--debug-symbols"
# A warning in the tree's own code fails the build when the self-host compiles it
# (the seed predates --warnings-as-errors).
WERR=""; [ "$BY_SEED" = 1 ] || WERR="--warnings-as-errors"
WHO="the seed"; [ "$BY_SEED" = 1 ] || WHO="the stage-1 fly"
echo "stage$STAGE: compiling std/lib with $WHO ...${DBG:+ (+debug-symbols)}"
# --lib emits one merged object; archive it. --out-dir is required: without it
# the self-host writes each module's `.fly.h` next to its source, inside std/lib.
"$FLY" --lib $DBG $WERR -o fly_std_lib --out-dir "$T" --src-dir "$STD"
[ -f "$T/fly_std_lib" ] || { echo "error: std object not emitted." >&2; exit 1; }
rm -f "$LIB/fly_std_lib.a"
"${AR:-ar}" rcs "$LIB/fly_std_lib.a" "$T/fly_std_lib"

# A lib directory holds `.fly.h` and nothing else: a header carries declarations,
# and for a module that declares generics it carries their source too. Copied
# VERBATIM. Nested `>>` used to be spaced by a sed pass here; the compiler now
# emits them spaced itself, which is the only safe place for it — rewriting the
# file would also hit the `>>` of a real right-shift inside a template body.
hdrs=0
for h in "$T"/*.fly.h; do
    [ -e "$h" ] || continue
    cp "$h" "$LIB/$(basename "$h")"
    hdrs=$((hdrs + 1))
done

rm -rf "$T"
echo "stage$STAGE: std -> $LIB/fly_std_lib.a (+ $hdrs *.fly.h)"
