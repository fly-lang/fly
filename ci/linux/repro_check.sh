#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# repro_check.sh - the shipped artifacts are REPRODUCIBLE (plan item 1.4).
# Linux mirror of ci/windows/repro_check.ps1.
#
# Rebuilds stage 2 from scratch with the same stage-1 compiler and compares every
# file of build/stage2/{bin,lib} byte for byte with the first build. Two clean
# builds from the same sources must give the same bits: a difference means the
# output depends on something other than the sources (a timestamp, an address or
# hash order leaking into codegen, a path of the build machine). Stage 3 only
# checks the compiler OBJECT; this covers what the release ships.
#
# Run after stage2.sh. The first build is set aside in build/stage2_ref, the
# second one takes its place; on success the reference is removed, on a mismatch
# both are kept and the differing files are listed. Only the BUILD steps of stage
# 2 run again. Exit 0 = reproducible.
# -----------------------------------------------------------------------------
set -euo pipefail
cd "$(dirname "$0")/../.."
export STAGE=2

if [ ! -x build/stage2/bin/fly ]; then
    echo "error: no stage 2 to compare - run ci/linux/stage2.sh first."
    exit 1
fi

ref=build/stage2_ref
rm -rf "$ref"
mv build/stage2 "$ref"
# The same build as stage2.sh (build_stage2.sh: one `fly build`).
echo "repro: rebuilding stage 2 from scratch ..."
if ! ./ci/linux/build_stage2.sh > build/repro_build_stage2.log 2>&1; then
    echo "error: repro build_stage2.sh failed (see build/repro_build_stage2.log)"
    exit 1
fi

# path<TAB>sha256 for every file of bin/ and lib/, sorted by path
tree() {
    (cd "$1" && find bin lib -type f -print0 2>/dev/null | sort -z | xargs -0 sha256sum | awk '{ print $2 "\t" $1 }')
}

a=$(mktemp)
b=$(mktemp)
tree "$ref" > "$a"
tree build/stage2 > "$b"
if ! diff_out=$(diff "$a" "$b"); then
    total=$(wc -l < "$a")
    echo "repro: NOT reproducible (of $total files):"
    echo "$diff_out" | grep '^[<>]' | sed 's/^</  first build: /; s/^>/  second build:/' | cut -f1
    echo "repro: first build kept in $ref, second in build/stage2."
    rm -f "$a" "$b"
    exit 1
fi
total=$(wc -l < "$a")
rm -f "$a" "$b"
rm -rf "$ref"
echo "repro: $total files of build/stage2/{bin,lib} identical across two clean builds."
