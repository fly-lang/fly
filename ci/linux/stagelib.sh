# ─────────────────────────────────────────────────────────────────────────────
# stagelib.sh — shared stage plumbing for the Rust-style bootstrap (sourced by
# the build_*.sh scripts, not executed).
#
#   stage0  downloaded bootstrap compiler (build/bootstrap) + its precompiled lib
#   stage1  stage0 builds runtime, std and the compiler FROM IN-TREE SOURCES into
#           build/stage1/, then links a first bin/fly (build_driver.sh)
#   stage2  that bin/fly rebuilds runtime, std and the driver into the SHIPPED
#           build/bin + build/lib
#
# Set STAGE=1|2 (default 2). Each stage compiles with its own world:
#   stage1: $FLY is hardlinked (same inode — no copy) into build/stage1/bin so
#           /proc/self/exe-based <exe>/../lib discovery serves build/stage1/lib,
#           never the bootstrap's own lib. $SEED = the stage0 lib (the only
#           stage0 artifacts consumed: llvm.fly.h + the runtime's C objects).
#   stage2: $FLY = build/bin/fly, lib = build/lib, $SEED = build/stage1/lib.
#
# Exports: STAGE, FLY (the compiler to run), LIB (its lib = output dir),
#          CDIR (stage-1 compiler artifacts, build-only), SEED (seed source).
# ─────────────────────────────────────────────────────────────────────────────
STAGE="${STAGE:-2}"
S1=build/stage1
CDIR="$S1/compiler"

if [ "$STAGE" = "1" ]; then
    FLY0="${FLY:-fly}"
    case "$FLY0" in */*) ;; *) FLY0="$(command -v "$FLY0" || true)";; esac
    [ -x "$FLY0" ] || { echo "error: stage0 compiler '$FLY0' not found (set FLY=/path/to/fly)" >&2; exit 1; }
    SEED="$(cd "$(dirname "$FLY0")/../lib" && pwd)"
    mkdir -p "$S1/bin" "$S1/lib"
    ln -f "$FLY0" "$S1/bin/fly" 2>/dev/null || cp -f "$FLY0" "$S1/bin/fly"
    FLY="$S1/bin/fly"
    LIB="$S1/lib"
else
    FLY="${FLY:-build/bin/fly}"
    case "$FLY" in */*) ;; *) FLY="$(command -v "$FLY" || true)";; esac
    [ -x "$FLY" ] || { echo "error: stage-1 fly '$FLY' not found — run the stage-1 builds first." >&2; exit 1; }
    SEED="$S1/lib"
    LIB=build/lib
    mkdir -p "$LIB"
fi
