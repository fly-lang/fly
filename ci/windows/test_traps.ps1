# -----------------------------------------------------------------------------
# test_traps.ps1 - run-time trap guard for the self-host compiler.
# Mirror of ci/linux/test_traps.sh.
#
# Some errors can only be caught while the program runs. Fly stops it there with
# a diagnostic - "error 134: <what> at <file>:<line>:<col>" and exit code 134 -
# instead of letting it crash or read garbage. The suites cannot see a trap that
# has gone missing: they run valid programs, so a compiler that stops emitting
# the check still passes every one of them.
#
# Each probe under ci/trap_probes\<name>\ is one trap in miniature:
#   main.fly     the program;
#   expect.txt   line 1 the exit code it must end with (`crash`: any non-zero
#                code, the system's own - a fault the language does not check,
#                reported by the crash handler); line 2 a fragment of the message
#                it must print; line 3 the position (file:line:col) that must
#                follow. A probe that must run normally expects 0 and empty lines.
#
# STAGE selects the compiler under test (default 2, the one that ships).
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

# The compiler loads LLVM-C.dll at run time and reaches ld.lld from build\llvm\bin.
$llvmBin = Join-Path (Get-Location) 'build\llvm\bin'
if ((Test-Path $llvmBin) -and ($env:PATH -notlike "*$llvmBin*")) { $env:PATH = "$llvmBin;$env:PATH" }

$OUT = 'build/test/traps'
New-Item -ItemType Directory -Force $OUT | Out-Null
$failed = 0
$count = 0

foreach ($probe in Get-ChildItem 'ci/trap_probes' -Directory | Sort-Object Name) {
    $name = $probe.Name
    $count++
    $lines = @(Get-Content (Join-Path $probe.FullName 'expect.txt'))
    $wantCrash = ($lines[0].Trim() -eq 'crash')
    $wantCode = if ($wantCrash) { 0 } else { [int]$lines[0] }
    $wantMsg = if ($lines.Count -gt 1) { $lines[1] } else { '' }
    $wantPos = if ($lines.Count -gt 2) { $lines[2] } else { '' }

    $dir = Join-Path $OUT $name
    Remove-Item $dir -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force $dir | Out-Null
    & $FLY @FLY_TARGET_ARGS --src-dir $probe.FullName -o "$name.exe" --out-dir $dir *> "$dir/compile.log"
    $exe = Join-Path $dir "$name.exe"
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $exe)) {
        Write-Host "  FAIL  $name - does not compile"
        Get-Content "$dir/compile.log" | Select-Object -Last 5 | ForEach-Object { Write-Host "        $_" }
        $failed++
        continue
    }

    & $exe *> "$dir/run.log"
    $rc = $LASTEXITCODE
    $text = Get-Content "$dir/run.log" -Raw
    if ($null -eq $text) { $text = '' }
    if (($wantCrash -and $rc -eq 0) -or (-not $wantCrash -and $rc -ne $wantCode)) {
        Write-Host "  FAIL  $name - exit $rc, expected $wantCode"
        Get-Content "$dir/run.log" | Select-Object -First 3 | ForEach-Object { Write-Host "        $_" }
        $failed++
    } elseif ($wantMsg -and -not $text.Contains($wantMsg)) {
        Write-Host "  FAIL  $name - message missing: $wantMsg"
        Get-Content "$dir/run.log" | Select-Object -First 3 | ForEach-Object { Write-Host "        $_" }
        $failed++
    } elseif ($wantPos -and -not $text.Contains($wantPos)) {
        Write-Host "  FAIL  $name - position missing: $wantPos"
        Get-Content "$dir/run.log" | Select-Object -First 3 | ForEach-Object { Write-Host "        $_" }
        $failed++
    } else {
        Write-Host "  ok    $name"
    }
}

if ($count -eq 0) { Write-Host 'error: no probes under ci/trap_probes'; exit 1 }
if ($failed -gt 0) { Write-Host "test_traps: $failed of $count probe(s) failed"; exit 1 }
Write-Host "test_traps: all checks passed ($count probes)"
exit 0
