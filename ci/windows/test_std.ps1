# -----------------------------------------------------------------------------
# test_std.ps1 - run every std/test/**/*Suite.fly (Windows). PowerShell port of
# test_std.sh. These are `suite`/`case` programs exercising the standard library,
# grouped in subdirectories:
#   data/  fly.data.* containers     core/  fly.str, fly.math, fly.mem, fly.bridge
#   os/    fly.os.*                  lang/  casts, enums, generics, interfaces,
#   meta/  fly.meta schema                  override/super/deep inheritance
# Each is compiled against the std archive via -L and driven with --suite: fly
# builds the suite executable AND runs it, exiting with the run's code. The
# per-case report (`    <case> ... FAIL(<code>): <msg>`, then the
# `suite <Name>: N cases, ...` summary) lands in the captured log, so a failure
# names the exact assertion.
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

# -- Stage plumbing: WHICH compiler runs the tests. ----------------------------
# STAGE=N runs the suites with build\stageN's own compiler — each stage tests the
# compiler it just produced, so every step of the bootstrap is covered:
#   STAGE=0  the pinned REFERENCE seed that stage0 downloaded, with its bundled
#            runtime/std. A failure here is a SOURCE-level problem.
#   STAGE=1  the self-host stage1 just built WITH the reference. A failure here
#            that passed at 0 is the self-host's own codegen.
#   STAGE=2  the self-host stage2 just built WITH the self-host — the shipped
#            fixpoint artifact. A failure here that passed at 1 is stage2's codegen.
# So a suite that passes at N and fails at N+1 indicts the compiler stage N+1 built.
# Default 2 = the artifact that ships; use STAGE=1 for meaningful compiler-suite
# results while the stage2 binary is still miscompiled. $env:FLY overrides all.
$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$STAGE_PREV = "stage$STAGE"
$STAGE_FLY  = "build/$STAGE_PREV/bin/fly.exe"
$FLY = if ($env:FLY) { $env:FLY } else { $STAGE_FLY }

# Both stages link the SAME fork LLVM, so a difference between the runs is never
# the LLVM underneath. stage0.ps1 exports LIB through $GITHUB_ENV in CI; a local
# shell that did not dot-source it still needs the dir on LIB, or lld-link fails
# with "could not open 'LLVM-20.lib'".
$llvmLib = Join-Path (Get-Location) 'build\llvm\lib'
if (Test-Path $llvmLib) {
    if ($env:LIB -notlike "*$llvmLib*") { $env:LIB = "$llvmLib;$env:LIB" }
}
# Also put the LLVM bin dir on PATH: the self-host fly.exe loads LLVM-C.dll (and
# its siblings) at RUNTIME from there, and the fork ld.lld lives there too. Without
# it a fresh shell gets 0xC0000135 (DLL not found) or "linker not found".
$llvmBin = Join-Path (Get-Location) 'build\llvm\bin'
if (Test-Path $llvmBin) {
    if ($env:PATH -notlike "*$llvmBin*") { $env:PATH = "$llvmBin;$env:PATH" }
}
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
    Write-Host "error: FLY='$FLY' is not an executable file - run ci\windows\$STAGE_PREV.ps1 first (or set `$env:FLY)."
    exit 1
}
$FLY = (Resolve-Path $FLY).Path

$pass = 0
$fail = 0
$tests = Get-ChildItem -Recurse -Filter *Suite.fly std/test | Sort-Object FullName
foreach ($t in $tests) {
    $name = $t.BaseName
    $log = "$OUT/_std_$name.log"

    # One-shot: --suite compiles AND runs; fly's exit code is the run's code (or
    # the compile failure). DIRECTORY CLI (every stage): the suite is discovered
    # by name from the source root — `std`, not std/test, so the fly.meta SOURCE
    # under std/lib/meta stays pullable (suite names also repeat across trees:
    # ManifestSuite exists in compiler/test too).
    & $FLY --suite=$name --src-dir std -o "std_$name" --out-dir $OUT -L $STD *> $log
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  PASS          $name"
        $pass++
        continue
    }

    # A `suite <Name>` line in the log means the compile succeeded and the run
    # got to the report: show the FAIL(<code>): <msg> cases and the summary
    # (log tail on a report-less crash). No report at all = the compile broke.
    if (Select-String -Path $log -Pattern '^suite ' -Quiet) {
        Write-Host "  RUN  FAIL     $name (exit $LASTEXITCODE)"
        $hits = Select-String -Path $log -Pattern 'FAIL\(|^suite .*:' | Select-Object -First 6
        if ($hits) { $hits | ForEach-Object { "      $($_.Line)" } }
        else { Get-Content -Tail 5 $log | ForEach-Object { "      $_" } }
    } else {
        Write-Host "  COMPILE FAIL  $name (exit $LASTEXITCODE)"
        $hits = Select-String -Path $log -Pattern 'error:|broken|abort' | Select-Object -First 3
        if ($hits) { $hits | ForEach-Object { "      $($_.Line)" } }
        else { Get-Content -Tail 3 $log | ForEach-Object { "      $_" } }
    }
    $fail++
}

Write-Host ([string]::new([char]0x2500, 45))
Write-Host "  $pass passed, $fail failed"
if ($fail -eq 0) { exit 0 } else { exit 1 }
