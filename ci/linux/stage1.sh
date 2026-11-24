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
# `fly --lib` / `fly -c` per artifact (build_runtime, build_std, build_compiler)
# and linked fly in link_bin.sh. Now the Manifest describes the artifacts and
# the seed's own Builder compiles and links them, the same way any Fly project
# is built. Profile `stage1` (`stage1-debug` with FLY_DEBUG_SYMBOLS=1), output
# target/<profile>; see the profiles in Manifest.fly.
#
# One target per `fly build --target`, laid out after each: the seed runs from
# the hardlink build/stage1/bin/fly0, so its <exe>/../lib is build/stage1/lib
# (never the seed's own lib, whose std headers would clash with the in-tree
# std), and every target finds there the headers of the ones before it —
# runtime.fly.h for TLS and std, the std headers for the compiler. `fly build`
# writes a library as one object under the archive's name; it is wrapped in a
# real archive here, as build_runtime.sh / build_std.sh do.
#
# After the link, the std is built AGAIN by the fly just produced: the seed's
# std only served to link that fly; the stage-1 suites and stage 2 use a std
# built by the tree's own compiler.
#
# FLY_STAGE1_TESTS=0 skips the test steps (fast local builds; CI sets it and
# runs the same three suites from its own "Test stage 1" step). The tools
# (fly-lsp, fly-registry) are built at stage 2, by the fly that ships.
# FLY_BUNDLE_LLVM=1 is honoured by link_fly.sh (bundle only, FLY_LINKED=1).
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
AR="${AR:-ar}"
if is_macos; then
    OSKEY=macos
else
    OSKEY=linux
fi
RUNTIME_HDR="${RUNTIME_SRC%.fly}.fly.h"

[ -x build/stage0/bin/fly ] || { echo "error: stage0 compiler missing — run ci/linux/stage0.sh first." >&2; exit 1; }
[ -f build/stage0/lib/llvm.fly.h ] || { echo "error: build/stage0/lib/llvm.fly.h missing — run stage0.sh first." >&2; exit 1; }
rm -rf "$BIN" "$LIB" "$T"
mkdir -p "$BIN" "$LIB"
FLY0="$BIN/fly0"
ln -f build/stage0/bin/fly "$FLY0" 2>/dev/null || cp -f build/stage0/bin/fly "$FLY0"
# The LLVM C API declarations every Fly compiler build needs.
cp -f build/stage0/lib/llvm.fly.h "$LIB/llvm.fly.h"

# fly_build FLY KEY [--force] — `fly build` one Manifest target.
fly_build() {
    local fly="$1" key="$2"; shift 2
    echo "stage1: fly build --profile $PROFILE --target $key ($(basename "$fly"))"
    "$fly" build --profile "$PROFILE" --target "$key" "$@" \
        || { echo "error: fly build --target $key failed" >&2; exit 1; }
}

# install_archive NAME — target/<profile>/NAME → build/stage1/lib/NAME, as a
# real archive (the object `fly build` writes under that name is its member).
install_archive() {
    local src="$T/$1"
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
}

# install_header SRC DST — a generated header into build/stage1/lib, with nested
# generic closers spaced (`>>` → `> >`) as build_runtime.sh does.
install_header() {
    [ -f "$T/$1" ] || { echo "error: $T/$1 was not generated." >&2; exit 1; }
    space_generic_closers "$T/$1" "$LIB/$2"
}

# install_std_headers — every header the std target left in target/<profile>
# (the runtime and TLS ones are installed under their own names above).
install_std_headers() {
    local n=0 h b
    for h in "$T"/*.fly.h; do
        [ -e "$h" ] || continue
        b="$(basename "$h")"
        case "$b" in
            Runtime[A-Z]*|Tls[A-Z]*) continue ;;
        esac
        cp -f "$h" "$LIB/$b"
        n=$((n + 1))
    done
    [ "$n" -gt 0 ] || { echo "error: no std headers in $T." >&2; exit 1; }
    echo "stage1: std -> $LIB (fly_std_lib.a + $n headers)"
}

# ── runtime and TLS archives, built by the seed ───────────────────────────────
fly_build "$FLY0" "runtime-$OSKEY"
install_archive fly_runtime_lib.a
install_header "$RUNTIME_HDR" runtime.fly.h
fly_build "$FLY0" tls-stub
install_archive fly_tls_stub.a
install_header TlsStub.fly.h TlsStub.fly.h
fly_build "$FLY0" "tls-openssl-$OSKEY"
install_archive fly_tls_lib.a

# ── std and the compiler, built by the seed ───────────────────────────────────
fly_build "$FLY0" std
install_archive fly_std_lib.a
install_std_headers
fly_build "$FLY0" fly
[ -x "$T/fly" ] || { echo "error: $T/fly was not linked." >&2; exit 1; }
cp -f "$T/fly" "$BIN/fly"
FLY_LINKED=1 ./ci/linux/link_fly.sh

# ── the std again, built by the stage-1 fly ───────────────────────────────────
fly_build "$BIN/fly" std --force
install_archive fly_std_lib.a
install_std_headers

# ── the stage-1 suites, bottom-up (runtime → std → compiler) ──────────────────
# No FLY override: at STAGE=1 the test scripts default to build/stage1/bin/fly.
[ "$STAGE1_TESTS" = 0 ] || ./ci/linux/test_runtime.sh
[ "$STAGE1_TESTS" = 0 ] || ./ci/linux/test_std.sh
[ "$STAGE1_TESTS" = 0 ] || ./ci/linux/test_compiler.sh

echo "stage1: done — build/stage1/bin/fly"
