#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_compiler.sh — compile the compiler (monolithic: the whole compiler + the
# driver, entry compiler/lib/driver/Driver.fly)
# into a single merged object at build/stage$STAGE/driver/Driver.o; link_fly.sh
# then links it into the fly executable. Run with STAGE=1 (stage0 compiles) or
# STAGE=2 (the stage1 fly recompiles it); see stage1.sh for the stage map.
# MONOLITHIC: the compiler is compiled from source INTO this object (--src-dir compiler),
# not linked as a static archive — see the note by the compile step below.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

# ── Stage plumbing: pick the compiler and the in/out dirs from $STAGE. ────────
STAGE="${STAGE:-1}"
LIB="build/stage$STAGE/lib"
CDIR=build/stage1/compiler
if [ "$STAGE" = "1" ]; then
    FLY=build/stage1/bin/fly0           # stage0 hardlink: <exe>/../lib → build/stage1/lib
    [ -x build/stage0/bin/fly ] || { echo "error: stage0 compiler missing — run ci/linux/stage0.sh first." >&2; exit 1; }
    mkdir -p build/stage1/bin
    ln -f build/stage0/bin/fly "$FLY" 2>/dev/null || cp -f build/stage0/bin/fly "$FLY"
else
    FLY=build/stage1/bin/fly            # the fly linked by stage1
    [ -x "$FLY" ] || { echo "error: stage1 fly '$FLY' not found — run ci/linux/stage1.sh first." >&2; exit 1; }
fi

[ -f "$LIB/fly_std_lib.a" ] && [ -f "$LIB/fly_runtime_lib.a" ] || { echo "error: std/runtime missing in $LIB — run build_runtime.sh + build_std.sh first." >&2; exit 1; }

# MONOLITHIC build: the compiler is compiled FROM SOURCE into the driver object
# (`--src-dir compiler` resolves fly.compiler.* from compiler/lib source; std stays an
# external archive). There is NO fly_compiler_lib.a static archive anymore — as a
# static lib the compiler's GENERIC INSTANTIATIONS (List<ASTNode> ...) were
# COMDAT-deduped by the linker against the driver's own copies, causing a
# use-after-free of a parsed module's List fields (the `fly build` Windows crash,
# root-caused 2026-07-19). Merging the compiler in removes the cross-archive dedup.
# The driver is always source (never a header).
for h in "$LIB"/*.fly.h; do
    [ -e "$h" ] || continue
    if grep -q "namespace fly.driver" "$h" 2>/dev/null; then rm -f "$h"; fi
done

# ── Emit the merged driver+compiler object. ──────────────────────────────────
# Kept in build/stage$STAGE/driver (with emit.log) for link_fly.sh + debugging.
D="build/stage$STAGE/driver"
rm -rf "$D"; mkdir -p "$D"
# FLY_DEBUG_SYMBOLS=1 → emit DWARF so a self-host crash symbolizes to a source line.
DBG=""; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && DBG="--debug-symbols"
echo "stage$STAGE: compiling driver + compiler (monolithic, from source) ...${DBG:+ (+debug-symbols)}"
# DIRECTORY CLI (seed and self-host alike): no positional — the entry (the
# single main(), compiler/lib/driver/Driver.fly) is discovered from --src-dir
# and its import closure pulls the whole compiler.
if [ "$STAGE" = "1" ]; then
    # stage0 reference: no -c — its in-process link fails on the LLVM C-API
    # symbols (resolved only by -lLLVM at link time) but emits the object first.
    "$FLY" --src-dir compiler \
        $DBG -o fly --out-dir "$D" > "$D/emit.log" 2>&1 || true
    OBJ="$D/Driver.fly.o"
else
    # self-host: -c emits a clean object, no link attempt.
    "$FLY" --src-dir compiler \
        $DBG -c -o Driver --out-dir "$D" > "$D/emit.log" 2>&1 || true
    OBJ="$D/Driver"
fi
if [ ! -f "$OBJ" ]; then
    echo "error: driver object not emitted; see $D/emit.log:" >&2
    grep -m5 -E 'error:|broken|abort' "$D/emit.log" | sed 's/^/      /' >&2 || true
    exit 1
fi
mv -f "$OBJ" "$D/Driver.o"

echo "stage$STAGE: driver -> $D/Driver.o"
