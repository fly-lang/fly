# -----------------------------------------------------------------------------
# test_std.ps1 - run every std/test/*_test.fly (Windows). PowerShell port of
# test_std.sh. These are main()-style programs (not `suite`/`--test` blocks)
# exercising the standard library: fly.str, fly.math, fly.data.*, fly.os.*,
# fly.mem, generics, enums, inheritance/override/super. Each is a standalone
# program compiled against the std archive via -L; the executable must exit 0
# (fly.assert.* exit non-zero with the failing code).
#
# Scope: ONLY std/test. Compiler, driver and runtime suites run in their own
# scripts, mirroring Linux.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Continue'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$OUT = "build/test"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

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
$tests = Get-ChildItem -Recurse -Filter *_test.fly std/test | Sort-Object FullName
foreach ($t in $tests) {
    $name = $t.BaseName
    $bin = "$OUT/std_$name.exe"
    $log = "$OUT/_std_$name.log"
    $run = "$OUT/_std_$name.run"

    & $FLY $t.FullName -o "std_$name" --out-dir $OUT -L $STD *> $log
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
