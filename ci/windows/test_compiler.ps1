# -----------------------------------------------------------------------------
# test_compiler.ps1 - run every test/**/*Suite.fly against the compiler sources,
# without flyp (Windows). PowerShell port of test_compiler.sh.
#
# Single-file build: each suite is the entry; source discovery is implicit (a fly
# project compiles from the CURRENT directory - the repo root here), pulling only
# what the import graph references (std stays archive-linked via the -L pass).
# `--test` builds in test mode; `--out-dir` sends the executable and its
# intermediate objects into $OUT; the resulting executable is then run.
# No file list, no concatenation.
# -----------------------------------------------------------------------------

# Keep $LASTEXITCODE the sole arbiter of pass/fail (like bash `if ! "$FLY"`):
# don't let a native command's stderr or non-zero exit raise a terminating error
# that would abort the loop before the summary. CI runs this under `shell: pwsh`
# where $ErrorActionPreference='Stop' and pwsh 7.4 enables
# $PSNativeCommandUseErrorActionPreference; both are neutralised here. (On PS 5.1
# the native-command variable is just an unused local.)
$ErrorActionPreference = 'Continue'
$PSNativeCommandUseErrorActionPreference = $false

# Scripts live in ci\windows\; operate from the project root (two levels up).
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# Scratch for per-suite test binaries/logs; under build/ but separate from
# build/bin so it doesn't sit next to the release artifact.
$OUT = "build/test"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

# CodeGen/target suites emit IR/objects to /tmp/cg and read them back (e.g.
# cgm.emitIR("/tmp/cg/run.ll")). On native Windows a leading-`/` path resolves to
# the current drive root, so /tmp/cg => <drive>:\tmp\cg. Create it up front -
# otherwise the LLVM file open fails and LLVMPrintModuleToFile/EmitToFile crash on
# the error path.
New-Item -ItemType Directory -Force "$($PWD.Drive.Root)tmp\cg" | Out-Null

# Bootstrap compiler: $FLY (default `fly`). The compiler derives its stdlib dir
# from its own executable path (argv[0]), and a bare name breaks that lookup -
# so a path-less $FLY is resolved through PATH into an absolute path here.
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
if (-not (Test-Path (Join-Path (Split-Path $FLY -Parent) '..\lib') -PathType Container)) {
    Write-Host "error: no lib\ directory next to '$FLY' (expected <exe_dir>\..\lib"
    Write-Host "       with llvm.fly.h, runtime.fly.h, fly_runtime_lib.lib)."
    Write-Host "       Point FLY at a built bootstrap compiler, e.g. fly\build\bin\fly.exe."
    exit 1
}

$pass = 0
$fail = 0
$suites = Get-ChildItem -Recurse -Filter *Suite.fly test | Sort-Object FullName
foreach ($suite in $suites) {
    $name = $suite.BaseName
    $bin = "$OUT/test_$name.exe"
    $log = "$OUT/_$name.log"
    $run = "$OUT/_$name.run"

    & $FLY $suite.FullName --test -o "test_$name" --out-dir $OUT -L $STD *> $log
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  COMPILE FAIL  $name (exit $LASTEXITCODE)"
        # Match real diagnostics (`error:`), not the substring "error" inside
        # warnings like 'errorHandler'/'SemaError'. If nothing matches (e.g. a
        # silent crash with a 0xC.. exit), fall back to the log tail for context.
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

# -- std library tests (std/test/*_test.fly, main()-style; mirrors fly/std/test) --
$stdTests = Get-ChildItem -Recurse -Filter *_test.fly std/test | Sort-Object FullName
foreach ($t in $stdTests) {
    $name = $t.BaseName
    $bin = "$OUT/std_$name.exe"
    $log = "$OUT/_std_$name.log"
    $run = "$OUT/_std_$name.run"

    & $FLY $t.FullName -o "std_$name" --out-dir $OUT -L $STD *> $log
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  COMPILE FAIL  std/$name (exit $LASTEXITCODE)"
        Get-Content -Tail 3 $log | ForEach-Object { "      $_" }
        $fail++
        continue
    }

    & $bin *> $run
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  PASS          std/$name"
        $pass++
    } else {
        Write-Host "  RUN  FAIL     std/$name (exit $LASTEXITCODE)"
        Get-Content -Tail 5 $run | ForEach-Object { "      $_" }
        $fail++
    }
}

Write-Host "---------------------------------------------"
Write-Host "  $pass passed, $fail failed"
exit $(if ($fail -eq 0) { 0 } else { 1 })
