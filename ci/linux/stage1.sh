#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# stage1.sh — stage 1 of the Rust-style staged bootstrap (Linux and macOS):
#
#   stage0  the downloaded seed + its precompiled lib (build/stage0, set up by
#           stage0.sh — which also fetches LLVM). From 0.16 the seed is the
#           0.15.2 self-host release, the first one that can build Fly with
#           `fly build`.
#   stage1  THIS SCRIPT: the seed runs `fly build` on the root Manifest.fly —
#           runtime, TLS archives, std, compiler, one target at a time — and
#           the results are laid out in build/stage1. THEN the suites run with
#           build/stage1/bin/fly, the compiler this stage just produced.
#   stage2  stage2.sh: build/stage1/bin/fly rebuilds the shipped artifacts
#           into build/stage2/ (the self-hosting fixpoint check).
#
# `fly build` (0.16): up to 0.15 this stage drove the seed with one hand-written
# `fly --lib` / `fly -c` per artifact and linked fly by hand (those scripts are
# gone since 0.16). Now the Manifest describes the artifacts and
# the seed's own Builder compiles and links them, the same way any Fly project
# is built. Profile `stage1` (`stage1-debug` with FLY_DEBUG_SYMBOLS=1), output
# target/<profile>; see the profiles in Manifest.fly.
#
# One target per `fly build --target`, laid out after each: the seed runs from
# the hardlink build/stage1/bin/fly0, so its <exe>/../lib is build/stage1/lib
# (never the seed's own lib, whose std headers would clash with the in-tree
# std), and every target finds there the headers of the ones before it —
# the runtime's for TLS and std, the std headers for the compiler. `fly build`
# writes a library as one object under the archive's name; it is wrapped in a
# real archive here.
#
# The seed (0.15.2) knows neither `dir` nor `rpaths` and writes every target
# flat into target/<profile>, but expands `@out/lib` — where the targets find
# the libraries before them — to target/<profile>/lib: that directory is kept a
# mirror of build/stage1/lib. The stage-1 fly honours `dir`, so the std it
# rebuilds lands in target/<profile>/lib.
#
# After the link, the std is built AGAIN by the fly just produced: the seed's
# std only served to link that fly; the stage-1 suites and stage 2 use a std
# built by the tree's own compiler.
#
# FLY_STAGE1_TESTS=0 skips the test steps (fast local builds; CI sets it and
# runs the same three suites from its own "Test stage 1" step). The tools
# (fly-lsp, fly-registry) are built at stage 2, by the fly that ships.
# FLY_BUNDLE_LLVM=1 adds the LLVM bundle (libLLVM, the linker, the debugger):
# the Manifest's copy targets, run by the stage-1 fly.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
export STAGE=1
. ci/linux/platform.sh

STAGE1_TESTS="${FLY_STAGE1_TESTS:-1}"
BIN=build/stage1/bin
LIB=build/stage1/lib
PROFILE=stage1; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && PROFILE=stage1-debug
T="target/$PROFILE"
TLIB="$T/lib"
AR="${AR:-ar}"
if is_macos; then
    OSKEY=macos
else
    OSKEY=linux
fi

[ -x build/stage0/bin/fly ] || { echo "error: stage0 compiler missing — run ci/linux/stage0.sh first." >&2; exit 1; }
[ -f build/stage0/lib/llvm.fly.h ] || { echo "error: build/stage0/lib/llvm.fly.h missing — run stage0.sh first." >&2; exit 1; }
rm -rf "$BIN" "$LIB" "$T" build/stage1/.fly
mkdir -p "$BIN" "$LIB" "$TLIB"
FLY0="$BIN/fly0"
ln -f build/stage0/bin/fly "$FLY0" 2>/dev/null || cp -f build/stage0/bin/fly "$FLY0"
# The LLVM C API declarations every Fly compiler build needs.
cp -f build/stage0/lib/llvm.fly.h "$LIB/llvm.fly.h"
cp -f build/stage0/lib/llvm.fly.h "$TLIB/llvm.fly.h"
# Where the target just built left its output: flat for the seed, lib/ for the
# stage-1 fly (switched before the rebuild below).
SRC="$T"

# fly_build FLY KEY [--force] — `fly build` one Manifest target.
fly_build() {
    local fly="$1" key="$2"; shift 2
    echo "stage1: fly build --profile $PROFILE --target $key ($(basename "$fly"))"
    "$fly" build --profile "$PROFILE" --target "$key" "$@" \
        || { echo "error: fly build --target $key failed" >&2; exit 1; }
}

# install_archive NAME — the built library NAME → build/stage1/lib/NAME (and the
# target/<profile>/lib mirror), as a real archive: the object `fly build`
# writes under that name becomes its member.
install_archive() {
    local src="$SRC/$1"
    [ -f "$src" ] || { echo "error: $src was not built." >&2; exit 1; }
    rm -f "$LIB/$1"
    if [ "$(head -c 7 "$src")" = '!<arch>' ]; then
        cp -f "$src" "$LIB/$1"
    else
        local o="build/tmp_stage1/${1%.a}.o"
        mkdir -p build/tmp_stage1
        cp -f "$src" "$o"
        "$AR" rcs "$LIB/$1" "$o"
        rm -rf build/tmp_stage1
    fi
    cp -f "$LIB/$1" "$TLIB/$1"
}

# install_headers — every header the targets built so far generated, under the
# name `fly build` gives it (RuntimeLinux.fly.h, TlsStub.fly.h, ...): stage 2
# lays out the same names, so the lib/ the stage-2 build reads headers from (this
# one) and the one it writes hold the same files — which the stage-3 fixpoint
# needs (a header present in both is read from the same place at both stages).
install_headers() {
    local n=0 h b
    for h in "$SRC"/*.fly.h; do
        [ -e "$h" ] || continue
        b="$(basename "$h")"
        cp -f "$h" "$LIB/$b"
        [ "$SRC" = "$TLIB" ] || cp -f "$h" "$TLIB/$b"
        n=$((n + 1))
    done
    [ "$n" -gt 0 ] || { echo "error: no headers in $SRC." >&2; exit 1; }
    echo "stage1: headers -> $LIB ($n)"
}

# ── runtime and TLS archives, built by the seed ───────────────────────────────
fly_build "$FLY0" "runtime-$OSKEY"
install_archive fly_runtime_lib.a
install_headers
fly_build "$FLY0" tls-stub
install_archive fly_tls_stub.a
fly_build "$FLY0" "tls-openssl-$OSKEY"
install_archive fly_tls_lib.a
install_headers

# ── std and the compiler, built by the seed ───────────────────────────────────
fly_build "$FLY0" std
install_archive fly_std_lib.a
install_headers
fly_build "$FLY0" fly
[ -x "$T/fly" ] || { echo "error: $T/fly was not linked." >&2; exit 1; }
cp -f "$T/fly" "$BIN/fly"
# The `fly.meta.package` module the seed's Builder wrote for the compiler, where
# stage 2 keeps its own (build/stage2/.fly/gen/fly): test_compiler and test_tools
# compile the compiler sources with the stage's copy.
PKG=build/stage1/.fly/gen/fly
mkdir -p "$PKG"
cp -f "$T/.fly/gen/fly/package.fly" "$PKG/package.fly"

# ── the LLVM pieces, by the stage-1 fly (the Manifest's copy targets) ─────────
# Into build/stage1 as stage 2 lays them out; the seed knows no copy targets.
# The stage-1 fly carries no RUNPATH: it finds libLLVM through the loader path
# stage0.sh exports. Always the compiler-rt builtins (Linux; libSystem has them
# on macOS), linked into every program fly produces; with FLY_BUNDLE_LLVM=1 also
# libLLVM, the linker and the debugger.
llvm_copy() {
    echo "stage1: fly build --profile $PROFILE --target $1 --out-dir build/stage1 (fly)"
    "$BIN/fly" build --profile "$PROFILE" --target "$1" --out-dir build/stage1 \
        || { echo "error: fly build --target $1 failed" >&2; exit 1; }
}
is_macos || llvm_copy llvm-linux-builtins
if [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
    llvm_copy "llvm-$OSKEY-bin"
    llvm_copy "llvm-$OSKEY-lib"
fi

# ── the std again, built by the stage-1 fly (which honours `dir`) ─────────────
fly_build "$BIN/fly" std --force
SRC="$TLIB"
install_archive fly_std_lib.a
install_headers

# ── the stage-1 suites, bottom-up (runtime → std → compiler) ──────────────────
# No FLY override: at STAGE=1 the test scripts default to build/stage1/bin/fly.
[ "$STAGE1_TESTS" = 0 ] || ./ci/linux/test_runtime.sh
[ "$STAGE1_TESTS" = 0 ] || ./ci/linux/test_std.sh
[ "$STAGE1_TESTS" = 0 ] || ./ci/linux/test_compiler.sh

echo "stage1: done — build/stage1/bin/fly"
