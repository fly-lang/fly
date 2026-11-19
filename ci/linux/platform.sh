#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# platform.sh — what differs between the two POSIX hosts these scripts serve,
# Linux x86_64 and macOS arm64 (Apple Silicon). Sourced, never executed:
#
#   . "$(dirname "$0")/platform.sh"
#
# Everything a script needs to branch on is set here once, so the build and test
# scripts stay one set for both systems. Only bash 3.2 features are used: that is
# the bash /bin/bash still is on macOS.
# ─────────────────────────────────────────────────────────────────────────────

case "$(uname -s)" in
    Darwin)
        HOST_PLATFORM="macos-arm64"
        DYLIB_EXT="dylib"
        LOADER_PATH_VAR="DYLD_LIBRARY_PATH"   # the dynamic loader's search path
        RUNTIME_SRC="RuntimeMacos.fly"
        EXE_RPATH='@loader_path/../lib'        # Mach-O spelling of $ORIGIN/../lib
        ;;
    *)
        HOST_PLATFORM="linux-x86_64"
        DYLIB_EXT="so"
        LOADER_PATH_VAR="LD_LIBRARY_PATH"
        RUNTIME_SRC="RuntimeLinux.fly"
        EXE_RPATH='$ORIGIN/../lib'
        ;;
esac

is_macos() { [ "$HOST_PLATFORM" = "macos-arm64" ]; }

# sha256_of FILE — sha256sum on Linux, shasum on macOS (which has no sha256sum).
sha256_of() {
    if command -v sha256sum > /dev/null 2>&1; then
        sha256sum "$1" | cut -d' ' -f1
    else
        shasum -a 256 "$1" | cut -d' ' -f1
    fi
}

# space_generic_closers IN OUT — rewrite `>>` as `> >` so nested generic closers
# re-lex as two tokens. perl, not `sed ':a;…;ta'`: BSD sed rejects a label that
# is not on a line of its own.
space_generic_closers() {
    perl -pe '1 while s/>>/> >/' "$1" > "$2"
}

# prepend_loader_path DIR — put DIR first on the dynamic loader's search path
# (LD_LIBRARY_PATH / DYLD_LIBRARY_PATH), once.
prepend_loader_path() {
    local cur
    eval "cur=\${$LOADER_PATH_VAR:-}"
    case ":$cur:" in
        *":$1:"*) ;;
        *) export "$LOADER_PATH_VAR=$1${cur:+:$cur}" ;;
    esac
}

# with_timeout SECONDS CMD... — run CMD, killed after SECONDS. macOS has no
# coreutils `timeout`; perl's alarm survives the exec, on both systems.
with_timeout() {
    local secs="$1"
    shift
    perl -e 'alarm shift; exec @ARGV or die "exec: $!"' "$secs" "$@"
}
