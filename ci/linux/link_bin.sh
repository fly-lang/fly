#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# link_bin.sh — link ONE Fly object into an executable with the fork's ld.lld
# against std, runtime and the host C runtime objects.
#
#   link_bin.sh <object> <output> [--with-llvm]
#
# Extracted from link_fly.sh so a SECOND executable (fly-lsp) can be linked the
# same way. link_fly.sh keeps its own LLVM bundling logic and calls this for the
# link itself.
#
# MONOLITHIC: the compiler is compiled from source INTO the object, never linked
# as a separate archive — a compiler static lib let the linker COMDAT-dedup its
# generic instantiations against the consumer's own copies and produced the
# `fly build` use-after-free.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

OBJ="${1:?usage: link_bin.sh <object> <output> [--with-llvm]}"
OUTBIN="${2:?usage: link_bin.sh <object> <output> [--with-llvm]}"
WITH_LLVM=0
[ "${3:-}" = "--with-llvm" ] && WITH_LLVM=1

STAGE="${STAGE:-1}"
LIB="build/stage$STAGE/lib"
mkdir -p "$(dirname "$OUTBIN")"

[ -f "$OBJ" ] || { echo "error: $OBJ missing." >&2; exit 1; }
[ -f "$LIB/fly_std_lib.a" ] && [ -f "$LIB/fly_runtime_lib.a" ] || { echo "error: std/runtime missing in $LIB — run build_runtime.sh + build_std.sh first." >&2; exit 1; }

FORK_LLVM=build/llvm
LLD="$FORK_LLVM/bin/ld.lld"
[ -x "$LLD" ] || { echo "error: fork ld.lld not found at $LLD — run ci/linux/stage0.sh." >&2; exit 1; }
MULTIARCH=/usr/lib/x86_64-linux-gnu
GCCDIR="$(ls -d /usr/lib/gcc/x86_64-linux-gnu/*/ 2>/dev/null | sort -V | tail -1)"
GCCDIR="${GCCDIR%/}"
if [ ! -f "$MULTIARCH/Scrt1.o" ] || [ ! -f "$GCCDIR/crtbeginS.o" ]; then
    echo "error: host C runtime objects missing (need libc6-dev + gcc)." >&2
    exit 1
fi

# TLS: the stub is ALWAYS linked, because std/lib/net/tls.fly references the
# tls_* symbols unconditionally — without it every link fails with "undefined
# symbol: tls_available". The REAL OpenSSL backend goes in FIRST, and only when
# FLY_WITH_TLS=1 asks for it: archive lazy extraction then leaves the stub member
# unpulled. It is opt-in even for fly itself, so a bootstrap on a box without
# libssl-dev keeps working; tlsAvailable() reports the truth either way.
# AUTO-DETECT: link the real backend when libssl is actually linkable, i.e. when
# the DEVELOPMENT symlink exists — the runtime package alone ships only
# libssl.so.3, which `-lssl` cannot use. Detecting it here means a machine with
# libssl-dev gets working https out of the box (which matters now that curl is
# gone from the driver), while a machine without it keeps linking exactly as
# before. FLY_WITH_TLS=1 forces it on, FLY_WITH_TLS=0 forces it off.
TLS_WANT="${FLY_WITH_TLS:-auto}"
if [ "$TLS_WANT" = "auto" ]; then
    TLS_WANT=0
    for d in /usr/lib/x86_64-linux-gnu /usr/lib64 /usr/lib; do
        if [ -e "$d/libssl.so" ] && [ -e "$d/libcrypto.so" ]; then TLS_WANT=1; break; fi
    done
fi
TLS_ARGS=()
if [ "$TLS_WANT" = "1" ] && [ -f "$LIB/fly_tls_lib.a" ]; then
    TLS_ARGS+=("$LIB/fly_tls_lib.a" -lssl -lcrypto)
fi
[ -f "$LIB/fly_tls_stub.a" ] && TLS_ARGS+=("$LIB/fly_tls_stub.a")

LLVM_ARGS=()
if [ "$WITH_LLVM" = "1" ]; then
    [ -f "$FORK_LLVM/lib/libLLVM.so" ] || { echo "error: $FORK_LLVM/lib/libLLVM.so not found." >&2; exit 1; }
    LLVM_ARGS=(-lLLVM -rpath "${FLY_LINK_RPATH:-$(cd "$FORK_LLVM/lib" && pwd)}")
fi

"$LLD" -pie --hash-style=gnu --eh-frame-hdr -m elf_x86_64 \
    -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o "$OUTBIN" \
    "$MULTIARCH/Scrt1.o" "$MULTIARCH/crti.o" "$GCCDIR/crtbeginS.o" \
    -L"$FORK_LLVM/lib" -L"$GCCDIR" -L"$MULTIARCH" \
    "$OBJ" "$LIB/fly_std_lib.a" "$LIB/fly_runtime_lib.a" \
    "${TLS_ARGS[@]}" "${LLVM_ARGS[@]}" \
    -lstdc++ -lm -lgcc_s -lgcc -lc \
    "$GCCDIR/crtendS.o" "$MULTIARCH/crtn.o"
