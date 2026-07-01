# -----------------------------------------------------------------------------
# test_compiler.ps1 - run every test/**/*Suite.fly against the compiler sources,
# without flyp (Windows). PowerShell port of test_compiler.sh.
#
# Single-file build: each suite is the entry, and `--src-dir compiler` resolves
# the whole `fly.compiler` dependency graph from its imports into one module.
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

Set-Location $PSScriptRoot

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

# Bootstrap compiler: $FLY (default `fly`). CI passes an absolute path - see the
# note in build_compiler.ps1 (executable-path stdlib discovery / bare-name trap).
$FLY = if ($env:FLY) { $env:FLY } else { "fly" }

$pass = 0
$fail = 0
$suites = Get-ChildItem -Recurse -Filter *Suite.fly test | Sort-Object FullName
foreach ($suite in $suites) {
    $name = $suite.BaseName
    $bin = "$OUT/test_$name.exe"
    $log = "$OUT/_$name.log"
    $run = "$OUT/_$name.run"

    & $FLY $suite.FullName --test --src-dir compiler -o "test_$name" --out-dir $OUT -L $STD *> $log
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

Write-Host "---------------------------------------------"
Write-Host "  $pass passed, $fail failed"
exit $(if ($fail -eq 0) { 0 } else { 1 })
