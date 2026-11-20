#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build_runtime.sh — build the Fly runtime from runtime/lib/RuntimeLinux.fly into
# build/stage$STAGE/lib. Run with STAGE=1 (the stage0 bootstrap compiles) or
# STAGE=2 (the stage1 fly recompiles); see stage1.sh for the stage map.
#
# What goes into the archive:
#   STAGE=2  the Fly runtime ALONE — this is what ships. RuntimeLinux.fly
#            defines every primitive anything references (a census found 61 also
#            defined by the seed's C objects, and the C-only rest referenced by
#            nothing), and the self-host lowers its four `__atomic_*_4` calls to
#            LLVM atomic instructions (CodeGenIntrinsics.genAtomicBuiltin), so no
#            libatomic is needed either.
#   STAGE=1  the seed's C primitive objects (Env.c.o, Fs.c.o, … — sources only in
#            the 0.13.x repo), kept from the seed's archive, plus the Fly member
#            WEAKENED so the C definitions win. The seed compiles this runtime and
#            lowers `__atomic_*_4` to calls nothing on Linux defines; the C
#            objects stand in. Stage 1 is the seed's world and never ships.
# Up to 0.14.7 the stage-2 archive kept the C objects too.
# Output: $LIB/fly_runtime_lib.a + runtime.fly.h.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
. ci/linux/platform.sh

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
    SEED=build/stage1/lib               # llvm.fly.h comes from the stage1 build
    FLY=build/stage1/bin/fly            # the fly linked by stage1
    [ -x "$FLY" ] || { echo "error: stage1 fly '$FLY' not found — run ci/linux/stage1.sh first." >&2; exit 1; }
fi

T=build/tmp_runtime
rm -rf "$T"; mkdir -p "$T"
AR="${AR:-ar}"
OBJCOPY="${OBJCOPY:-objcopy}"

# llvm.fly.h (refreshed every run so a stale copy never wins)
[ -f "$SEED/llvm.fly.h" ] || { echo "error: $SEED/llvm.fly.h missing — run the previous stage first." >&2; exit 1; }
cp -f "$SEED/llvm.fly.h" "$LIB/"
# A fresh archive every run; at stage 1 it starts from the seed's C members only.
rm -f "$LIB/fly_runtime_lib.a"
if [ "$STAGE" = "1" ] && ! is_macos; then
    [ -f "$SEED/fly_runtime_lib.a" ] || { echo "error: $SEED/fly_runtime_lib.a missing — run stage0 first." >&2; exit 1; }
    cp -f "$SEED/fly_runtime_lib.a" "$LIB/"
    # Keep ONLY the C members (*.c.o): the Fly member's NAME varies by producer
    # (`runtime.fly.o` / `fly_runtime_lib`), so a name-based swap could leave a
    # stale duplicate defining every fly.runtime symbol twice.
    for m in $("$AR" t "$LIB/fly_runtime_lib.a"); do
        case "$m" in *.c.o) ;; *) "$AR" d "$LIB/fly_runtime_lib.a" "$m" ;; esac
    done
fi

# DIRECTORY CLI: --lib compiles the whole --src-dir; the one runtime source is
# STAGED into the temp dir so exactly that file is the library (its
# same-namespace siblings RuntimeMacos/RuntimeWindows must stay out — pulling
# them in would define every C-ABI symbol three times with wrong-platform code).
# FLY_DEBUG_SYMBOLS=1 → emit DWARF so a self-host crash symbolizes to a source line.
DBG=""; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && DBG="--debug-symbols"
echo "stage$STAGE: compiling runtime/lib/$RUNTIME_SRC ...${DBG:+ (+debug-symbols)}"
cp -f "runtime/lib/$RUNTIME_SRC" "$T/$RUNTIME_SRC"
if [ "$STAGE" = "1" ] && is_macos; then
    # macOS: no seed C objects to keep. They exist on Linux only because the
    # seed lowers `__atomic_*_4` to calls no Linux library defines; libSystem
    # exports them. So the stage-1 archive is the Fly member alone, as at stage 2
    # (and no `objcopy --weaken`, which Mach-O objects do not support reliably).
    # The seed's archive is taken whole: member by member, `ar t` also lists the
    # Darwin symbol table `__.SYMDEF`, which `ar x` extracts unreadable (mode 0).
    "$FLY" --lib $DBG -o "$T/fly_runtime_lib" --src-dir "$T"
    cp -f "$T/fly_runtime_lib.a" "$LIB/fly_runtime_lib.a"
elif [ "$STAGE" = "1" ]; then
    # stage0 reference: --lib emits the archive itself; merge its member(s) in,
    # WEAKENED so the kept C objects' strong definitions win (no duplicate-symbol
    # error); the Fly-only symbols it uniquely provides are still used.
    "$FLY" --lib $DBG -o "$T/fly_runtime_lib" --src-dir "$T"
    for m in $("$AR" t "$T/fly_runtime_lib.a"); do
        (cd "$T" && "$AR" x fly_runtime_lib.a "$m")
        "$OBJCOPY" --weaken "$T/$m"
        "$AR" rc "$LIB/fly_runtime_lib.a" "$T/$m"
    done
else
    # self-host: --lib emits one merged object — the whole archive.
    "$FLY" --lib $DBG -o fly_runtime_lib --out-dir "$T" --src-dir "$T"
    [ -f "$T/fly_runtime_lib" ] || { echo "error: runtime object not emitted." >&2; exit 1; }
    "$AR" rc "$LIB/fly_runtime_lib.a" "$T/fly_runtime_lib"
fi
"$AR" s "$LIB/fly_runtime_lib.a"
# What ships carries no object the seed compiled.
if [ "$STAGE" != "1" ] && "$AR" t "$LIB/fly_runtime_lib.a" | grep -q '\.c\.o$'; then
    echo "error: the stage-$STAGE runtime archive contains C objects from the seed." >&2; exit 1
fi

# headers (nested `>>` spaced so re-reads lex them; idempotent). The compiler names
# the emitted header after the source (RuntimeLinux.fly.h); std/compiler compile
# against runtime.fly.h, so CANONICALISE the single runtime header to that fixed
# name regardless of the source basename.
for h in "$T"/*.fly.h; do
    [ -e "$h" ] || continue
    space_generic_closers "$h" "$LIB/runtime.fly.h"
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
[ -f "$T2/TlsStub.fly.h" ] && space_generic_closers "$T2/TlsStub.fly.h" "$LIB/TlsStub.fly.h"

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
