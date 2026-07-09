# -----------------------------------------------------------------------------
# test_compiler.ps1 - run every compiler/test/**/*Suite.fly (Windows).
# PowerShell port of test_compiler.sh.
#
# Single-file build: each suite is the entry; source discovery is implicit (a fly
# project compiles from the CURRENT directory - the repo root here), pulling only
# what the import graph references (std stays archive-linked via the -L pass).
# `--test` builds in test mode; `--out-dir` sends the executable and its
# intermediate objects into $OUT; the resulting executable is then run.
#
# Scope: ONLY compiler/test. Driver, std and runtime suites run in their own
# scripts (test_driver.ps1 / test_std.ps1 / test_runtime.ps1), mirroring Linux.
# -----------------------------------------------------------------------------

# Keep $LASTEXITCODE the sole arbiter of pass/fail (like bash `if ! "$FLY"`):
# don't let a native command's stderr or non-zero exit raise a terminating error
# that would abort the loop before the summary.
$ErrorActionPreference = 'Continue'
$PSNativeCommandUseErrorActionPreference = $false

# Scripts live in ci\windows\; operate from the project root (two levels up).
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# Scratch for per-suite test binaries/logs; under build/ but separate from
# the stage dirs so it doesn't sit next to the release artifact.
$OUT = "build/test"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

# CodeGen/target suites emit IR/objects to /tmp/cg and read them back. On native
# Windows a leading-`/` path resolves to the current drive root, so
# /tmp/cg => <drive>:\tmp\cg. Create it up front.
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
$suites = Get-ChildItem -Recurse -Filter *Suite.fly compiler/test | Sort-Object FullName
foreach ($suite in $suites) {
    $name = $suite.BaseName
    $bin = "$OUT/test_$name.exe"
    $log = "$OUT/_$name.log"
    $run = "$OUT/_$name.run"

    & $FLY $suite.FullName --test -o "test_$name" --out-dir $OUT -L $STD *> $log
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  COMPILE FAIL  $name (exit $LASTEXITCODE)"
        # Match real diagnostics (`error:`), not the substring "error" inside
        # warnings like 'errorHandler'. If nothing matches, show the log tail.
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
