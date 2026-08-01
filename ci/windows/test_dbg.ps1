# -----------------------------------------------------------------------------
# test_dbg.ps1 - smoke-test the bundled debugger (lldb.exe, shipped under its
# original LLVM name next to fly.exe by link_fly.ps1 when FLY_BUNDLE_LLVM=1).
#
# Three checks, all cheap and deterministic (no Python scripting needed — the
# bundled lldb is built self-contained, scripting OFF; batch mode drives the
# built-in command interpreter via -o):
#   1. lldb --version runs and reports the fork LLVM version — proves the
#      binary starts from the shipped layout (liblldb.dll import-by-name) with
#      zero external dependencies.
#   2. a breakpoint round-trip on a DWARF probe: the probe is compiled by the
#      SEED (build\stage0\bin\fly.exe) with --debug-symbols, because the
#      self-host codegen does not emit DWARF yet. Compile is -c (gnu codegen,
#      object) + link_bin.ps1 (fork ld.lld + mingw/UCRT — ld.lld keeps the
#      .debug_* sections by default), the same no-MSVC path every shipped
#      binary takes.
#   3. lldb-dap.exe (the IDE/DAP adapter, same liblldb) exists and answers
#      --help. A full DAP session needs a client and is not worth the CI flake.
#
# Skips (exit 0) when the debugger is not bundled and FLY_BUNDLE_LLVM != 1
# (plain dev build); with FLY_BUNDLE_LLVM=1 a missing lldb.exe is a FAILURE —
# the shipped artifact would be incomplete.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$env:STAGE = $STAGE                          # link_bin.ps1 derives its lib dir from it
$BIN = "build/stage$STAGE/bin"
$DBG = "$BIN/lldb.exe"
$SEED = 'build/stage0/bin/fly.exe'

if (-not (Test-Path $DBG -PathType Leaf)) {
    if ($env:FLY_BUNDLE_LLVM -eq '1') {
        Write-Host "error: $DBG missing but FLY_BUNDLE_LLVM=1 - the bundle is incomplete."; exit 1
    }
    Write-Host 'test_dbg: skipped (non-bundle build - no lldb next to fly.exe)'; exit 0
}
if (-not (Test-Path $SEED -PathType Leaf)) {
    Write-Host "error: seed compiler '$SEED' not found - run ci\windows\stage0.ps1 first."; exit 1
}

$OUT = 'build/test/dbg'
New-Item -ItemType Directory -Force $OUT | Out-Null

# -- 1. version -----------------------------------------------------------------
$ver = & $DBG --version 2>&1 | Out-String
if ($LASTEXITCODE -ne 0 -or $ver -notmatch 'lldb version \d+\.\d+\.\d+') {
    Write-Host "  FAIL  lldb --version (exit $LASTEXITCODE): $ver"; exit 1
}
Write-Host "  ok    $(($ver.Trim() -split "`n")[0])"

# -- 2. breakpoint round-trip on a seed-built DWARF probe -----------------------
@'
int add(const int a, const int b) {
    int s = a + b
    out = s
}

void main() {
    int x = 10
    int y = 32
    int r = add(x, y)
}
'@ | Set-Content "$OUT/dbg_probe.fly" -Encoding UTF8

& $SEED --debug-symbols -c @FLY_TARGET_ARGS --src-dir $OUT -o dbg_probe --out-dir $OUT *> "$OUT/compile.log"
if ($LASTEXITCODE -ne 0) {
    Write-Host '  FAIL  probe compile (seed, --debug-symbols)'; Get-Content "$OUT/compile.log" | Select-Object -Last 10; exit 1
}
# The emitted object name varies by driver version - normalize.
$obj = @("$OUT/dbg_probe", "$OUT/dbg_probe.o", "$OUT/dbg_probe.fly.o", "$OUT/dbg_probe.fly.obj") |
       Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $obj) { Write-Host "  FAIL  probe object not found under $OUT"; exit 1 }
& "$PSScriptRoot\link_bin.ps1" -Obj $obj -Out "$OUT/dbg_probe.exe"
if ($LASTEXITCODE -ne 0) { Write-Host '  FAIL  probe link'; exit 1 }

$bp = & $DBG -b -o 'breakpoint set -f dbg_probe.fly -l 2' -o 'run' -o 'frame info' -o 'continue' `
             -- "$OUT/dbg_probe.exe" 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) { Write-Host "  FAIL  lldb batch run (exit $LASTEXITCODE)"; Write-Host $bp; exit 1 }
if ($bp -notmatch 'stop reason = breakpoint') {
    Write-Host '  FAIL  breakpoint did not hit (no "stop reason = breakpoint")'; Write-Host $bp; exit 1
}
if ($bp -notmatch 'exited with status = 0') {
    Write-Host '  FAIL  probe did not exit cleanly under lldb'; Write-Host $bp; exit 1
}
Write-Host '  ok    breakpoint at dbg_probe.fly:2 hit, probe resumed and exited 0'

# -- 3. lldb-dap presence -------------------------------------------------------
$DAP = "$BIN/lldb-dap.exe"
if (-not (Test-Path $DAP -PathType Leaf)) { Write-Host "  FAIL  $DAP missing"; exit 1 }
& $DAP --help *> $null
if ($LASTEXITCODE -ne 0) { Write-Host "  FAIL  lldb-dap --help (exit $LASTEXITCODE)"; exit 1 }
Write-Host '  ok    lldb-dap present and answers --help'

Write-Host 'test_dbg: all checks passed'
exit 0
