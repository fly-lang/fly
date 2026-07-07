# -----------------------------------------------------------------------------
# test_tools_flyp.ps1 - run every tools/flyp/test/*Suite.fly, the unit tests of
# the flyp package manager (Fly port). PowerShell port of test_tools_flyp.sh.
#
# Each suite is the entry; `--src-dir tools/flyp` pulls the flyp modules into one
# module while std namespaces stay archive-linked (the -L pass). `--test` builds
# in test mode; the resulting executable is then run. Kept separate from
# test_compiler.ps1 and wired as its own workflow step.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
# Under `shell: pwsh` CI runners, pwsh 7.4 enables this preference, making a
# native non-zero exit throw before our own $LASTEXITCODE check. Disable it so
# the explicit check below is the sole arbiter (harmless no-op on PS 5.1).
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$OUT = "build/test"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

# Bootstrap compiler: $FLY (default `fly`), resolved to an absolute path for the
# argv[0]-based stdlib lookup.
$FLY = if ($env:FLY) { $env:FLY } else { "fly" }
if ($FLY -notmatch '[\\/]') {
    $resolved = Get-Command $FLY -CommandType Application -ErrorAction SilentlyContinue |
                Select-Object -First 1
    if (-not $resolved) {
        Write-Host "error: bootstrap compiler '$FLY' not found on PATH."
        Write-Host "       `$env:FLY = 'C:\path\to\fly\build\bin\fly.exe'"
        exit 1
    }
    $FLY = $resolved.Source
}
if (-not (Test-Path $FLY -PathType Leaf)) {
    Write-Host "error: FLY='$FLY' is not an executable file."
    exit 1
}
if (-not (Test-Path (Join-Path (Split-Path $FLY -Parent) '..\lib') -PathType Container)) {
    Write-Host "error: no lib\ directory next to '$FLY' (expected <exe_dir>\..\lib"
    Write-Host "       with llvm.fly.h, runtime.fly.h, fly_runtime_lib.lib)."
    Write-Host "       Point FLY at a built bootstrap compiler, e.g. fly\build\bin\fly.exe."
    exit 1
}

$pass = 0
$fail = 0
$suites = Get-ChildItem -Recurse -Filter *Suite.fly tools/flyp/test -ErrorAction SilentlyContinue | Sort-Object FullName
foreach ($suite in $suites) {
    $name = $suite.BaseName
    $bin = "$OUT/flyp_$name.exe"
    $log = "$OUT/_flyp_$name.log"
    $run = "$OUT/_flyp_$name.run"

    & $FLY $suite.FullName --test --src-dir tools/flyp -o "flyp_$name" --out-dir $OUT -L $STD *> $log
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  COMPILE FAIL  flyp/$name (exit $LASTEXITCODE)"
        $hits = Select-String -Path $log -Pattern 'error:|broken|abort' | Select-Object -First 3
        if ($hits) { $hits | ForEach-Object { "      $($_.Line)" } }
        else { Get-Content -Tail 3 $log | ForEach-Object { "      $_" } }
        $fail++
        continue
    }

    & $bin *> $run
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  PASS          flyp/$name"
        $pass++
    } else {
        Write-Host "  RUN  FAIL     flyp/$name (exit $LASTEXITCODE)"
        Get-Content -Tail 5 $run | ForEach-Object { "      $_" }
        $fail++
    }
}

Write-Host "---------------------------------------------"
Write-Host "  $pass passed, $fail failed"
if ($fail -ne 0) { exit 1 }
