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

$OUT = "build/test"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

# CodeGen/target paths (pulled transitively via fly.compiler.*) may emit IR to
# /tmp/cg => <drive>:\tmp\cg on Windows. Create it up front.
New-Item -ItemType Directory -Force "$($PWD.Drive.Root)tmp\cg" | Out-Null

$FLY = if ($env:FLY) { $env:FLY } else { "fly" }
if ($FLY -notmatch '[\\/]') {
    $resolved = Get-Command $FLY -CommandType Application -ErrorAction SilentlyContinue |
                Select-Object -First 1
    if (-not $resolved) {
        Write-Host "error: bootstrap compiler '$FLY' not found on PATH."
        Write-Host "       Set FLY to the bootstrap compiler, e.g.:"
        Write-Host "       `$env:FLY = 'C:\path\to\fly\build\bin\fly.exe'"
        exit 1
    }
    $FLY = $resolved.Source
}
if (-not (Test-Path $FLY -PathType Leaf)) {
    Write-Host "error: FLY='$FLY' is not an executable file."
    exit 1
}

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
