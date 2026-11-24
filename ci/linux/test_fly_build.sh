#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_fly_build.sh - the toolchain `fly build` built is one of its own.
# Linux/macOS mirror of ci/windows/test_fly_build.ps1.
#
# From 0.16 stage 2 IS `fly build --profile dist --out-dir build/stage2` on the
# root Manifest.fly (runtime, TLS archives, std, compiler, tools and the LLVM
# pieces a release ships), so build/stage<N> must be a complete release laid
# out as bin/ and lib/. Then, in a bare environment (PATH=/usr/bin:/bin, no
# LD_LIBRARY_PATH / DYLD_LIBRARY_PATH / LIBRARY_PATH, no option pointing into
# build/):
#   1. the stage's fly starts - libLLVM is found through its RUNPATH
#      ($ORIGIN/../lib) alone - and reports the version Manifest.fly declares;
#   2. that fly compiles and runs a program with what lies next to it alone:
#      the std, runtime and compiler-rt builtins in lib/, the linker in bin/;
#   3. a scratch project built with `fly build` reads its own manifest through
#      the generated fly.meta.package module (name() and version()).
# Run from stage2.sh (STAGE=2 by default). Exit 0 = all checks passed.
# -----------------------------------------------------------------------------
set -euo pipefail
cd "$(dirname "$0")/../.."
. ci/linux/platform.sh

STAGE="${STAGE:-2}"
FLY="$(cd "build/stage$STAGE/bin" && pwd)/fly"
OUT=build/test/fly_build
rm -rf "$OUT"
mkdir -p "$OUT"

# The version Manifest.fly declares (`string version = "x.y.z"`, its single source).
VERSION="$(sed -n 's/^[[:space:]]*string[[:space:]]\{1,\}version[[:space:]]*=[[:space:]]*"\([0-9]*\.[0-9]*\.[0-9]*\)".*/\1/p' Manifest.fly | head -1)"
[ -n "$VERSION" ] || { echo '  FAIL  Manifest.fly has no `string version = "x.y.z"` line'; exit 1; }

# -- the layout of the release ----------------------------------------------------
R="build/stage$STAGE"
WANT="bin/fly bin/fly-lsp bin/fly-registry bin/lldb bin/lldb-dap lib/fly_std_lib.a lib/fly_runtime_lib.a"
WANT="$WANT lib/fly_tls_stub.a lib/fly_tls_lib.a lib/llvm.fly.h lib/str.fly.h"
if is_macos; then
    WANT="$WANT bin/ld64.lld lib/libLLVM.dylib"
else
    WANT="$WANT bin/ld.lld lib/libLLVM.so.20.1 lib/clang/20/lib/x86_64-unknown-linux-gnu/libclang_rt.builtins.a"
fi
for a in $WANT; do
    [ -f "$R/$a" ] || { echo "  FAIL  $R/$a missing"; exit 1; }
done
echo "  ok    $R holds bin/ (fly, tools, linker, debugger) and lib/ (archives, headers, LLVM)"

# bare COMMAND... — run it with nothing found through build/ or a loader path.
bare() {
    env -u LD_LIBRARY_PATH -u DYLD_LIBRARY_PATH -u LIBRARY_PATH PATH=/usr/bin:/bin "$@"
}

# -- 1. it starts on its own and reports its version -------------------------------
v="$(bare "$FLY" --version 2>&1 || true)"
case "$v" in
    "fly version $VERSION "*) ;;
    *) echo "  FAIL  $R/bin/fly --version in a bare environment: $v (want $VERSION)"; exit 1 ;;
esac
echo "  ok    the stage's fly starts in a bare environment (RUNPATH) and reports $VERSION (Manifest.fly)"

# -- 2. it compiles and runs a program with what lies next to it --------------------
P="$(cd "$OUT" && pwd)/hello"
mkdir -p "$P"
cat > "$P/main.fly" <<'EOF'
import fly.os.io
import fly.str

void main() {
    io.printLn(str.concat("hello ", str.convertD(0.25)))
}
EOF
if ! bare "$FLY" --src-dir "$P" -o hello --out-dir "$P" > "$P/compile.log" 2>&1; then
    echo "  FAIL  the stage's fly compiling a program with no options"
    tail -10 "$P/compile.log"
    exit 1
fi
run="$(bare "$P/hello" 2>&1 || true)"
[ "$run" = "hello 0.25" ] || { echo "  FAIL  program built by the stage's fly printed: $run"; exit 1; }
echo "  ok    the stage's fly compiles and runs a program with no options: std, runtime, linker of its own"

# -- 3. a project reads its own manifest (fly.meta.package) -------------------------
J="$(cd "$OUT" && pwd)/proj"
mkdir -p "$J/src"
cat > "$J/Manifest.fly" <<'EOF'
import fly.meta.*

class Manifest : fly.meta.Manifest {
    string name = "pkgdemo"
    string version = "3.2.1"
    Target[] targets = { new Target("pkgdemo", "src/main.fly", "") }
}
EOF
cat > "$J/src/main.fly" <<'EOF'
import fly.os.io
import fly.str
import fly.meta.package

void main() {
    io.printLn(str.concat(package.name(), str.concat(" ", package.version())))
}
EOF
if ! (cd "$J" && "$FLY" build > "$J/build.log" 2>&1); then
    echo "  FAIL  fly build of a project"
    tail -10 "$J/build.log"
    exit 1
fi
pr="$("$J/target/debug/pkgdemo" 2>&1 || true)"
[ "$pr" = "pkgdemo 3.2.1" ] || { echo "  FAIL  fly.meta.package: the program printed '$pr'"; exit 1; }
echo "  ok    a program reads its manifest: fly.meta.package name() and version()"

echo "test_fly_build: all checks passed"
