#!/usr/bin/env bash
# make_fixtures.sh — regenerate the archives TarGzSuite reads.
#
# Run from anywhere on a box with GNU tar; the fixtures land next to this script.
# They are checked in because the suite must agree with what the REST OF THE
# WORLD produces, not with something this repo wrote — but they have to be
# reproducible, hence --sort=name, a fixed --mtime and numeric owner: rerunning
# this must yield byte-identical files, or the checked-in copies would churn.
#
# Two archives, because GNU tar has two ways to carry a path longer than the
# 100-byte `name` field and they are DIFFERENT code paths in the reader:
#   pkg.tar.gz   — default GNU format → a 'L' long-name RECORD
#   ustar.tar.gz — --format=ustar     → split across `prefix` (155 @345) and `name`
#
# If you change the contents, update the expected digests in
# std/test/core/TarGzSuite.fly: this script prints them at the end.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"

DEEP="pkg/very/deeply/nested/directory/structure/that/goes/past/the/one/hundred/byte/limit/of/ustar"
TARFLAGS="--sort=name --mtime=2026-01-01 00:00:00Z --owner=0 --group=0 --numeric-owner"

# ── pkg.tar.gz — the main fixture ────────────────────────────────────────────
W=$(mktemp -d)
mkdir -p "$W/pkg/src" "$W/$DEEP"
printf 'hello from fly\nsecond line\n'                      > "$W/pkg/README.md"
# CRLF and a 0x1A: the two bytes a text-mode round trip destroys on Windows.
printf 'void main() {\r\n    print("hi")\r\n}\r\n\032tail\n' > "$W/pkg/src/main.fly"
# 3000 deterministic bytes spanning 0..255 — every value ≥ 128 is covered, which
# is where a sign-extended byte read breaks the Huffman decode on Linux only.
perl -e 'for my $i (0..2999) { print chr(($i*37+11) % 256) }' > "$W/pkg/src/blob.bin"
printf 'deep\n'                                             > "$W/$DEEP/leaf.txt"
: > "$W/pkg/empty"
( cd "$W" && tar --sort=name --mtime='2026-01-01 00:00:00Z' --owner=0 --group=0 \
      --numeric-owner -czf "$HERE/pkg.tar.gz" pkg )

# ── ustar.tar.gz — the same long path, prefix-split instead of a 'L' record ───
U=$(mktemp -d)
mkdir -p "$U/$DEEP"
printf 'hello from fly\nsecond line\n' > "$U/pkg/README.md"
printf 'deep\n'                        > "$U/$DEEP/leaf.txt"
( cd "$U" && tar --format=ustar --sort=name --mtime='2026-01-01 00:00:00Z' --owner=0 \
      --group=0 --numeric-owner -czf "$HERE/ustar.tar.gz" pkg )

echo "wrote $HERE/pkg.tar.gz ($(wc -c < "$HERE/pkg.tar.gz") bytes)"
echo "wrote $HERE/ustar.tar.gz ($(wc -c < "$HERE/ustar.tar.gz") bytes)"
echo
echo "expected digests for TarGzSuite (sha256 of the files BEFORE archiving):"
for f in pkg/README.md pkg/src/main.fly pkg/src/blob.bin pkg/empty "$DEEP/leaf.txt"; do
    printf '  %s  %s\n' "$(sha256sum "$W/$f" | cut -d' ' -f1)" "$f"
done
rm -rf "$W" "$U"
