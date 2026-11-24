#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_fly_build.sh - `fly build` builds Fly itself (0.15.2, plan "level 2").
# Linux/macOS mirror of ci/windows/test_fly_build.ps1.
#
# The stage's fly runs `fly build --profile dist` on the repository: the root
# Manifest.fly describes runtime, TLS archives, std, compiler and tools, and
# everything must come out in target/dist. Then:
#   1. the fly it built reports the version Manifest.fly declares;
#   2. that fly compiles and runs a program against the std and runtime built
#      next to it;
#   3. a scratch project built with `fly build` reads its own manifest through
#      the generated fly.meta.package module (name() and version()).
# release/0.16.X bootstraps with `fly build`: this is what it will rely on.
# Run from stage2.sh (STAGE=2 by default). Exit 0 = all checks passed.
# -----------------------------------------------------------------------------
set -euo pipefail
cd "$(dirname "$0")/../.."

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
for a in fly fly-lsp fly-registry fly_std_lib.a fly_runtime_lib.a fly_tls_stub.a; do
    [ -f "target/dist/$a" ] || { echo "  FAIL  target/dist/$a missing"; exit 1; }
done
echo "  ok    fly build --profile dist built runtime, TLS, std, fly, fly-lsp, fly-registry"

# -- 1. its version --------------------------------------------------------------
v="$(target/dist/fly --version 2>&1 || true)"
case "$v" in
    "fly version $VERSION "*) ;;
    *) echo "  FAIL  target/dist/fly --version: $v (want $VERSION)"; exit 1 ;;
esac
echo "  ok    the built fly reports $VERSION (Manifest.fly)"

# -- 2. it compiles and runs a program on its own std and runtime ------------------
P="$OUT/hello"
mkdir -p "$P"
cat > "$P/main.fly" <<'EOF'
import fly.os.io
import fly.str

void main() {
    io.printLn(str.concat("hello ", str.convertD(0.25)))
}
EOF
if ! target/dist/fly --src-dir "$P" -o hello --out-dir "$P" --runtime-lib-dir target/dist -L target/dist \
        > "$P/compile.log" 2>&1; then
    echo "  FAIL  the built fly compiling a program"
    tail -10 "$P/compile.log"
    exit 1
fi
run="$("$P/hello" 2>&1 || true)"
[ "$run" = "hello 0.25" ] || { echo "  FAIL  program built by the built fly printed: $run"; exit 1; }
echo "  ok    the built fly compiles and runs a program on the std and runtime built with it"

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
