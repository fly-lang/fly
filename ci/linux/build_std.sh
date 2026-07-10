#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_std.sh — build the standard library from std/lib sources into
# build/stage$STAGE/lib. Run with STAGE=1 (stage0 compiles) or STAGE=2
# (self-host); see stage1.sh for the stage map. Pure Fly — no seeds. Requires
# build_runtime.sh first (std imports fly.runtime/fly.llvm).
# Output: $LIB/fly_std_lib.a + one .fly.h per module.
#
# NOTE: the SHIPPED std is the stage-1 (reference-ABI) build — stage2.sh copies
# it from build/stage1/lib instead of calling this script. The self-host lays
# out native classes FLAT ({vtable, fields…}) while header consumers assume the
# reference layout ({vtable, base subobjects…, fields}), so a stage-2 std would
# break every class with an interface base (io's FileWriter/Reader: `fd` read
# at offset 8 vs written at 16). Call this with STAGE=2 only once the self-host
# adopts the reference class ABI for based classes.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

# ── Stage plumbing: pick the compiler and the in/out dirs from $STAGE. ────────
STAGE="${STAGE:-1}"
LIB="build/stage$STAGE/lib"; mkdir -p "$LIB"
if [ "$STAGE" = "1" ]; then
    FLY=build/stage1/bin/fly0           # stage0 hardlink: <exe>/../lib → build/stage1/lib
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
FILES=(
    "$STD/assert.fly" "$STD/str.fly" "$STD/math.fly"
    "$STD/os/time.fly" "$STD/os/env.fly" "$STD/os/path.fly" "$STD/os/io.fly" "$STD/os/fs.fly"
    "$STD/sync.fly" "$STD/mem.fly" "$STD/bridge/clang.fly"
    "$STD/data/list.fly" "$STD/data/stack.fly" "$STD/data/queue.fly" "$STD/data/deque.fly"
    "$STD/data/map.fly" "$STD/data/set.fly" "$STD/data/tree.fly" "$STD/data/wrapper.fly"
    "$STD/os/proc.fly"
)

echo "stage$STAGE: compiling ${#FILES[@]} std files ..."
if [ "$STAGE" = "1" ]; then
    # stage0 reference: --lib emits the archive itself.
    "$FLY" --lib -o "$T/fly_std_lib" "${FILES[@]}"
    mv -f "$T/fly_std_lib.a" "$LIB/fly_std_lib.a"
else
    # self-host: --lib emits one merged object; archive it.
    "$FLY" --lib -o fly_std_lib --out-dir "$T" "${FILES[@]}"
    [ -f "$T/fly_std_lib" ] || { echo "error: std object not emitted." >&2; exit 1; }
    rm -f "$LIB/fly_std_lib.a"
    "${AR:-ar}" rcs "$LIB/fly_std_lib.a" "$T/fly_std_lib"
fi

# headers (nested `>>` spaced so re-reads lex them; idempotent)
hdrs=0
for h in "$T"/*.fly.h; do
    [ -e "$h" ] || continue
    sed -E ':a;s/>>/> >/;ta' "$h" > "$LIB/$(basename "$h")"
    hdrs=$((hdrs + 1))
done

rm -rf "$T"
echo "stage$STAGE: std -> $LIB/fly_std_lib.a (+ $hdrs *.fly.h)"
