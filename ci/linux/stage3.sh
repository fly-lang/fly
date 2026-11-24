#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# stage3.sh — fixpoint check of the staged bootstrap (see the stage map in
# stage1.sh; Windows mirror: ci/windows/stage3.ps1). CI runs it on every build.
#
#   stage1  built by the SEED (the previous release)   → self-host #1
#   stage2  built by stage1 (self-host #1)             → the SHIPPED compiler
#   stage3  built by stage2 (the shipped compiler)     → THIS SCRIPT
#
# stage2 and stage3 are compiled from the SAME sources; they differ only in the
# compiler that compiled them. If the shipped compiler generates the same code
# as the compiler that built it, the two outputs are byte-identical — the
# canonical bootstrap fixpoint (GCC: `cmp stage2 stage3`). A difference means
# the self-host's codegen depends on WHICH compiler built it: a latent codegen
# bug that the suites — which only ever run one compiler — cannot see.
#
# From 0.16 stage 3 is the same `fly build` as stage 2 (build_stage2.sh), for
# the compiler target only, run by the stage-2 fly into build/stage3:
#
#   build/stage2/bin/fly build --profile dist[-debug] --out-dir build/stage3 --target fly --keep-objects
#
# build/stage3/lib starts as a copy of build/stage2/lib, so the compiler links
# and reads headers exactly as at stage 2. What is compared: the compiler's
# OBJECT, bin/fly.o (stage 2 keeps its own in build/stage2/obj), the output of
# the compile before the link — the compiler's own product; the linked exe adds
# what the linker records (on Mach-O the path of that object). The profile's
# prefix map writes the package root, the compiling fly's lib/ and the output
# directory the same way at both stages, so the two can be compared at all.
#
# stage3 does NOT touch build/stage2 (that is what ships). On a mismatch it
# builds ONCE MORE with the same stage2 compiler to tell apart the two causes:
# stage3a != stage3b means the compiler is not deterministic (no verdict — fix
# that first), stage3a == stage3b != stage2 means a genuine fixpoint failure.
#
# Exit codes: 0 fixpoint holds, 1 fixpoint broken or stage3 failed to build,
#             2 compiler non-deterministic (no verdict).
# FLY_DEBUG_SYMBOLS must be the same as when stage2 was built.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."
. ci/linux/platform.sh

FLY2=build/stage2/bin/fly
OBJ2=build/stage2/obj/fly.o
for need in "$FLY2" "$OBJ2"; do
    [ -f "$need" ] || { echo "error: '$need' missing — run ci/linux/stage2.sh first." >&2; exit 1; }
done
PROFILE=dist; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && PROFILE=dist-debug

# compile_fly DIR — the compiler target, built by the stage-2 fly into DIR.
compile_fly() {
    local dir="$1"
    rm -rf "$dir"
    mkdir -p "$dir"
    cp -R build/stage2/lib "$dir/lib"
    "$FLY2" build --profile "$PROFILE" --out-dir "$dir" --target fly --keep-objects > "$dir/build.log" 2>&1 || true
    if [ ! -f "$dir/bin/fly.o" ]; then
        echo "error: stage3 compiler not built; see $dir/build.log:" >&2
        grep -m5 -E 'error|broken|abort' "$dir/build.log" | sed 's/^/      /' >&2 || true
        exit 1
    fi
}

sha() { sha256_of "$1"; }
size() { wc -c < "$1" | tr -d ' '; }

echo "stage3: compiling the compiler with the stage2 fly (fixpoint check, profile $PROFILE) ..."
start=$SECONDS
compile_fly build/stage3
echo "stage3: built in $((SECONDS - start))s"
OBJ3=build/stage3/bin/fly.o
h2="$(sha "$OBJ2")"; h3="$(sha "$OBJ3")"
echo "  stage2 fly.o  $h2  ($(size "$OBJ2") bytes)"
echo "  stage3 fly.o  $h3  ($(size "$OBJ3") bytes)"
if [ "$h2" = "$h3" ]; then
    echo "stage3: OK — fixpoint holds (stage2 == stage3, byte-identical object)."
    exit 0
fi

echo "stage3: objects differ — building once more to classify ..."
compile_fly build/stage3_repeat
OBJ3B=build/stage3_repeat/bin/fly.o
h3b="$(sha "$OBJ3B")"
echo "  stage3' fly.o $h3b  ($(size "$OBJ3B") bytes)"
if [ "$h3" != "$h3b" ]; then
    echo "stage3: INCONCLUSIVE — the stage2 compiler is not deterministic (two runs on the same input differ)."
    echo "        make it deterministic first; compare build/stage3/bin/fly.o and build/stage3_repeat/bin/fly.o."
    exit 2
fi
echo "stage3: FAILED — the fixpoint is broken: the stage2 compiler generates different code than the"
echo "        stage1 compiler that built it, from the same sources."
echo "        $(cmp "$OBJ2" "$OBJ3" 2>&1 | head -1) (sizes $(size "$OBJ2") vs $(size "$OBJ3"))."
echo "        inspect: nm / objdump -d on build/stage2/obj/fly.o vs build/stage3/bin/fly.o"
exit 1
