#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# test_dbg.sh - smoke-test the bundled debugger (lldb, shipped under its
# original LLVM name next to fly when FLY_BUNDLE_LLVM=1: the llvm-*-bin copy target).
# Mirror of ci/windows/test_dbg.ps1.
#
# Three checks, cheap and deterministic (no Python — the bundled lldb is built
# self-contained, scripting OFF; batch mode drives the built-in interpreter):
#   1. lldb --version from the shipped layout (liblldb.so under lib/, RUNPATH
#      $ORIGIN/../lib) with zero external dependencies.
#   2. breakpoint round-trip on a DWARF probe compiled and linked by THIS
#      stage's fly with --debug-symbols: the breakpoint must resolve to
#      file:line, the caller frame must carry its line, the parameters and the
#      local must show their values, and `next` must reach the following line.
#      On Linux this also exercises the lldb-server launch path
#      (local processes are spawned through it) and lldb-argdumper.
#   3. lldb-dap (IDE/DAP adapter) exists and answers --help.
#
# Which debugger is under test:
#   * bundle build (lldb next to fly) - the SHIPPED layout. With
#     FLY_BUNDLE_LLVM=1 a missing lldb is a FAILURE: the artifact would be
#     incomplete. This is what CI runs.
#   * plain dev build - the debugger stage0 provisions into build/llvm/bin
#     (RUNPATH $ORIGIN/../lib finds liblldb in build/llvm/lib), the very files
#     the bundle copies. This used to SKIP, which meant a dev build never
#     exercised the DWARF round-trip at all. Only the
#     shipped LAYOUT is left untested here, and only CI can test it.
# Skips (exit 0) only when neither exists (stage0 was never run).
# -----------------------------------------------------------------------------
set -u
cd "$(dirname "$0")/../.."

STAGE="${STAGE:-2}"
# BIN is the directory holding the debugger under test - lldb AND lldb-dap
# (check 3) - so the two always come from the same layout.
BIN="build/stage$STAGE/bin"
DBG="$BIN/lldb"
FLY="build/stage$STAGE/bin/fly"

if [ ! -x "$DBG" ]; then
    if [ "${FLY_BUNDLE_LLVM:-0}" = "1" ]; then
        echo "error: $DBG missing but FLY_BUNDLE_LLVM=1 - the bundle is incomplete."; exit 1
    fi
    LLVM_BIN="build/llvm/bin"
    if [ ! -x "$LLVM_BIN/lldb" ]; then
        echo "test_dbg: skipped (no lldb next to fly nor in $LLVM_BIN - run ci/linux/stage0.sh)"; exit 0
    fi
    BIN="$LLVM_BIN"
    DBG="$BIN/lldb"
    echo "test_dbg: non-bundle build - testing the provisioned debugger in $BIN (the shipped layout is checked with FLY_BUNDLE_LLVM=1)"
fi
[ -x "$FLY" ] || { echo "error: '$FLY' not found - build stage $STAGE first."; exit 1; }

OUT="build/test/dbg"
mkdir -p "$OUT"

# -- 1. version ----------------------------------------------------------------
ver="$("$DBG" --version 2>&1)" || { echo "  FAIL  lldb --version: $ver"; exit 1; }
echo "$ver" | grep -qE 'lldb version [0-9]+\.[0-9]+\.[0-9]+' \
    || { echo "  FAIL  unexpected lldb --version output: $ver"; exit 1; }
echo "  ok    $(echo "$ver" | head -1)"

# -- 2. breakpoint round-trip on a self-host DWARF probe ----------------------
cat > "$OUT/dbg_probe.fly" <<'EOF'
int add(const int a, const int b) {
    int s = a + b
    return s
}

void main() {
    int x = 10
    int y = 32
    int r = add(x, y)
    Box bx = new Box()
    bx.v = r
    int z = bx.v
}

struct Pt {
    int x
    int y
}

class Box {
    public int v
    public Pt p

    public Box() {
        this.v = 0
        this.p.x = 3
        this.p.y = 4
    }
}
EOF

# Compiled AND linked by this stage's own fly — the self-host emits the DWARF
# (phase 1.6) and links it the way a user program is linked.
rm -f "$OUT/dbg_probe"
if ! "$FLY" --debug-symbols --src-dir "$OUT" -o dbg_probe --out-dir "$OUT" > "$OUT/compile.log" 2>&1; then
    echo "  FAIL  probe compile ($FLY --debug-symbols)"; tail -10 "$OUT/compile.log"; exit 1
fi
[ -x "$OUT/dbg_probe" ] || { echo "  FAIL  probe executable not produced"; exit 1; }

bp="$("$DBG" -b -o 'breakpoint set -f dbg_probe.fly -l 2' -o 'run' -o 'bt 2' -o 'frame variable a b' -o 'next' -o 'frame variable s' -o 'breakpoint set -f dbg_probe.fly -l 12' -o 'continue' -o 'frame variable *bx' -o 'continue' \
      -- "$OUT/dbg_probe" 2>&1)"
rc=$?
if [ "$rc" -ne 0 ]; then echo "  FAIL  lldb batch run (exit $rc)"; echo "$bp"; exit 1; fi
# Each expectation is one thing the debug info must get right.
expect() {
    echo "$bp" | grep -qE "$1" || { echo "  FAIL  $2 (no match for /$1/)"; echo "$bp"; exit 1; }
}
expect 'Breakpoint 1: where = .*add.* at dbg_probe\.fly:2' 'breakpoint resolved to file:line'
expect 'stop reason = breakpoint'                           'breakpoint hit'
expect 'frame #1: .*main at dbg_probe\.fly:9'               'caller frame has its line'
expect '\(int\) a = 10'                                     'parameter a has its value'
expect '\(int\) b = 32'                                     'parameter b has its value'
expect 'stop reason = step over'                            'next stopped'
expect 'add\(.*\) at dbg_probe\.fly:3'                      'next steps to the following line'
expect '\(int\) s = 42'                                     'the local holds a + b after the step'
expect 'v = 42'                                             'a class field has its value'
expect 'p = \(x = 3, y = 4\)'                               'an inline struct field has its members'
expect 'exited with status = 0'                             'probe exited cleanly'
echo "  ok    self-host DWARF: breakpoint at dbg_probe.fly:2, caller at :9, a=10 b=32, step to :3 with s=42, Box fields v=42 p=(3,4), exit 0"

# -- 3. lldb-dap presence ------------------------------------------------------
DAP="$BIN/lldb-dap"
[ -x "$DAP" ] || { echo "  FAIL  $DAP missing"; exit 1; }
"$DAP" --help > /dev/null 2>&1 || { echo "  FAIL  lldb-dap --help (exit $?)"; exit 1; }
echo "  ok    lldb-dap present and answers --help"

echo "test_dbg: all checks passed"
exit 0
