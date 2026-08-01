#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# build_registry.sh - build the fly-registry package server into
# build/stage$STAGE/bin. Mirror of ci/windows/build_registry.ps1.
#
# std-only: unlike fly-lsp this never reaches the compiler, so no
# --src-dir compiler/lib and no LLVM on the link.
# -----------------------------------------------------------------------------
set -e
cd "$(dirname "$0")/../.."

# EXPORTED, not just local: link_bin.sh derives its own lib dir from $STAGE and
# defaults to 1 (see build_lsp.sh).
export STAGE="${STAGE:-2}"
OUT="build/stage$STAGE/bin"
LIB="build/stage$STAGE/lib"
D="build/stage$STAGE/registry"
mkdir -p "$OUT" "$D"

FLY="build/stage$STAGE/bin/fly"
[ -x "$FLY" ] || { echo "error: compiler '$FLY' not found - run link_fly.sh for this stage first." >&2; exit 1; }
[ -f tools/registry/lib/FlyRegistry.fly ] || { echo "error: tools/registry/lib/FlyRegistry.fly not found." >&2; exit 1; }

DBG=""; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && DBG="--debug-symbols"

echo "stage$STAGE: compiling tools/registry ..."
"$FLY" --entry tools/registry/lib/FlyRegistry.fly --src-dir tools/registry \
       $DBG -c -o FlyRegistry --out-dir "$D" -L "$LIB"

OBJ=""
for c in "$D/FlyRegistry" "$D/FlyRegistry.o" "$D/FlyRegistry.fly.o"; do
    [ -f "$c" ] && { OBJ="$c"; break; }
done
[ -n "$OBJ" ] || { echo "error: fly-registry object not emitted in $D." >&2; exit 1; }
[ "$OBJ" = "$D/FlyRegistry.o" ] || mv -f "$OBJ" "$D/FlyRegistry.o"

./ci/linux/link_bin.sh "$D/FlyRegistry.o" "$OUT/fly-registry"

echo "stage$STAGE: fly-registry -> $OUT/fly-registry"
exit 0
