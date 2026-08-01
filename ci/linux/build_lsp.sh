#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_lsp.sh — build the fly-lsp language server into build/stage$STAGE/bin.
#
# This is the SECOND executable in the tree, and it exists because of --entry:
# `--src-dir compiler/lib` makes the compiler's namespaces importable, but that
# directory also declares driver/Driver.fly's main(). Discovery would then see
# two entry points and stop with "multiple main() functions found". --entry names
# the entry outright, so discovery never runs; the import closure of the named
# file still pulls what it needs from every --src-dir root.
#
# The compiler that BUILDS it is THIS STAGE'S OWN fly, not the seed the rest of
# the stage uses: fly-lsp is a PRODUCT of the toolchain, not part of the
# bootstrap, and --entry is a self-host option the pinned 0.13.x seed rejects.
#
# MONOLITHIC, like build_compiler.sh: the compiler is compiled FROM SOURCE into
# the LSP object rather than linked as an archive (a compiler static lib let the
# linker COMDAT-dedup its generic instantiations → use-after-free).
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

# EXPORTED, not just local: link_bin.sh derives its own lib dir from $STAGE and
# defaults to 1, so a standalone run of this script would otherwise compile the
# object at stage 2 and link it against the stage-1 libraries.
export STAGE="${STAGE:-2}"
OUT="build/stage$STAGE/bin"
LIB="build/stage$STAGE/lib"
D="build/stage$STAGE/lsp"
mkdir -p "$OUT" "$D"

FLY="build/stage$STAGE/bin/fly"
[ -x "$FLY" ] || { echo "error: compiler '$FLY' not found — run link_fly.sh for this stage first." >&2; exit 1; }
[ -f tools/lsp/lib/FlyLsp.fly ] || { echo "error: tools/lsp/lib/FlyLsp.fly not found." >&2; exit 1; }

DBG=""; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && DBG="--debug-symbols"

echo "stage$STAGE: compiling tools/lsp (monolithic, compiler from source) ..."
"$FLY" --entry tools/lsp/lib/FlyLsp.fly --src-dir tools/lsp --src-dir compiler/lib \
       $DBG -c -o FlyLsp --out-dir "$D" -L "$LIB"

# The emitted object's name varies by producer, as in build_compiler.sh.
OBJ=""
for c in "$D/FlyLsp" "$D/FlyLsp.o" "$D/FlyLsp.fly.o"; do
    [ -f "$c" ] && { OBJ="$c"; break; }
done
[ -n "$OBJ" ] || { echo "error: fly-lsp object not emitted in $D." >&2; exit 1; }
[ "$OBJ" = "$D/FlyLsp.o" ] || mv -f "$OBJ" "$D/FlyLsp.o"

# --with-llvm: the import closure reaches the compiler's CodeGen through Sema,
# so the LLVM symbols must resolve even though the LSP never emits code.
ci/linux/link_bin.sh "$D/FlyLsp.o" "$OUT/fly-lsp" --with-llvm

echo "stage$STAGE: fly-lsp -> $OUT/fly-lsp"
