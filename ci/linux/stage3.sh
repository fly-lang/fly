#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# stage3.sh — fixpoint check of the staged bootstrap (see the stage map in
# stage1.sh; Windows mirror: ci/windows/stage3.ps1). CI runs it on every build.
#
#   stage1  built by the SEED (C++ reference)          → self-host #1
#   stage2  built by stage1 (self-host #1)             → the SHIPPED compiler
#   stage3  built by stage2 (the shipped compiler)     → THIS SCRIPT
#
# stage2 and stage3 are compiled from the SAME sources; they differ only in the
# compiler that compiled them. If the shipped compiler generates the same code
# as the compiler that built it, the two outputs are byte-identical — the
# canonical bootstrap fixpoint (GCC: `cmp stage2 stage3`). A difference means
# the self-host's codegen depends on WHICH compiler built it: a latent codegen
# bug the suites — which only ever run one compiler — cannot see.
#
# What is compared: the merged driver+compiler OBJECT (build/stage<N>/driver/
# Driver.o), the output of `fly -c` before the link — the compiler's own
# deterministic product, free of linker-chosen layout.
#
# stage3 does NOT touch build/stage2 (that is what ships): everything goes to
# build/stage3. It needs a finished stage 2 (stage2.sh) and costs one more
# compile of the compiler - seconds. It runs as its own CI step after stage 2,
# blocking, on every push and release (it used to be release-only, and the
# first release found a seed defect every push had carried: ASTBuilder.CreateParam).
#
# On a mismatch it recompiles ONCE MORE with the same stage2 compiler to tell
# apart the two causes: stage3 != stage3' means the compiler is not
# deterministic (no verdict — fix that first); stage3 == stage3' != stage2 means
# a genuine fixpoint failure.
#
# Exit codes: 0 fixpoint holds, 1 fixpoint broken or stage3 failed to build,
#             2 compiler non-deterministic (no verdict).
# FLY_DEBUG_SYMBOLS must match the value stage2 was built with.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail
cd "$(dirname "$0")/../.."

FLY2=build/stage2/bin/fly
OBJ2=build/stage2/driver/Driver.o
for need in "$FLY2" "$OBJ2"; do
    [ -f "$need" ] || { echo "error: '$need' missing — run ci/linux/stage2.sh first." >&2; exit 1; }
done
DBG=""; [ "${FLY_DEBUG_SYMBOLS:-0}" = "1" ] && DBG="--debug-symbols"
# Same mapping as build_compiler.sh: the stage2 fly reads its headers from
# build/stage2/lib, the one that built stage 2 from build/stage1/lib — both
# become "lib" in the DWARF, or the objects would differ by that path alone.
[ -n "$DBG" ] && DBG="$DBG --debug-prefix-map=$(cd build/stage2/lib && pwd)=lib"

# Compile the compiler with the STAGE-2 fly into $1; print the object path.
# Same invocation as build_compiler.sh STAGE=2 (self-host: -c, no link).
build_driver() {
    local dir="$1"
    rm -rf "$dir"; mkdir -p "$dir"
    "$FLY2" --src-dir compiler $DBG -c -o Driver --out-dir "$dir" > "$dir/emit.log" 2>&1 || true
    if [ ! -f "$dir/Driver" ]; then
        echo "error: stage3 driver object not emitted; see $dir/emit.log:" >&2
        grep -m5 -E 'error:|broken|abort' "$dir/emit.log" | sed 's/^/      /' >&2 || true
        exit 1
    fi
    mv -f "$dir/Driver" "$dir/Driver.o"
}
sha() { sha256sum "$1" | cut -d' ' -f1; }
size() { stat -c %s "$1"; }

echo "stage3: compiling driver + compiler with the stage2 fly (fixpoint check) ...${DBG:+ (+debug-symbols)}"
start=$SECONDS
build_driver build/stage3/driver
echo "stage3: built in $((SECONDS - start))s"
OBJ3=build/stage3/driver/Driver.o

h2="$(sha "$OBJ2")"; h3="$(sha "$OBJ3")"
echo "  stage2 Driver.o  $h2  ($(size "$OBJ2") bytes)"
echo "  stage3 Driver.o  $h3  ($(size "$OBJ3") bytes)"
if [ "$h2" = "$h3" ]; then
    echo "stage3: OK — fixpoint holds (stage2 == stage3, byte-identical object)."
    exit 0
fi

echo "stage3: objects differ — recompiling once more to classify ..."
build_driver build/stage3/driver_repeat
OBJ3B=build/stage3/driver_repeat/Driver.o
h3b="$(sha "$OBJ3B")"
echo "  stage3' Driver.o $h3b  ($(size "$OBJ3B") bytes)"
if [ "$h3" != "$h3b" ]; then
    echo "stage3: INCONCLUSIVE — the stage2 compiler is not deterministic (two runs on the same input differ)."
    echo "        make it deterministic first; compare build/stage3/driver and build/stage3/driver_repeat."
    exit 2
fi
echo "stage3: FAILED — the fixpoint is broken: the stage2 compiler generates different code than the"
echo "        stage1 compiler that built it, from the same sources."
echo "        $(cmp "$OBJ2" "$OBJ3" 2>&1 | head -1) (sizes $(size "$OBJ2") vs $(size "$OBJ3"))."
echo "        inspect: nm / objdump -d on build/stage2/driver/Driver.o vs build/stage3/driver/Driver.o"
exit 1
