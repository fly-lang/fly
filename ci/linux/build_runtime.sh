#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_runtime.sh — build the Fly runtime from runtime/lib/RuntimeLinux.fly into
# build/stage$STAGE/lib. Run with STAGE=1 (the stage0 bootstrap compiles) or
# STAGE=2 (the stage1 fly recompiles); see stage1.sh for the stage map.
#
# The runtime is Fly layered over C primitives (mem_alloc, copyCStr, …) whose .c
# sources live only in the reference repo — the two seeds taken from the previous
# stage's lib are llvm.fly.h (generated bridge header, no source here) and the
# runtime archive (for its C members; the Fly member is REPLACED by this build).
# Output: $LIB/fly_runtime_lib.a + runtime.fly.h.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

# ── Stage plumbing: pick the compiler and the in/out dirs from $STAGE. ────────
STAGE="${STAGE:-1}"
LIB="build/stage$STAGE/lib"; mkdir -p "$LIB"
if [ "$STAGE" = "1" ]; then
    SEED=build/stage0/lib               # seeds come from the bootstrap's precompiled lib
    FLY=build/stage1/bin/fly0           # stage0 hardlink: <exe>/../lib → build/stage1/lib
    [ -x build/stage0/bin/fly ] || { echo "error: stage0 compiler missing — run ci/linux/stage0.sh first." >&2; exit 1; }
    mkdir -p build/stage1/bin
    ln -f build/stage0/bin/fly "$FLY" 2>/dev/null || cp -f build/stage0/bin/fly "$FLY"
else
    SEED=build/stage1/lib               # seeds come from the stage1 build
    FLY=build/stage1/bin/fly            # the fly linked by stage1
    [ -x "$FLY" ] || { echo "error: stage1 fly '$FLY' not found — run ci/linux/stage1.sh first." >&2; exit 1; }
fi

T=build/tmp_runtime
rm -rf "$T"; mkdir -p "$T"
AR="${AR:-ar}"
OBJCOPY="${OBJCOPY:-objcopy}"

# stage seeds (refreshed every run so a stale lib never wins)
[ -f "$SEED/llvm.fly.h" ] && [ -f "$SEED/fly_runtime_lib.a" ] || { echo "error: seeds missing in $SEED — run the previous stage first." >&2; exit 1; }
cp -f "$SEED/llvm.fly.h" "$LIB/"
cp -f "$SEED/fly_runtime_lib.a" "$LIB/"

# Keep ONLY the C-primitive members (*.c.o) in the seeded archive: the Fly member's
# NAME varies by producer (bootstrap/reference: `runtime.fly.o` or `fly_runtime_lib`;
# self-host: `fly_runtime_lib`), so a name-based `ar r` swap could leave a stale
# duplicate defining every fly.runtime symbol twice.
for m in $("$AR" t "$LIB/fly_runtime_lib.a"); do
    case "$m" in *.c.o) ;; *) "$AR" d "$LIB/fly_runtime_lib.a" "$m" ;; esac
done

# DIRECTORY CLI: --lib compiles the whole --src-dir; the one runtime source is
# STAGED into the temp dir so exactly that file is the library (its
# same-namespace siblings RuntimeMacos/RuntimeWindows must stay out — pulling
# them in would define every C-ABI symbol three times with wrong-platform code).
# FLY_DEBUG_SYMBOLS=1 → emit DWARF so a self-host crash symbolizes to a source line.
DBG=""; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && DBG="--debug-symbols"
echo "stage$STAGE: compiling runtime/lib/RuntimeLinux.fly ...${DBG:+ (+debug-symbols)}"
cp -f runtime/lib/RuntimeLinux.fly "$T/RuntimeLinux.fly"
if [ "$STAGE" = "1" ]; then
    # stage0 reference: --lib emits the archive itself; merge its member(s) in.
    "$FLY" --lib $DBG -o "$T/fly_runtime_lib" --src-dir "$T"
    for m in $("$AR" t "$T/fly_runtime_lib.a"); do
        (cd "$T" && "$AR" x fly_runtime_lib.a "$m")
        # WEAKEN the fresh Fly runtime member: it redefines the C-ABI symbols
        # (mem_alloc, fs_*, …) that the kept seed `.c.o` C primitives also provide.
        # As a weak member the seed's strong defs win (no duplicate-symbol error);
        # the Fly-only symbols it uniquely provides (e.g. dir_open) are still used.
        # (stage0 emits these STRONG; the self-host already emits weak C-ABI wrappers.)
        "$OBJCOPY" --weaken "$T/$m"
        "$AR" r "$LIB/fly_runtime_lib.a" "$T/$m"
    done
else
    # self-host: --lib emits one merged object; add it (weakened, as above).
    "$FLY" --lib $DBG -o fly_runtime_lib --out-dir "$T" --src-dir "$T"
    [ -f "$T/fly_runtime_lib" ] || { echo "error: runtime object not emitted." >&2; exit 1; }
    "$OBJCOPY" --weaken "$T/fly_runtime_lib"
    "$AR" r "$LIB/fly_runtime_lib.a" "$T/fly_runtime_lib"
fi
"$AR" s "$LIB/fly_runtime_lib.a"

# headers (nested `>>` spaced so re-reads lex them; idempotent). The compiler names
# the emitted header after the source (RuntimeLinux.fly.h); std/compiler compile
# against runtime.fly.h, so CANONICALISE the single runtime header to that fixed
# name regardless of the source basename.
for h in "$T"/*.fly.h; do
    [ -e "$h" ] || continue
    sed -E ':a;s/>>/> >/;ta' "$h" > "$LIB/runtime.fly.h"
done

# ── TLS: the always-linked stub, plus the real OpenSSL backend ────────────────
#
# TWO separate archives, and the separation is the whole design:
#   fly_tls_stub.a  — built and linked on every platform; its header carries the
#                     canonical tls_* declarations std/lib/net/tls.fly needs.
#   fly_tls_lib.a   — the real OpenSSL implementation, linked ONLY under --tls,
#                     AHEAD of the stub. Archive lazy extraction then means the
#                     stub member is never pulled, with no duplicate symbol.
# Its header is DISCARDED: emitting both would declare tls_* twice in fly.runtime.
#
# Neither goes inside fly_runtime_lib.a, because that archive's member is always
# extracted — which would make `-lssl` mandatory for every program, breaking hello
# world on any box without libssl-dev.
T2=build/tmp_tls
rm -rf "$T2"; mkdir -p "$T2"
echo "stage$STAGE: compiling runtime/lib/TlsStub.fly ..."
cp -f runtime/lib/TlsStub.fly "$T2/TlsStub.fly"
if [ "$STAGE" = "1" ]; then
    "$FLY" --lib $DBG -o "$T2/fly_tls_stub" -L "$LIB" --src-dir "$T2"
else
    "$FLY" --lib $DBG -o fly_tls_stub --out-dir "$T2" -L "$LIB" --src-dir "$T2"
fi
STUB=""
for c in "$T2/fly_tls_stub.a" "$T2/fly_tls_stub"; do [ -f "$c" ] && { STUB="$c"; break; }; done
[ -n "$STUB" ] || { echo "error: tls stub library not emitted." >&2; exit 1; }
if [ "${STUB##*.}" = "a" ]; then cp -f "$STUB" "$LIB/fly_tls_stub.a"
else "$AR" rcs "$LIB/fly_tls_stub.a" "$STUB"; fi
[ -f "$T2/TlsStub.fly.h" ] && sed -E ':a;s/>>/> >/;ta' "$T2/TlsStub.fly.h" > "$LIB/TlsStub.fly.h"

rm -rf "$T2"; mkdir -p "$T2"
echo "stage$STAGE: compiling runtime/lib/TlsOpenSSL.fly (real backend, --tls only) ..."
cp -f runtime/lib/TlsOpenSSL.fly "$T2/TlsOpenSSL.fly"
if [ "$STAGE" = "1" ]; then
    "$FLY" --lib $DBG -o "$T2/fly_tls_lib" -L "$LIB" --src-dir "$T2"
else
    "$FLY" --lib $DBG -o fly_tls_lib --out-dir "$T2" -L "$LIB" --src-dir "$T2"
fi
REALTLS=""
for c in "$T2/fly_tls_lib.a" "$T2/fly_tls_lib"; do [ -f "$c" ] && { REALTLS="$c"; break; }; done
[ -n "$REALTLS" ] || { echo "error: tls backend library not emitted." >&2; exit 1; }
if [ "${REALTLS##*.}" = "a" ]; then cp -f "$REALTLS" "$LIB/fly_tls_lib.a"
else "$AR" rcs "$LIB/fly_tls_lib.a" "$REALTLS"; fi
# header deliberately NOT copied — TlsStub.fly.h is the canonical declaration
rm -rf "$T2"

rm -rf "$T"
echo "stage$STAGE: runtime -> $LIB/fly_runtime_lib.a (+ runtime.fly.h, fly_tls_stub.a, fly_tls_lib.a)"
