# -----------------------------------------------------------------------------
# test_driver.ps1 - run every driver/test/**/*Suite.fly (the driver + package
# manager unit suites: CLI parsing, Manifest/toml, lockfile, semver/MVS resolver,
# registry, ToolChain, cache/checksum/json). PowerShell port of test_driver.sh.
#
# Single-file build: each suite is the entry; source discovery is implicit (the
# import graph pulls fly.driver.* AND fly.compiler.* source into one module while
# std namespaces stay archive-linked via the -L pass). `--test` builds in test
# mode; the executable is then run.
#
# Scope: ONLY driver/test. Compiler, std and runtime suites run in their own
# scripts, mirroring Linux.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Continue'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# The driver pulls fly.compiler.* → codegen, so these test executables import
# LLVM-C.dll (the LLVM C API). They run from $OUT, not next to the DLL, so put the
# LLVM bin dir on PATH for the run step — else they abort at startup with
# STATUS_DLL_NOT_FOUND (0xC0000135). build/llvm/bin is populated by stage0.
$llvmBin = Join-Path (Get-Location) 'build\llvm\bin'
if (Test-Path (Join-Path $llvmBin 'LLVM-C.dll')) { $env:PATH = "$llvmBin;$env:PATH" }

$OUT = "build/test"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

# CodeGen/target paths (pulled transitively via fly.compiler.*) may emit IR to
# /tmp/cg => <drive>:\tmp\cg on Windows. Create it up front.
New-Item -ItemType Directory -Force "$($PWD.Drive.Root)tmp\cg" | Out-Null

# Compiler under test: $FLY (default: the stage2 self-host fly - the artifact
# that ships; its --test system was ported from the reference). Resolved to an
# absolute path for the argv[0]-based stdlib lookup.
$FLY = if ($env:FLY) { $env:FLY } else { "build/stage2/bin/fly.exe" }
if ($FLY -notmatch '[\\/]') {
    $resolved = Get-Command $FLY -CommandType Application -ErrorAction SilentlyContinue |
                Select-Object -First 1
    if (-not $resolved) {
        Write-Host "error: compiler '$FLY' not found on PATH."
        exit 1
    }
    $FLY = $resolved.Source
}
if (-not (Test-Path $FLY -PathType Leaf)) {
    Write-Host "error: FLY='$FLY' is not an executable file - run ci\windows\stage2.ps1 first (or set `$env:FLY)."
    exit 1
}
$FLY = (Resolve-Path $FLY).Path

$pass = 0
$fail = 0
$suites = Get-ChildItem -Recurse -Filter *Suite.fly driver/test | Sort-Object FullName
foreach ($suite in $suites) {
    $name = $suite.BaseName
    $bin = "$OUT/test_$name.exe"
    $log = "$OUT/_$name.log"
    $run = "$OUT/_$name.run"

    & $FLY $suite.FullName --test -o "test_$name" --out-dir $OUT -L $STD *> $log
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  COMPILE FAIL  $name (exit $LASTEXITCODE)"
        $hits = Select-String -Path $log -Pattern 'error:|broken|abort' | Select-Object -First 3
        if ($hits) { $hits | ForEach-Object { "      $($_.Line)" } }
        else { Get-Content -Tail 3 $log | ForEach-Object { "      $_" } }
        $fail++
        continue
    }

    & $bin *> $run
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  PASS          $name"
        $pass++
    } else {
        Write-Host "  RUN  FAIL     $name (exit $LASTEXITCODE)"
        Get-Content -Tail 5 $run | ForEach-Object { "      $_" }
        $fail++
    }
}

Write-Host ([string]::new([char]0x2500, 45))
Write-Host "  $pass passed, $fail failed"
if ($fail -eq 0) { exit 0 } else { exit 1 }
