# -----------------------------------------------------------------------------
# test_runtime.ps1 - run every runtime/test/*Suite.fly (Windows). PowerShell port
# of test_runtime.sh. These are `suite`/`case` programs (compiled with --test)
# exercising the Fly runtime (fly.runtime + the fly.os wrappers). Each is compiled
# against the std + runtime archives via -L; the executable must exit 0.
#
# The runtime is platform-specific (fly.runtime.* links the HOST runtime), so this
# runs ONLY the host suite: it SKIPS foreign-platform suites (RuntimeLinux* /
# RuntimeMacos*) so RuntimeWindowsSuite is what runs on Windows.
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
$tests = @(Get-ChildItem -Recurse -Filter *Suite.fly runtime/test -ErrorAction SilentlyContinue | Sort-Object FullName)
foreach ($t in $tests) {
    $name = $t.BaseName
    # Skip foreign-platform suites: their osname/arch assertions target another OS
    # and would fail against the Windows runtime linked here.
    if ($name -match 'Linux|Macos') { continue }
    $found++
    $bin = "$OUT/rt_$name.exe"
    $log = "$OUT/_rt_$name.log"
    $run = "$OUT/_rt_$name.run"

    & $FLY $t.FullName --test -o "rt_$name" --out-dir $OUT -L $STD *> $log
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
    Write-Host "  no runtime/test/*Suite.fly found for this platform"
}
Write-Host "  $pass passed, $fail failed"
if ($fail -eq 0) { exit 0 } else { exit 1 }
