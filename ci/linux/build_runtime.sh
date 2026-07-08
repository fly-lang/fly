#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_runtime.sh — build the Fly runtime from runtime/lib/runtime.fly.
# Run with STAGE=1 (stage0 compiler) or STAGE=2 (self-host); see stagelib.sh.
#
# The runtime is Fly layered over C primitives (mem_alloc, copyCStr, …) whose .c
# sources live only in the reference repo — the two seeds taken from $SEED are
# llvm.fly.h (generated bridge header, no source here) and the runtime archive
# (for its C members; the Fly member is REPLACED by this build).
# Output: $LIB/fly_runtime_lib.a + runtime.fly.h.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
. ci/linux/stagelib.sh

T=build/tmp_runtime
rm -rf "$T"; mkdir -p "$T"
AR="${AR:-ar}"

# stage seeds (refreshed every run so a stale lib never wins)
[ -f "$SEED/llvm.fly.h" ] && [ -f "$SEED/fly_runtime_lib.a" ] || { echo "error: seeds missing in $SEED — run the previous stage first (or set FLY to a valid stage0)." >&2; exit 1; }
cp -f "$SEED/llvm.fly.h" "$LIB/"
cp -f "$SEED/fly_runtime_lib.a" "$LIB/"

# Keep ONLY the C-primitive members (*.c.o) in the seeded archive: the Fly member's
# NAME varies by producer (bootstrap/reference: `runtime.fly.o` or `fly_runtime_lib`;
# self-host: `fly_runtime_lib`), so a name-based `ar r` swap could leave a stale
# duplicate defining every fly.runtime symbol twice.
for m in $("$AR" t "$LIB/fly_runtime_lib.a"); do
    case "$m" in *.c.o) ;; *) "$AR" d "$LIB/fly_runtime_lib.a" "$m" ;; esac
done

# --src-dir $T (an empty dir): the same-namespace source scan would otherwise pull
# runtime-macos.fly / runtime-windows.fly (same `namespace fly.runtime`) into this
# Linux build — three definitions of every C-ABI symbol, wrong-platform code.
echo "stage$STAGE: compiling runtime/lib/runtime.fly ..."
if [ "$STAGE" = "1" ]; then
    # stage0 reference: --lib emits the archive itself; merge its member(s) in.
    "$FLY" --lib -o "$T/fly_runtime_lib" --src-dir "$T" runtime/lib/runtime.fly
    for m in $("$AR" t "$T/fly_runtime_lib.a"); do
        (cd "$T" && "$AR" x fly_runtime_lib.a "$m")
        "$AR" r "$LIB/fly_runtime_lib.a" "$T/$m"
    done
else
    # self-host: --lib emits one merged object; add it.
    "$FLY" --lib -o fly_runtime_lib --out-dir "$T" --src-dir "$T" runtime/lib/runtime.fly
    [ -f "$T/fly_runtime_lib" ] || { echo "error: runtime object not emitted." >&2; exit 1; }
    "$AR" r "$LIB/fly_runtime_lib.a" "$T/fly_runtime_lib"
fi
"$AR" s "$LIB/fly_runtime_lib.a"

# headers (nested `>>` spaced so re-reads lex them; idempotent)
for h in "$T"/*.fly.h; do
    [ -e "$h" ] || continue
    sed -E ':a;s/>>/> >/;ta' "$h" > "$LIB/$(basename "$h")"
done

rm -rf "$T"
echo "stage$STAGE: runtime -> $LIB/fly_runtime_lib.a (+ runtime.fly.h)"
