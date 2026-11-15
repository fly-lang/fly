# -----------------------------------------------------------------------------
# test_sema.ps1 - semantic-check guard for the self-host compiler.
# Mirror of ci/linux/test_sema.sh.
#
# The suites only ever compile VALID programs, so they cannot see a check that
# has gone missing: a compiler that stops rejecting `int n = aLong` still passes
# every one of them. That is how the self-host came to accept programs the seed
# refuses (narrowing, private access, abstract instantiation, ...) while the
# whole pipeline stayed green - the tree itself was protected only because the
# SEED type-checks it at stage 1.
#
# Each probe under ci/sema_probes is one rule in miniature:
#   reject\<name>\main.fly    must NOT compile, and
#   reject\<name>\expect.txt  holds a fragment of the diagnostic it must get.
#   accept\<name>\main.fly    must compile, link and exit 0 - the edge cases a
#                             check must keep allowing (a literal that fits, a
#                             shadowing declaration, an overload that differs).
#
# A reject probe fails three ways, and they are told apart on purpose:
#   ACCEPTED   the compiler took the program - the check is gone;
#   INTERNAL   it died in the backend verifier instead of diagnosing - the
#              program reached codegen, which is the same hole with a crash on top;
#   WRONG MSG  it was rejected, but not for the reason the probe is about.
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

$OUT = 'build/test/sema'
New-Item -ItemType Directory -Force $OUT | Out-Null
$failed = 0
$count = 0

foreach ($probe in Get-ChildItem 'ci/sema_probes/reject' -Directory | Sort-Object Name) {
    $name = $probe.Name
    $count++
    $expect = (Get-Content (Join-Path $probe.FullName 'expect.txt') -Raw).Trim()
    $log = Join-Path $OUT "reject_$name.log"
    & $FLY @FLY_TARGET_ARGS --src-dir $probe.FullName --no-output *> $log
    $rc = $LASTEXITCODE
    $text = Get-Content $log -Raw
    if ($null -eq $text) { $text = '' }
    if ($text.Contains('backend module verification failed')) {
        Write-Host "  FAIL  reject/$name - INTERNAL error instead of a diagnostic"; $failed++
    } elseif ($rc -eq 0) {
        Write-Host "  FAIL  reject/$name - ACCEPTED (expected: $expect)"; $failed++
    } elseif (-not $text.Contains($expect)) {
        Write-Host "  FAIL  reject/$name - WRONG MSG (expected: $expect)"
        Get-Content $log | Select-Object -First 3 | ForEach-Object { Write-Host "        $_" }
        $failed++
    } else {
        Write-Host "  ok    reject/$name"
    }
}

foreach ($probe in Get-ChildItem 'ci/sema_probes/accept' -Directory | Sort-Object Name) {
    $name = $probe.Name
    $count++
    $dir = Join-Path $OUT "accept_$name"
    New-Item -ItemType Directory -Force $dir | Out-Null
    $exe = Join-Path $dir "$name.exe"
    Remove-Item -ErrorAction SilentlyContinue $exe
    & $FLY @FLY_TARGET_ARGS --src-dir $probe.FullName -o "$name.exe" --out-dir $dir *> "$dir/compile.log"
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $exe)) {
        Write-Host "  FAIL  accept/$name - REJECTED"
        Get-Content "$dir/compile.log" | Select-Object -First 5 | ForEach-Object { Write-Host "        $_" }
        $failed++; continue
    }
    & $exe *> "$dir/run.log"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  FAIL  accept/$name - exit $LASTEXITCODE"; $failed++
    } else {
        Write-Host "  ok    accept/$name"
    }
}

if ($failed -gt 0) { Write-Host "test_sema: $failed of $count probe(s) failed"; exit 1 }
Write-Host "test_sema: all $count probes behave as specified"
exit 0
