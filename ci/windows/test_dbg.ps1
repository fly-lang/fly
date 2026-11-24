# -----------------------------------------------------------------------------
# test_dbg.ps1 - smoke-test the bundled debugger (lldb.exe, shipped under its
# original LLVM name next to fly.exe when FLY_BUNDLE_LLVM=1: the copy target llvm-windows).
#
# Three checks, all cheap and deterministic (no Python scripting needed — the
# bundled lldb is built self-contained, scripting OFF; batch mode drives the
# built-in command interpreter via -o):
#   1. lldb --version runs and reports the fork LLVM version — proves the
#      binary starts from the shipped layout (liblldb.dll import-by-name) with
#      zero external dependencies.
#   2. a breakpoint round-trip on a DWARF probe compiled and linked by THIS
#      stage's fly with --debug-symbols: the breakpoint must resolve to
#      file:line, the caller frame must carry its line, the parameters and the
#      local must show their values, and `next` must reach the following line.
#      ld.lld keeps the .debug_* sections by default.
#   3. lldb-dap.exe (the IDE/DAP adapter, same liblldb) exists and answers
#      --help. A full DAP session needs a client and is not worth the CI flake.
#
# Which debugger is under test:
#   * bundle build (lldb.exe next to fly.exe) - the SHIPPED layout. With
#     FLY_BUNDLE_LLVM=1 a missing lldb.exe is a FAILURE: the artifact would be
#     incomplete. This is what CI runs.
#   * plain dev build - the debugger stage0 provisions into build\llvm\bin, the
#     very files the bundle copies. This used to SKIP, which meant a dev
#     build never exercised the DWARF round-trip at all.
#     Only the shipped LAYOUT is left untested here, and only CI can test it.
# Skips (exit 0) only when neither exists (stage0 was never run).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
# $BIN is the directory holding the debugger under test - lldb.exe AND
# lldb-dap.exe (check 3) - so the two always come from the same layout.
$BIN = "build/stage$STAGE/bin"
$DBG = "$BIN/lldb.exe"
$FLY = "build/stage$STAGE/bin/fly.exe"

if (-not (Test-Path $DBG -PathType Leaf)) {
    if ($env:FLY_BUNDLE_LLVM -eq '1') {
        Write-Host "error: $DBG missing but FLY_BUNDLE_LLVM=1 - the bundle is incomplete."; exit 1
    }
    $llvmBin = 'build/llvm/bin'
    if (-not (Test-Path "$llvmBin/lldb.exe" -PathType Leaf)) {
        Write-Host "test_dbg: skipped (no lldb next to fly.exe nor in $llvmBin - run ci\windows\stage0.ps1)"; exit 0
    }
    $BIN = $llvmBin
    $DBG = "$BIN/lldb.exe"
    Write-Host "test_dbg: non-bundle build - testing the provisioned debugger in $BIN (the shipped layout is checked with FLY_BUNDLE_LLVM=1)"
}
if (-not (Test-Path $FLY -PathType Leaf)) {
    Write-Host "error: '$FLY' not found - build stage $STAGE first."; exit 1
}

$OUT = 'build/test/dbg'
New-Item -ItemType Directory -Force $OUT | Out-Null

# -- 1. version -----------------------------------------------------------------
$ver = & $DBG --version 2>&1 | Out-String
if ($LASTEXITCODE -ne 0 -or $ver -notmatch 'lldb version \d+\.\d+\.\d+') {
    Write-Host "  FAIL  lldb --version (exit $LASTEXITCODE): $ver"; exit 1
}
Write-Host "  ok    $(($ver.Trim() -split "`n")[0])"

# -- 2. breakpoint round-trip on a self-host DWARF probe -----------------------
@'
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
'@ | Set-Content "$OUT/dbg_probe.fly" -Encoding UTF8

# Compiled AND linked by this stage's own fly — the self-host emits the DWARF
# (phase 1.6) and links it the way a user program is linked.
Remove-Item "$OUT/dbg_probe.exe" -Force -ErrorAction SilentlyContinue
& $FLY --debug-symbols --src-dir $OUT -o dbg_probe --out-dir $OUT *> "$OUT/compile.log"
if ($LASTEXITCODE -ne 0 -or -not (Test-Path "$OUT/dbg_probe.exe")) {
    Write-Host "  FAIL  probe compile ($FLY --debug-symbols)"; Get-Content "$OUT/compile.log" | Select-Object -Last 10; exit 1
}

$bp = & $DBG -b -o 'breakpoint set -f dbg_probe.fly -l 2' -o 'run' -o 'bt 2' -o 'frame variable a b' -o 'next' -o 'frame variable s' -o 'breakpoint set -f dbg_probe.fly -l 12' -o 'continue' -o 'frame variable *bx' -o 'continue' `
             -- "$OUT/dbg_probe.exe" 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) { Write-Host "  FAIL  lldb batch run (exit $LASTEXITCODE)"; Write-Host $bp; exit 1 }
# Each expectation is one thing the debug info must get right.
$expect = [ordered]@{
    'Breakpoint 1: where = .*add.* at dbg_probe\.fly:2'   = 'breakpoint resolved to file:line'
    'stop reason = breakpoint'                            = 'breakpoint hit'
    'frame #1: .*main at dbg_probe\.fly:9'                = 'caller frame has its line'
    '\(int\) a = 10[\s\S]*\(int\) b = 32'                = 'parameters have their values'
    'stop reason = step over[\s\S]*add\(.*\) at dbg_probe\.fly:3' = 'next steps to the following line'
    '\(int\) s = 42'                                      = 'the local holds a + b after the step'
    '\(Box\) \*bx = \{[\s\S]*v = 42[\s\S]*p = \(x = 3, y = 4\)' = 'class fields, an inline struct among them'
    'exited with status = 0'                              = 'probe exited cleanly'
}
foreach ($re in $expect.Keys) {
    if ($bp -notmatch $re) { Write-Host "  FAIL  $($expect[$re]) (no match for /$re/)"; Write-Host $bp; exit 1 }
}
Write-Host '  ok    self-host DWARF: breakpoint at dbg_probe.fly:2, caller at :9, a=10 b=32, step to :3 with s=42, Box fields v=42 p=(3,4), exit 0'

# -- 3. lldb-dap presence -------------------------------------------------------
$DAP = "$BIN/lldb-dap.exe"
if (-not (Test-Path $DAP -PathType Leaf)) { Write-Host "  FAIL  $DAP missing"; exit 1 }
& $DAP --help *> $null
if ($LASTEXITCODE -ne 0) { Write-Host "  FAIL  lldb-dap --help (exit $LASTEXITCODE)"; exit 1 }
Write-Host '  ok    lldb-dap present and answers --help'

Write-Host 'test_dbg: all checks passed'
exit 0
