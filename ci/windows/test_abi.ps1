# -----------------------------------------------------------------------------
# test_abi.ps1 - seed <-> self-host calling-convention guard.
#
# The shipped std archive (build\stage2\lib\fly_std_lib.lib) is compiled by the
# pinned REFERENCE SEED, while user programs are compiled by the SELF-HOST that
# ships beside it. So every call a user program makes into the std crosses a
# compiler boundary, and the two compilers must lower arguments identically. The
# suites cannot see this: they compile both sides with the SAME compiler, so a
# convention both halves get wrong in the same way still passes everywhere.
#
# Each probe under ci/abi_probes/<name>/ is that boundary in miniature:
#   lib/   compiled by build\stage0 (the seed) with --lib -> archive + .fly.h
#   prog/  compiled by the compiler under test, linking that archive via -L
# prog/main.fly checks every value it gets back and `fail`s with a distinct code
# and message, so a mismatch names the clause of the convention that broke
# instead of just crashing.
#
# STAGE selects the self-host under test (default 2, the compiler that ships);
# the seed side is always build\stage0, which is what built the shipped std.
# Mirror: ci/linux/test_abi.sh.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$FLY = if ($env:FLY) { $env:FLY } else { "build/stage$STAGE/bin/fly.exe" }
if (-not (Test-Path $FLY -PathType Leaf)) { Write-Host "error: compiler '$FLY' not found."; exit 1 }
$FLY = (Resolve-Path $FLY).Path

# The seed: the compiler that built the std this stage links. Not overridable by
# $env:FLY - the whole point is that the two sides are DIFFERENT compilers.
$SEED = 'build/stage0/bin/fly.exe'
if (-not (Test-Path $SEED -PathType Leaf)) {
    Write-Host "error: seed compiler '$SEED' not found - run ci\windows\stage0.ps1 first."; exit 1
}
$SEED = (Resolve-Path $SEED).Path

# The self-host loads LLVM-C.dll at runtime and reaches ld.lld from build\llvm\bin.
$llvmBin = Join-Path (Get-Location) 'build\llvm\bin'
if (Test-Path $llvmBin) {
    if ($env:PATH -notlike "*$llvmBin*") { $env:PATH = "$llvmBin;$env:PATH" }
}

$OUT = 'build/test/abi'
New-Item -ItemType Directory -Force $OUT | Out-Null

$failed = 0
$passed = 0
foreach ($probe in Get-ChildItem 'ci/abi_probes' -Directory | Sort-Object Name) {
    $name = $probe.Name
    $dir = Join-Path $OUT $name
    $libDir = Join-Path $dir 'lib'
    Remove-Item $dir -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force $dir, $libDir | Out-Null

    # (1) the CALLEE side, built by the seed. --lib emits the archive next to one
    # .fly.h per public module, which is exactly what a -L package dir is.
    & $SEED --lib @FLY_TARGET_ARGS -o (Join-Path $libDir "abi_$name") `
            --src-dir (Join-Path $probe.FullName 'lib') *> "$dir/seed.log"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  FAIL  $name (seed --lib, exit $LASTEXITCODE)"
        Get-Content $dir/seed.log | Select-Object -Last 8 | ForEach-Object { "        $_" }
        $failed++; continue
    }
    $emitted = @(Get-ChildItem $libDir | Where-Object { $_.Extension -in '.a', '.lib' })
    $header = @(Get-ChildItem $libDir -Filter '*.fly.h')
    if ($emitted.Count -eq 0 -or $header.Count -eq 0) {
        Write-Host "  FAIL  $name (seed --lib produced no archive/header)"
        $failed++; continue
    }

    # (2) the CALLER side, built by the compiler under test, resolving the
    # imports through that .fly.h and linking that archive.
    $exe = Join-Path $dir "abi_$name.exe"
    & $FLY @FLY_TARGET_ARGS --src-dir (Join-Path $probe.FullName 'prog') `
           -o "abi_$name.exe" --out-dir $dir -L $libDir *> "$dir/compile.log"
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $exe)) {
        Write-Host "  FAIL  $name (self-host compile, exit $LASTEXITCODE)"
        Get-Content $dir/compile.log | Select-Object -Last 8 | ForEach-Object { "        $_" }
        $failed++; continue
    }

    # (3) run it. Exit 0 = every clause agreed; anything else is the `fail` code,
    # and the probe printed which check it was.
    & $exe *> "$dir/run.log"
    $rc = $LASTEXITCODE
    if ($rc -eq 0) {
        Write-Host "  ok    $name"
        $passed++
    } else {
        Write-Host "  FAIL  $name (exit $rc) - seed/self-host ABI mismatch"
        Get-Content $dir/run.log -ErrorAction SilentlyContinue | ForEach-Object { "        $_" }
        $failed++
    }
}

if ($passed -eq 0 -and $failed -eq 0) { Write-Host 'error: no probes under ci/abi_probes'; exit 1 }
if ($failed -gt 0) { Write-Host "test_abi: $failed probe(s) failed"; exit 1 }
Write-Host "test_abi: all checks passed ($passed probe(s) agree with the seed ABI)"
exit 0
