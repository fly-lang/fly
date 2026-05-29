#!/bin/bash
# End-to-end test: compile a Fly program with --debug and verify that
# llvm-dwarfdump reports valid DWARF info (compile unit, variables, line info).
#
# Supports Linux and macOS.
#
# Usage:
#   bash runtime/test/debug_dwarf_test.sh [fly-binary] [fly-std-lib] [llvm-dwarfdump]
#
# CMake passes all three paths automatically via add_test().

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FLY_BIN="${1:-$SCRIPT_DIR/../../cmake-build-relwithdebinfo/bin/fly}"
FLY_STD="${2:-$SCRIPT_DIR/../../cmake-build-relwithdebinfo/lib/fly_std_lib.a}"
DWARFDUMP="${3:-}"

# ── Prerequisites ──────────────────────────────────────────────────────────────
if [[ ! -x "$FLY_BIN" ]]; then
    echo "SKIP: fly binary not found at $FLY_BIN"
    exit 0
fi

# Resolve llvm-dwarfdump: prefer the path passed by CMake, then search PATH.
if [[ -z "$DWARFDUMP" || "$DWARFDUMP" == *"-NOTFOUND" ]]; then
    for candidate in llvm-dwarfdump llvm-dwarfdump-20 llvm-dwarfdump-19 llvm-dwarfdump-18; do
        if command -v "$candidate" &>/dev/null; then
            DWARFDUMP="$(command -v "$candidate")"
            break
        fi
    done
fi

if [[ -z "$DWARFDUMP" ]]; then
    echo "SKIP: llvm-dwarfdump not found in PATH"
    exit 0
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# ── Fly source ─────────────────────────────────────────────────────────────────
cat > "$WORK/dbg_test.fly" << 'FLY'
main() {
    int a = 10
    int b = 20
    int c = a + b
}
FLY

# ── Compile with debug symbols ─────────────────────────────────────────────────
# Redirect stderr: --debug also enables DebugLog which produces massive compiler output.
"$FLY_BIN" --debug "$WORK/dbg_test.fly" "$FLY_STD" -o "$WORK/dbg_test" 2>/dev/null
echo "Compile: OK"

# ── DWARF content assertions ───────────────────────────────────────────────────
"$DWARFDUMP" --debug-info "$WORK/dbg_test" > "$WORK/dwarf_info.txt" 2>&1
"$DWARFDUMP" --debug-line "$WORK/dbg_test" > "$WORK/dwarf_line.txt" 2>&1

PASS=1

grep -q "DW_TAG_compile_unit" "$WORK/dwarf_info.txt" || {
    echo "FAIL: no DW_TAG_compile_unit"
    PASS=0
}

grep -q "DW_TAG_subprogram" "$WORK/dwarf_info.txt" || {
    echo "FAIL: no DW_TAG_subprogram (function debug info missing)"
    PASS=0
}

grep -q "DW_TAG_variable" "$WORK/dwarf_info.txt" || {
    echo "FAIL: no DW_TAG_variable (local variable debug info missing)"
    PASS=0
}

grep -q "Line table" "$WORK/dwarf_line.txt" || {
    echo "FAIL: no line table in .debug_line"
    PASS=0
}

if [ "$PASS" -eq 1 ]; then
    echo "PASS: DWARF debug info test succeeded"
    exit 0
else
    exit 1
fi
