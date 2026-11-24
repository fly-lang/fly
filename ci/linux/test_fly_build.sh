#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_fly_build.sh - `fly build` builds Fly itself (0.15.2, plan "level 2"),
# as a self-contained toolchain (0.16: `dir`, `copy`, `rpaths`).
# Linux/macOS mirror of ci/windows/test_fly_build.ps1.
#
# The stage's fly runs `fly build --profile dist` on the repository: the root
# Manifest.fly describes runtime, TLS archives, std, compiler, tools and the
# LLVM pieces a release ships, and everything must come out in target/dist laid
# out as a release - bin/ and lib/. Then, in a bare environment (PATH=/usr/bin:
# /bin, no LD_LIBRARY_PATH / DYLD_LIBRARY_PATH / LIBRARY_PATH, no option
# pointing into build/):
#   1. the fly it built starts - libLLVM is found through its RUNPATH
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

VERSION="$(bash ci/linux/gen_package.sh)"

# -- the toolchain, built by fly build -----------------------------------------
rm -rf target/dist
if ! "$FLY" build --profile dist > "$OUT/build.log" 2>&1; then
    echo "  FAIL  fly build --profile dist"
    tail -20 "$OUT/build.log"
    exit 1
fi
WANT="bin/fly bin/fly-lsp bin/fly-registry bin/lldb bin/lldb-dap lib/fly_std_lib.a lib/fly_runtime_lib.a"
WANT="$WANT lib/fly_tls_stub.a lib/fly_tls_lib.a lib/llvm.fly.h lib/str.fly.h"
if is_macos; then
    WANT="$WANT bin/ld64.lld lib/libLLVM.dylib"
else
    WANT="$WANT bin/ld.lld lib/libLLVM.so.20.1 lib/clang/20/lib/x86_64-unknown-linux-gnu/libclang_rt.builtins.a"
fi
for a in $WANT; do
    [ -f "target/dist/$a" ] || { echo "  FAIL  target/dist/$a missing"; exit 1; }
done
echo "  ok    fly build --profile dist laid out bin/ (fly, tools, linker, debugger) and lib/ (archives, headers, LLVM)"

DIST="$(pwd)/target/dist/bin/fly"
# bare COMMAND... — run it with nothing found through build/ or a loader path.
bare() {
    env -u LD_LIBRARY_PATH -u DYLD_LIBRARY_PATH -u LIBRARY_PATH PATH=/usr/bin:/bin "$@"
}

# -- 1. it starts on its own and reports its version -------------------------------
v="$(bare "$DIST" --version 2>&1 || true)"
case "$v" in
    "fly version $VERSION "*) ;;
    *) echo "  FAIL  target/dist/bin/fly --version in a bare environment: $v (want $VERSION)"; exit 1 ;;
esac
echo "  ok    the built fly starts in a bare environment (RUNPATH) and reports $VERSION (Manifest.fly)"

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
if ! bare "$DIST" --src-dir "$P" -o hello --out-dir "$P" > "$P/compile.log" 2>&1; then
    echo "  FAIL  the built fly compiling a program with no options"
    tail -10 "$P/compile.log"
    exit 1
fi
run="$(bare "$P/hello" 2>&1 || true)"
[ "$run" = "hello 0.25" ] || { echo "  FAIL  program built by the built fly printed: $run"; exit 1; }
echo "  ok    the built fly compiles and runs a program with no options: std, runtime, linker of its own"

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
