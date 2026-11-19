#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# stage0.sh — set up stage 0 of the staged bootstrap on Linux:
#
#   • the fork LLVM toolchain (fly-lang/llvm-project release) → build/llvm
#     libLLVM.so + ld.lld + llvm-config + compiler-rt builtins. NO system
#     package manager: everything the build needs comes from this tarball.
#   • the pinned bootstrap `fly` release ($FLY_VERSION below) → build/stage0
#     (bin/ + precompiled lib/)
#
# stage1.sh / stage2.sh build on top of these (see the stage map in stage1.sh).
#
# Local use: SOURCE it so PATH/env persist in your shell:
#     . ./ci/linux/stage0.sh
#     ./ci/linux/stage1.sh && ./ci/linux/stage2.sh
#
# In CI ($GITHUB_ENV set) it appends to $GITHUB_ENV / $GITHUB_PATH instead of the
# process environment - auto-detected below.
# -----------------------------------------------------------------------------

# Fail fast only when executed (not sourced): `set -e` in a sourced script would
# kill the caller's interactive shell on any error.
(return 0 2>/dev/null) && SOURCED=1 || SOURCED=0
[ "$SOURCED" -eq 0 ] && set -euo pipefail

# Resolve the project root (this script lives in ci/linux/, two levels down).
# BASH_SOURCE works whether the script is executed or sourced.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Bootstrap compiler release used to compile the std --lib archive + the self-host
# sources. Must ship the ptrsize header-gen fix (fly Frontend.cpp typeStr).
# 0.13.13 brought the array subscript `k[i]`, `C[N]` declarations and
# reference-counted array buffers — the syntax the tree may use is bounded by
# what the seed can parse, since it compiles the whole tree at stage 1.
# 0.13.14 is the first STD-LESS seed: the package is just bin/fly +
# lib/{llvm.fly.h, runtime.fly.h, fly_runtime_lib.a} — the std lives in-tree
# and stage 1 builds it from source. It also carries the inherited-interface
# sema fix and the __out_N StringRef-dangle fix.
# 0.13.15 drops the CLI test runner (--test/--suite): a lone `suite` in a
# linking root is now the structural test entry, and more than one is an error.
# It is the first seed that never runs anything it builds — stage 1 tests with
# the fly it just linked instead (see stage1.sh).
# 0.13.16 makes a generated `.fly.h` a DECLARATION file: no `{}` stubs, the
# `abstract` modifier preserved, and a module holding generic templates shipped
# as a `.fly` SOURCE instead (the specializer needs the bodies). It is the seed
# that builds the shipped std, so only a seed change can alter that shape.
# 0.13.17 folds that companion back in: `.fly.h` is the ONLY textual file a lib
# directory holds, carrying declarations plus the source of the templates alone —
# not the whole module. It also spaces nested generic closers itself, so the
# build script no longer rewrites `>>` over shipped code.
# 0.13.18 replaces the implicit `out` return variable with `return expr` (and
# `return a, b` for a multi-return). The lowering is unchanged — the value is
# still stored into a hidden trailing out parameter, then `ret void` — so the ABI
# is the same; what changed is the surface syntax, plus two rules the seed now
# enforces: a value function must return on every path, and a struct local that
# every return hands back is built directly in the caller's slot (NRVO).
FLY_VERSION="${FLY_VERSION:-0.13.18}"

BUILD_DIR="$ROOT/build"
STAGE0_DIR="$BUILD_DIR/stage0"
FLY_BIN="$STAGE0_DIR/bin/fly"

# --- Host platform -------------------------------------------------------------
# Linux x86_64 or macOS arm64 (platform.sh): which archives to fetch, how to hash
# them, the shared-library suffix and the loader's search variable.
. "$SCRIPT_DIR/platform.sh"
if is_macos; then
    LLVM_ASSET="v@V@-macos-aarch64/llvm-@V@-aarch64-apple-darwin.tar.gz"
else
    LLVM_ASSET="v@V@-linux-x86_64/llvm-@V@-x86_64-linux-gnu.tar.gz"
fi

# --- LLVM 20 (fly-lang/llvm-project fork, downloaded — NO package manager) ------
# Everything comes from the project's OWN LLVM build (LLVM_BUILD_LLVM_DYLIB=ON +
# LLVM_ENABLE_LIBXML2=OFF + compiler-rt): a libLLVM.so with no external deps, the
# lld linker, and libclang_rt.builtins.a — which the fly ToolChain links into every
# executable it produces (this used to come from an apt LLVM; see the symlink below).
# The tarball is ~1 GB; cache build/llvm in CI. The bundle build additionally needs
# the lld static archives + LLVM/lld headers.
LLVM_VERSION="${LLVM_VERSION:-20.1.8}"
FORK_LLVM="$BUILD_DIR/llvm"
# On macOS the builtins are the `osx` flavour and libSystem re-exports them, so
# fly links none — but the archive must still carry them (a release asset that
# lost compiler-rt is a broken asset).
if [ "$HOST_PLATFORM" = "macos-arm64" ]; then
    RT_BUILTINS="$FORK_LLVM/lib/clang/${LLVM_VERSION%%.*}/lib/darwin/libclang_rt.osx.a"
else
    RT_BUILTINS="$FORK_LLVM/lib/clang/${LLVM_VERSION%%.*}/lib/x86_64-unknown-linux-gnu/libclang_rt.builtins.a"
fi
NEED_STATIC=0
[ "${FLY_BUNDLE_LLVM:-0}" = "1" ] && [ ! -f "$FORK_LLVM/lib/liblldELF.a" ] && [ "$HOST_PLATFORM" = "linux-x86_64" ] && NEED_STATIC=1
# llvm_sha256 VERSION PLATFORM — the pinned SHA-256 of the fork LLVM archive for
# that platform. Taken from the release asset (GitHub records the same digest).
# When bumping LLVM_VERSION, pin the new hashes here AND in ci/windows/stage0.ps1.
llvm_sha256() {
    case "$1/$2" in
        20.1.8/linux-x86_64) echo "ce7a10e2365a4343ae073133e33bdcc3d1ba0d4dc7904f0ec22365c0e6677231" ;;
        *)                   echo "" ;;
    esac
}
LLVM_SHA256="${FLY_LLVM_SHA256:-$(llvm_sha256 "$LLVM_VERSION" "$HOST_PLATFORM")}"
if [ ! -f "$FORK_LLVM/lib/libLLVM.$DYLIB_EXT" ] || [ ! -f "$RT_BUILTINS" ] || [ ! -x "$FORK_LLVM/bin/lldb" ] || [ "$NEED_STATIC" = "1" ]; then
    url="https://github.com/fly-lang/llvm-project/releases/download/${LLVM_ASSET//@V@/$LLVM_VERSION}"
    mkdir -p "$BUILD_DIR"
    tarball="$BUILD_DIR/fork-llvm.tar.gz"
    # Atomic download (.part + mv, with retries): an interrupted curl must never
    # leave a truncated tarball that a rerun would mistake for the real one.
    if [ ! -f "$tarball" ]; then
        curl -fSL --retry 6 --retry-delay 15 --retry-all-errors -o "$tarball.part" "$url"
        mv -f "$tarball.part" "$tarball"
    fi
    # Verify before extracting, exactly like the seed below: the toolchain links
    # every fly-produced program, so an unverified archive is as much a hole as an
    # unverified seed. A mismatch deletes the archive so a rerun downloads again.
    if [ "$LLVM_SHA256" = "none" ]; then
        echo "warning: FLY_LLVM_SHA256=none — the LLVM archive is NOT verified." >&2
    elif [ -z "$LLVM_SHA256" ]; then
        rm -f "$tarball"
        echo "error: no pinned SHA-256 for LLVM $LLVM_VERSION — add it to llvm_sha256() in ci/linux/stage0.sh" >&2
        echo "       (or set FLY_LLVM_SHA256; FLY_LLVM_SHA256=none skips the check, for testing only)." >&2
        [ "$SOURCED" -eq 0 ] && exit 1 || return 1
    else
        got="$(sha256_of "$tarball")"
        if [ "$got" != "$LLVM_SHA256" ]; then
            rm -f "$tarball"
            echo "error: the LLVM $LLVM_VERSION archive does not match its pinned SHA-256 ($url)" >&2
            echo "       expected sha256 $LLVM_SHA256" >&2
            echo "       got      sha256 $got" >&2
            [ "$SOURCED" -eq 0 ] && exit 1 || return 1
        fi
        echo "stage0: LLVM $LLVM_VERSION verified (sha256 $got)"
    fi
    # Always: the shared libLLVM.so (link + runtime), the lld linker, llvm-config,
    # the compiler-rt builtins (linked into every fly-produced executable) and the
    # debugger the release bundles — lldb + liblldb + lldb-server (lldb launches
    # local processes through it) + lldb-dap (IDE/DAP) + lldb-argdumper, all under
    # their ORIGINAL LLVM names. Keep this list in sync with the guard above AND
    # the fly-llvm cache key in .github/workflows/build-linux.yml.
    # Bundle also: the lld static archives + LLVM/lld headers.
    paths="llvm/lib/libLLVM.so* llvm/bin/ld.lld llvm/bin/lld llvm/bin/llvm-config llvm/lib/clang"
    paths="$paths llvm/bin/lldb llvm/bin/lldb-server llvm/bin/lldb-dap llvm/bin/lldb-argdumper llvm/lib/liblldb.so*"
    [ "${FLY_BUNDLE_LLVM:-0}" = "1" ] && paths="$paths llvm/lib/liblld*.a llvm/include"
    if [ "$HOST_PLATFORM" = "macos-arm64" ]; then
        # bsdtar has no --wildcards and the macOS lldb layout differs (no
        # lldb-server: local processes go through debugserver), so take the
        # whole archive.
        tar -xzf "$tarball" -C "$BUILD_DIR"
    else
        # shellcheck disable=SC2086
        tar -xzf "$tarball" -C "$BUILD_DIR" --wildcards $paths   # → $BUILD_DIR/llvm/
    fi
    rm -f "$tarball"
fi
# The codegen bridge emits `-lLLVM-20` (see compiler/lib/codegen/LLVMApi.fly); the fork
# ships the shared lib as `libLLVM.so` / `libLLVM.dylib`, so alias it to that link name.
[ -e "$FORK_LLVM/lib/libLLVM-20.$DYLIB_EXT" ] || ln -sf "libLLVM.$DYLIB_EXT" "$FORK_LLVM/lib/libLLVM-20.$DYLIB_EXT"

# compiler-rt builtins are a hard link-time requirement (GetCompilerRTBuiltinsPath
# errors out without them) — fail loudly here rather than at the first test link.
if [ ! -f "$RT_BUILTINS" ]; then
    echo "error: $RT_BUILTINS missing — the fork LLVM tarball did not provide compiler-rt." >&2
    echo "       (delete build/llvm and rerun; if it persists the fork release lacks compiler-rt)" >&2
    [ "$SOURCED" -eq 0 ] && exit 1 || return 1
fi

# No system LLVM, and no `sudo`: the self-host ToolChain finds the compiler-rt
# builtins next to its own binary (link_fly.sh copies them into
# build/stage<N>/lib/clang/…, the layout the release ships) or in build/llvm.
# This used to `sudo ln -s build/llvm /usr/lib/llvm-20` so the ToolChain's
# system-path probe would hit — which also hid that a release without the
# builtins could link nothing on a host without LLVM 20.

# --- Download the bootstrap fly (stage 0) --------------------------------------
# Reuse an existing seed only when it is EXACTLY the pinned version (local
# convenience; a fresh CI runner never has one).
#
# Presence alone is not enough: after a pin bump, a checkout that already had the
# previous seed would keep it forever and this script would still report the
# pinned version it had not installed. The failure that follows is thoroughly
# misleading — stage1 compiles std with an old seed and a handful of suites fail
# on constructs the language does support.
#
# The downloaded archive is checked against a PINNED SHA-256 before it is
# extracted. The seed compiles the whole tree at stage 1, so it is the root of
# trust of every artifact Fly ships: an archive that is not byte-for-byte the one
# this pin was taken from must not be used. That covers a corrupted or tampered
# download, and also a seed RE-CUT under the same version number — which the
# `--version` check above cannot see. A re-cut must therefore update the hash
# here (and in ci/windows/stage0.ps1) in the same change.
#
# seed_sha256 VERSION — the pinned SHA-256 of the Linux archive of that seed.
# Taken from the published release asset (GitHub records the same digest):
#   0.13.18  fly-0.13.18-linux-x86_64.tar.gz, released 2026-09-20 from the
#            `bootstrap` branch at 32528a0bf (sources = release/0.13.X df0b20f0c)
#   0.13.18  fly-0.13.18-macos-arm64.tar.gz: the same sources (release/0.13.X =
#            the v0.13.18 tag + a CI-only commit) built on macos-latest and
#            attached to the existing release by build-macos.yml
seed_sha256() {
    case "$1/$2" in
        0.13.18/linux-x86_64) echo "f328c4d081de52140e3e704c6f73d71792f21b34b7039be8dc4470b7fb1bbf50" ;;
        *)                    echo "" ;;
    esac
}
# FLY_SEED_SHA256 overrides the table: the hash of a candidate seed under test,
# or `none` to skip verification (testing only — never set it in CI).
SEED_SHA256="${FLY_SEED_SHA256:-$(seed_sha256 "$FLY_VERSION" "$HOST_PLATFORM")}"

have=""
if [ -x "$FLY_BIN" ]; then
    have="$("$FLY_BIN" --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
fi

if [ "$have" != "$FLY_VERSION" ]; then
    [ -n "$have" ] && echo "stage0: seed is $have, pinned is $FLY_VERSION — fetching the pinned one."
    if [ -z "$SEED_SHA256" ]; then
        echo "error: no pinned SHA-256 for seed v$FLY_VERSION." >&2
        echo "       add it to seed_sha256() in ci/linux/stage0.sh, or set FLY_SEED_SHA256" >&2
        echo "       (FLY_SEED_SHA256=none skips the check — for testing a candidate seed only)." >&2
        [ "$SOURCED" -eq 0 ] && exit 1 || return 1
    fi
    url="https://github.com/fly-lang/fly/releases/download/v${FLY_VERSION}/fly-${FLY_VERSION}-${HOST_PLATFORM}.tar.gz"
    # Staged swap: an existing seed is replaced only AFTER the new one is in
    # hand. Deleting first would strand a machine whose pinned release is not
    # published yet — which is a real state during a seed re-cut.
    rm -rf "$STAGE0_DIR.new"
    mkdir -p "$STAGE0_DIR.new"
    if curl -fsSL --retry 6 --retry-delay 15 --retry-all-errors "$url" -o "$BUILD_DIR/fly.tar.gz"; then
        if [ "$SEED_SHA256" = "none" ]; then
            echo "warning: FLY_SEED_SHA256=none — the seed archive is NOT verified." >&2
        else
            got="$(sha256_of "$BUILD_DIR/fly.tar.gz")"
            if [ "$got" != "$SEED_SHA256" ]; then
                rm -rf "$STAGE0_DIR.new" "$BUILD_DIR/fly.tar.gz"
                echo "error: seed v$FLY_VERSION failed verification: the archive is not the pinned one." >&2
                echo "       expected sha256 $SEED_SHA256" >&2
                echo "       got      sha256 $got" >&2
                echo "       a corrupted download, or the release was re-cut: if the re-cut is intended," >&2
                echo "       update seed_sha256() in this script and in ci/windows/stage0.ps1." >&2
                [ -n "$have" ] && echo "       the existing $have seed was left in place." >&2
                [ "$SOURCED" -eq 0 ] && exit 1 || return 1
            fi
            echo "stage0: seed v$FLY_VERSION verified (sha256 $got)"
        fi
        tar -xzf "$BUILD_DIR/fly.tar.gz" -C "$STAGE0_DIR.new"
        rm -f "$BUILD_DIR/fly.tar.gz"
        rm -rf "$STAGE0_DIR"
        mv "$STAGE0_DIR.new" "$STAGE0_DIR"
        chmod +x "$FLY_BIN"
    else
        rm -rf "$STAGE0_DIR.new" "$BUILD_DIR/fly.tar.gz"
        echo "error: seed v$FLY_VERSION is not downloadable ($url)." >&2
        if [ -n "$have" ]; then
            echo "       the existing $have seed was left in place, but it does NOT match the pin:" >&2
            echo "       stage1 will compile std with it and can fail on newer constructs." >&2
        fi
        exit 1
    fi
fi

# --- Environment -------------------------------------------------------------
# The build/test scripts derive their compiler from $STAGE (see stage1.sh /
# stage2.sh) — no $FLY export needed. Only the fork LLVM is wired up: its bin on
# PATH (ld.lld, llvm-config), its lib on LIBRARY_PATH (the `-lLLVM-20` link) and
# LD_LIBRARY_PATH (the non-bundled staged `fly` loads libLLVM.so at run time).
if [ -n "${GITHUB_ENV:-}" ]; then
    echo "$FORK_LLVM/bin"                              >> "$GITHUB_PATH"
    echo "LIBRARY_PATH=$FORK_LLVM/lib:${LIBRARY_PATH:-}"       >> "$GITHUB_ENV"
    echo "$LOADER_PATH_VAR=$FORK_LLVM/lib:${!LOADER_PATH_VAR:-}" >> "$GITHUB_ENV"
else
    export PATH="$FORK_LLVM/bin:$PATH"
    export LIBRARY_PATH="$FORK_LLVM/lib:${LIBRARY_PATH:-}"
    export "$LOADER_PATH_VAR=$FORK_LLVM/lib:${!LOADER_PATH_VAR:-}"
    echo "stage0 ready:"
    echo "  fly $FLY_VERSION  -> $FLY_BIN"
    echo "  LLVM $LLVM_VERSION (fork) -> PATH += $FORK_LLVM/bin ; LIBRARY_PATH/$LOADER_PATH_VAR += $FORK_LLVM/lib"
fi
