#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_dbg.sh - smoke-test the bundled debugger (lldb, shipped under its
# original LLVM name next to fly by link_fly.sh when FLY_BUNDLE_LLVM=1).
# Mirror of ci/windows/test_dbg.ps1.
#
# Three checks, cheap and deterministic (no Python — the bundled lldb is built
# self-contained, scripting OFF; batch mode drives the built-in interpreter):
#   1. lldb --version from the shipped layout (liblldb.so under lib/, RUNPATH
#      $ORIGIN/../lib) with zero external dependencies.
#   2. breakpoint round-trip on a DWARF probe compiled by the SEED
#      (build/stage0/bin/fly --debug-symbols) — the self-host codegen emits no
#      DWARF yet. On Linux this also exercises the lldb-server launch path
#      (local processes are spawned through it) and lldb-argdumper.
#   3. lldb-dap (IDE/DAP adapter) exists and answers --help.
#
# Skips (exit 0) when the debugger is not bundled and FLY_BUNDLE_LLVM != 1;
# with FLY_BUNDLE_LLVM=1 a missing lldb is a FAILURE (incomplete artifact).
# -----------------------------------------------------------------------------
set -u
cd "$(dirname "$0")/../.."

STAGE="${STAGE:-2}"
BIN="build/stage$STAGE/bin"
DBG="$BIN/lldb"
SEED="build/stage0/bin/fly"

if [ ! -x "$DBG" ]; then
    if [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
        echo "error: $DBG missing but FLY_BUNDLE_LLVM=1 - the bundle is incomplete."; exit 1
    fi
    echo "test_dbg: skipped (non-bundle build - no lldb next to fly)"; exit 0
fi
[ -x "$SEED" ] || { echo "error: seed compiler '$SEED' not found - run ci/linux/stage0.sh first."; exit 1; }

OUT="build/test/dbg"
mkdir -p "$OUT"

# -- 1. version ----------------------------------------------------------------
ver="$("$DBG" --version 2>&1)" || { echo "  FAIL  lldb --version: $ver"; exit 1; }
echo "$ver" | grep -qE 'lldb version [0-9]+\.[0-9]+\.[0-9]+' \
    || { echo "  FAIL  unexpected lldb --version output: $ver"; exit 1; }
echo "  ok    $(echo "$ver" | head -1)"

# -- 2. breakpoint round-trip on a seed-built DWARF probe ----------------------
cat > "$OUT/dbg_probe.fly" <<'EOF'
int add(const int a, const int b) {
    int s = a + b
    out = s
}

void main() {
    int x = 10
    int y = 32
    int r = add(x, y)
}
EOF

if ! "$SEED" --debug-symbols --src-dir "$OUT" -o dbg_probe --out-dir "$OUT" > "$OUT/compile.log" 2>&1; then
    echo "  FAIL  probe compile (seed, --debug-symbols)"; tail -10 "$OUT/compile.log"; exit 1
fi
[ -x "$OUT/dbg_probe" ] || { echo "  FAIL  probe executable not produced"; exit 1; }

bp="$("$DBG" -b -o 'breakpoint set -f dbg_probe.fly -l 2' -o 'run' -o 'frame info' -o 'continue' \
      -- "$OUT/dbg_probe" 2>&1)"
rc=$?
if [ "$rc" -ne 0 ]; then echo "  FAIL  lldb batch run (exit $rc)"; echo "$bp"; exit 1; fi
echo "$bp" | grep -q 'stop reason = breakpoint' \
    || { echo '  FAIL  breakpoint did not hit (no "stop reason = breakpoint")'; echo "$bp"; exit 1; }
echo "$bp" | grep -q 'exited with status = 0' \
    || { echo '  FAIL  probe did not exit cleanly under lldb'; echo "$bp"; exit 1; }
echo "  ok    breakpoint at dbg_probe.fly:2 hit, probe resumed and exited 0"

# -- 3. lldb-dap presence ------------------------------------------------------
DAP="$BIN/lldb-dap"
[ -x "$DAP" ] || { echo "  FAIL  $DAP missing"; exit 1; }
"$DAP" --help > /dev/null 2>&1 || { echo "  FAIL  lldb-dap --help (exit $?)"; exit 1; }
echo "  ok    lldb-dap present and answers --help"

echo "test_dbg: all checks passed"
exit 0
