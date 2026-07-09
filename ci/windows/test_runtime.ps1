# -----------------------------------------------------------------------------
# test_runtime.ps1 - run every runtime/test/*_test.fly (Windows). PowerShell port
# of test_runtime.sh. These are main()-style programs exercising the Fly runtime
# (fly.runtime + the fly.os wrappers). Each is compiled against the std + runtime
# archives via -L; the executable must exit 0.
#
# runtime/test is currently empty (the runtime is exercised indirectly by the
# std/os suites); this script runs 0 tests today but picks up any *_test.fly
# added under runtime/test. Scope: ONLY runtime/test, mirroring Linux.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Continue'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$OUT = "build/test"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

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
$found = 0
$tests = @(Get-ChildItem -Recurse -Filter *_test.fly runtime/test -ErrorAction SilentlyContinue | Sort-Object FullName)
foreach ($t in $tests) {
    $found++
    $name = $t.BaseName
    $bin = "$OUT/rt_$name.exe"
    $log = "$OUT/_rt_$name.log"
    $run = "$OUT/_rt_$name.run"

    & $FLY $t.FullName -o "rt_$name" --out-dir $OUT -L $STD *> $log
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
if ($found -eq 0) {
    Write-Host "  no runtime/test/*_test.fly found (runtime exercised via std/os suites)"
}
Write-Host "  $pass passed, $fail failed"
if ($fail -eq 0) { exit 0 } else { exit 1 }
