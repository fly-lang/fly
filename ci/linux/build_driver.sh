#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_driver.sh — compile the driver (driver/lib, entry Driver.fly) into a
# single merged object at build/stage$STAGE/driver/Driver.o; link_fly.sh then
# links it into the fly executable. Run with STAGE=1 (stage0 compiles) or
# STAGE=2 (the stage1 fly recompiles it); see stage1.sh for the stage map.
# The compiler archive + headers always come from build/stage1/compiler
# (build-only, never shipped).
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

[ -f "$CDIR/fly_compiler_lib.a" ] || { echo "error: $CDIR/fly_compiler_lib.a missing — run build_compiler.sh first." >&2; exit 1; }
[ -f "$LIB/fly_std_lib.a" ] && [ -f "$LIB/fly_runtime_lib.a" ] || { echo "error: std/runtime missing in $LIB — run build_runtime.sh + build_std.sh first." >&2; exit 1; }

# The driver is always compiled FROM SOURCE — never consumed as a header.
for h in "$CDIR"/*.fly.h "$LIB"/*.fly.h; do
    [ -e "$h" ] || continue
    if grep -q "namespace fly.driver" "$h" 2>/dev/null; then rm -f "$h"; fi
done

# ── Emit the merged driver object (-L serves the compiler headers). ───────────
# Kept in build/stage$STAGE/driver (with emit.log) for link_fly.sh + debugging.
D="build/stage$STAGE/driver"
rm -rf "$D"; mkdir -p "$D"
echo "stage$STAGE: compiling driver ..."
if [ "$STAGE" = "1" ]; then
    # stage0 reference: no -c — its in-process link fails on the LLVM C-API
    # symbols (resolved only by -lLLVM at link time) but emits the object first.
    "$FLY" driver/lib/Driver.fly "$CDIR/fly_compiler_lib.a" --src-dir driver/lib -L "$CDIR" \
        -o fly --out-dir "$D" > "$D/emit.log" 2>&1 || true
    OBJ="$D/Driver.fly.o"
else
    # self-host: -c emits a clean object, no link attempt.
    "$FLY" driver/lib/Driver.fly --src-dir driver/lib -L "$CDIR" \
        -c -o Driver --out-dir "$D" > "$D/emit.log" 2>&1 || true
    OBJ="$D/Driver"
fi
if [ ! -f "$OBJ" ]; then
    echo "error: driver object not emitted; see $D/emit.log:" >&2
    grep -m5 -E 'error:|broken|abort' "$D/emit.log" | sed 's/^/      /' >&2 || true
    exit 1
fi
mv -f "$OBJ" "$D/Driver.o"

echo "stage$STAGE: driver -> $D/Driver.o"
