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
. ci/linux/platform.sh

[ -f "$OBJ" ] || { echo "error: $OBJ missing." >&2; exit 1; }
[ -f "$LIB/fly_std_lib.a" ] && [ -f "$LIB/fly_runtime_lib.a" ] || { echo "error: std/runtime missing in $LIB — run build_runtime.sh + build_std.sh first." >&2; exit 1; }

FORK_LLVM=build/llvm

# ── macOS: ld64.lld, libSystem, no C start-up objects ────────────────────────
# What clang would pass to Apple's ld: the architecture, the platform with its
# minimum version (13.0, as everything else in the release) and the SDK root
# holding libSystem's .tbd stubs. libSystem is libc, libm, pthreads and the
# compiler-rt builtins at once. ld64.lld signs arm64 output ad hoc by itself.
# TLS: only the stub — there is no OpenSSL on a stock Mac.
if is_macos; then
    LLD64="$FORK_LLVM/bin/ld64.lld"
    [ -x "$LLD64" ] || { echo "error: fork ld64.lld not found at $LLD64 — run ci/linux/stage0.sh." >&2; exit 1; }
    SDK="$(xcrun --show-sdk-path 2>/dev/null || true)"
    [ -d "$SDK" ] || SDK=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
    [ -d "$SDK" ] || { echo "error: no macOS SDK (xcode-select --install)." >&2; exit 1; }
    MAC_ARGS=()
    [ -f "$LIB/fly_tls_stub.a" ] && MAC_ARGS+=("$LIB/fly_tls_stub.a")
    if [ "$WITH_LLVM" = "1" ]; then
        [ -f "$FORK_LLVM/lib/libLLVM.dylib" ] || { echo "error: $FORK_LLVM/lib/libLLVM.dylib not found." >&2; exit 1; }
        if [ -n "${FLY_LINK_RPATH:-}" ]; then
            RUNPATH="$FLY_LINK_RPATH"
        elif [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
            RUNPATH="$EXE_RPATH"
        else
            RUNPATH="$(cd "$FORK_LLVM/lib" && pwd)"
        fi
        MAC_ARGS+=(-L"$FORK_LLVM/lib" -lLLVM -rpath "$RUNPATH")
    fi
    "$LLD64" -arch arm64 -platform_version macos 13.0 13.0 -syslibroot "$SDK" \
        -o "$OUTBIN" "$OBJ" "$LIB/fly_std_lib.a" "$LIB/fly_runtime_lib.a" \
        ${MAC_ARGS[@]+"${MAC_ARGS[@]}"} -lSystem
    exit 0
fi
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
    # Where the executable finds libLLVM at run time. In a bundle (what ships)
    # it is the lib/ next to bin/, i.e. $ORIGIN/../lib — for EVERY executable,
    # not only fly: fly-lsp used to get the absolute build/llvm/lib of the
    # machine that built it, so the released fly-lsp found no libLLVM on any
    # other machine (and the release was not path-independent).
    if [ -n "${FLY_LINK_RPATH:-}" ]; then
        RUNPATH="$FLY_LINK_RPATH"
    elif [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
        RUNPATH='$ORIGIN/../lib'
    else
        RUNPATH="$(cd "$FORK_LLVM/lib" && pwd)"
    fi
    LLVM_ARGS=(-lLLVM -rpath "$RUNPATH")
fi

"$LLD" -pie --hash-style=gnu --eh-frame-hdr -m elf_x86_64 \
    -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o "$OUTBIN" \
    "$MULTIARCH/Scrt1.o" "$MULTIARCH/crti.o" "$GCCDIR/crtbeginS.o" \
    -L"$FORK_LLVM/lib" -L"$GCCDIR" -L"$MULTIARCH" \
    "$OBJ" "$LIB/fly_std_lib.a" "$LIB/fly_runtime_lib.a" \
    "${TLS_ARGS[@]}" "${LLVM_ARGS[@]}" \
    -lstdc++ -lm -lgcc_s -lgcc -lc \
    "$GCCDIR/crtendS.o" "$MULTIARCH/crtn.o"
