#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_tools.sh - run the UNIT suites of the tools (tools/*/test/*Suite.fly).
# Mirror of ci/windows/test_tools.ps1.
#
# The in-process counterpart of the end-to-end scripts: test_lsp.sh and
# test_registry.sh drive the BUILT binaries over stdio and sockets, while these
# suites call the libraries directly and reach the corners a wire test cannot.
#
# Optional SELECTOR ($1 or $FLY_TEST_SUITE): a suite name. Empty = all.
# -----------------------------------------------------------------------------
set -u
cd "$(dirname "$0")/../.."

SEL="${1:-${FLY_TEST_SUITE:-}}"
STAGE="${STAGE:-2}"
FLY="${FLY:-build/stage$STAGE/bin/fly}"
[ -x "$FLY" ] || { echo "error: compiler '$FLY' not found."; exit 1; }
FLY="$(cd "$(dirname "$FLY")" && pwd)/$(basename "$FLY")"

OUT="build/test"
STD="std/lib"
mkdir -p "$OUT"

# name:root:needs-compiler
SUITES="LspProtocolSuite:tools/lsp:1 LspTransportSuite:tools/lsp:1 LspAnalyzerSuite:tools/lsp:1 RegistryHandlerSuite:tools/registry:0"

passed=0
failed=0
for entry in $SUITES; do
    name="${entry%%:*}"
    rest="${entry#*:}"
    root="${rest%%:*}"
    needsc="${rest##*:}"
    [ -n "$SEL" ] && [ "$SEL" != "$name" ] && continue
    log="$OUT/tools_$name.log"
    if [ "$needsc" = "1" ]; then
        "$FLY" --suite="$name" --src-dir "$root" --src-dir compiler/lib \
               -o "test_$name" --out-dir "$OUT" -L "$STD" > "$log" 2>&1
    else
        "$FLY" --suite="$name" --src-dir "$root" \
               -o "test_$name" --out-dir "$OUT" -L "$STD" > "$log" 2>&1
    fi
    rc=$?
    line=$(grep "^suite $name:" "$log" | tail -1)
    if [ "$rc" -eq 0 ]; then
        echo "  PASS  $name  $line"
        passed=$((passed+1))
    else
        echo "  FAIL  $name  $line"
        grep -E 'FAIL|error:' "$log" | head -10 | sed 's/^/        /'
        failed=$((failed+1))
    fi
done

echo ""
echo "  $passed passed, $failed failed"
[ "$failed" -ne 0 ] && exit 1
exit 0
