# -----------------------------------------------------------------------------
# test_leaks.ps1 - memory-leak guard for the self-host codegen.
#
# Every program under ci/leak_probes/<name>/ loops millions of times over one
# ownership pattern (a string call result bound to a local, a callee local left
# through `return`, a call result passed as an argument, locals left through
# break/continue). Each is compiled by the compiler under test and run while its
# peak working set is sampled; a probe that goes over the ceiling FAILS.
#
# Why a ceiling works: one leaked buffer per iteration costs tens of MB over the
# run (the two leaks this guards against measured 60-120 MB), while a clean
# probe stays at the process baseline (~4 MB). The ceiling sits well between.
# The suites cannot see this: a leak changes no value, it only grows the heap.
#
# The probes are compiled by build/stage$STAGE/bin/fly.exe (default STAGE=2,
# the compiler that ships and that compiles user programs). The seed is not
# involved: these are self-host codegen properties.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$env:STAGE = $STAGE
$FLY = if ($env:FLY) { $env:FLY } else { "build/stage$STAGE/bin/fly.exe" }
if (-not (Test-Path $FLY -PathType Leaf)) { Write-Host "error: compiler '$FLY' not found."; exit 1 }
$FLY = (Resolve-Path $FLY).Path

$CEILING_MB = 20
$OUT = 'build/test/leaks'
New-Item -ItemType Directory -Force $OUT | Out-Null

$failed = 0
foreach ($probe in Get-ChildItem 'ci/leak_probes' -Directory | Sort-Object Name) {
    $name = $probe.Name
    $dir = Join-Path $OUT $name
    New-Item -ItemType Directory -Force $dir | Out-Null
    $exe = Join-Path $dir "$name.exe"
    Remove-Item -ErrorAction SilentlyContinue $exe

    & $FLY @FLY_TARGET_ARGS --src-dir $probe.FullName -o "$name.exe" --out-dir $dir *> "$dir/compile.log"
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $exe)) {
        Write-Host "  FAIL  $name (compile)"; Get-Content "$dir/compile.log" | Select-Object -Last 10
        $failed++; continue
    }

    # Sample the peak working set until exit. PeakWorkingSet64 is itself a
    # running maximum, so only the last few ms before exit can be missed —
    # negligible against a leak that grows linearly over the whole run.
    $p = Start-Process -FilePath $exe -PassThru -NoNewWindow
    $peak = 0
    while (-not $p.HasExited) {
        try { $p.Refresh(); if ($p.PeakWorkingSet64 -gt $peak) { $peak = $p.PeakWorkingSet64 } } catch {}
        Start-Sleep -Milliseconds 10
    }
    $p.WaitForExit()
    $mb = [math]::Round($peak / 1MB, 1)
    if ($p.ExitCode -ne 0) {
        Write-Host "  FAIL  $name (exit $($p.ExitCode), peak $mb MB)"; $failed++
    } elseif ($mb -gt $CEILING_MB) {
        Write-Host "  FAIL  $name peak $mb MB > $CEILING_MB MB - memory leaked"; $failed++
    } else {
        Write-Host "  ok    $name peak $mb MB"
    }
}

if ($failed -gt 0) { Write-Host "test_leaks: $failed probe(s) failed"; exit 1 }
Write-Host 'test_leaks: all probes under the ceiling'
exit 0
