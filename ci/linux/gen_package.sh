#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# gen_package.sh - generate build/gen/package.fly from the root Manifest.fly.
#
# The root Manifest.fly is the single source of truth for Fly's version (0.15.2).
# This writes the `fly.meta.package` module - name(), version(), description() -
# the same module `fly build` gives every executable it builds. The compiler
# reads its version from it (compiler/lib/basic/Version.fly: package.version()),
# so every compile of the compiler sources adds `--src-dir build/gen`: a plain
# source, which the seed compiles too. Once the tree is built with `fly build`
# (release/0.16.X) the Builder writes this module and the script goes away.
#
# The file is rewritten only when its content changes, so its timestamp - and
# the incremental fingerprints - stay put across builds. Prints the version.
# Mirrors ci/windows/gen_package.ps1 and the Builder (same bytes: LF line ends).
# The fields must not hold a double quote or a backslash: the seed reads
# literals raw.
# -----------------------------------------------------------------------------
set -euo pipefail
cd "$(dirname "$0")/../.."

field() {
    sed -nE "s/^[[:space:]]*string[[:space:]]+$1[[:space:]]*=[[:space:]]*\"([^\"]*)\".*/\\1/p" Manifest.fly | head -1
}
NAME="$(field name)"
VERSION="$(field version)"
DESC="$(field description)"
if ! printf '%s' "$VERSION" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$'; then
    echo 'error: Manifest.fly has no `string version = "x.y.z"` line.' >&2
    exit 1
fi
case "$NAME$DESC" in
    *\\*) echo 'error: a Manifest.fly name/description holds a backslash.' >&2; exit 1 ;;
esac

mkdir -p build/gen
OUT=build/gen/package.fly
TMP="$OUT.tmp"
{
    printf 'namespace fly.meta.package\n\n'
    printf '// GENERATED from Manifest.fly - do not edit.\n'
    printf 'public string name() {\n    return "%s"\n}\n' "$NAME"
    printf 'public string version() {\n    return "%s"\n}\n' "$VERSION"
    printf 'public string description() {\n    return "%s"\n}\n' "$DESC"
} > "$TMP"
if [ -f "$OUT" ] && cmp -s "$TMP" "$OUT"; then
    rm -f "$TMP"
else
    mv -f "$TMP" "$OUT"
fi
echo "$VERSION"
