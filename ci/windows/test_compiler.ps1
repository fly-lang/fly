# -----------------------------------------------------------------------------
# test_compiler.ps1 - run every compiler/test/**/*Suite.fly (Windows).
# PowerShell port of test_compiler.sh.
#
# Single-file build: each suite is the entry; source discovery is implicit (a fly
# project compiles from the CURRENT directory - the repo root here), pulling only
# what the import graph references (std stays archive-linked via the -L pass).
# `--suite` builds the suite executable in test mode AND runs it in one shot —
# fly exits with the run's code and the per-case FAIL(<code>): <msg> report
# lands in the captured log; `--out-dir` sends the executable and its
# intermediate objects into $OUT.
#
# Scope: compiler/test - which now INCLUDES the driver + package-manager suites
# under compiler/test/driver (the driver is compiled INTO the compiler
# monolithically, so there is no separate test_driver step). std and runtime
# suites run in their own scripts (test_std.ps1 / test_runtime.ps1).
# -----------------------------------------------------------------------------

# Optional SELECTOR (first argument, or $env:FLY_TEST_SUITE): a suite name, a
# qualified ns.SuiteName, or a NAMESPACE — forwarded to the namespace-aware
# `--suite=<sel>` (a namespace runs every suite under it). Empty = every suite.
param([string]$Suite = '')

# Keep $LASTEXITCODE the sole arbiter of pass/fail (like bash `if ! "$FLY"`):
# don't let a native command's stderr or non-zero exit raise a terminating error
# that would abort the loop before the summary.
$ErrorActionPreference = 'Continue'
$PSNativeCommandUseErrorActionPreference = $false

# Scripts live in ci\windows\; operate from the project root (two levels up).
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# The CodeGen/Target suites link the LLVM C API, so their test executables import
# LLVM-C.dll. They run from $OUT, not next to the DLL, so put the LLVM bin dir on
# PATH for the run step — else they abort at startup with STATUS_DLL_NOT_FOUND
# (0xC0000135). build/llvm/bin is populated by stage0.
$llvmBin = Join-Path (Get-Location) 'build\llvm\bin'
if (Test-Path (Join-Path $llvmBin 'LLVM-C.dll')) { $env:PATH = "$llvmBin;$env:PATH" }

# Scratch for per-suite test binaries/logs; under build/ but separate from
# the stage dirs so it doesn't sit next to the release artifact.
$OUT = "build/test"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

# CodeGen/target suites emit IR/objects to /tmp/cg and read them back. On native
# Windows a leading-`/` path resolves to the current drive root, so
# /tmp/cg => <drive>:\tmp\cg. Create it up front.
New-Item -ItemType Directory -Force "$($PWD.Drive.Root)tmp\cg" | Out-Null

# Compiler under test: $FLY (default: the compiler for $STAGE - see below; the
# that ships; its --test system was ported from the reference). Resolved to an
# absolute path for the argv[0]-based stdlib lookup.
# -- Stage plumbing: WHICH compiler runs the tests. ----------------------------
# STAGE=N runs the suites with build\stageN's own compiler — each stage tests the
# compiler it just produced, so every step of the bootstrap is covered:
#   STAGE=0  the pinned REFERENCE seed that stage0 downloaded. The suites
#            compile the in-tree std sources (-L), so a failure here is a
#            SOURCE-level problem (from 0.13.14 the seed ships no std of its own).
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

$suites = Get-ChildItem -Recurse -Filter *Suite.fly compiler/test | Sort-Object FullName

# ── Selector ──────────────────────────────────────────────────────────────────
# $SEL narrows the run: bare suite name, ns.SuiteName, or a namespace (its
# whole subtree). Mirrors the compiler's --suite matching so the per-suite
# fallback (STAGE=0 seed has no namespace selector) filters identically.
$SEL = if ($Suite) { $Suite } elseif ($env:FLY_TEST_SUITE) { $env:FLY_TEST_SUITE } else { '' }

function Test-SuiteSelMatch([string]$file, [string]$name, [string]$sel) {
    if (-not $sel) { return $true }
    if ($name -eq $sel) { return $true }
    $nsLine = Select-String -Path $file -Pattern '^namespace\s+([A-Za-z0-9_.]+)' | Select-Object -First 1
    if (-not $nsLine) { return $false }             # namespace-less: bare name only
    $ns = $nsLine.Matches[0].Groups[1].Value
    if ($ns -eq $sel) { return $true }
    if ("$ns.$name" -eq $sel) { return $true }
    if ($ns.StartsWith("$sel.")) { return $true }
    return $false
}

$selected = @($suites | Where-Object { Test-SuiteSelMatch $_.FullName $_.BaseName $SEL })
if ($selected.Count -eq 0) {
    Write-Host "error: no suite or namespace matches '$SEL' under compiler/test"
    exit 1
}

# ── Run mode ──────────────────────────────────────────────────────────────────
# ONE-SHOT (default for the self-host stages): a single bare `--suite` build —
# the driver discovers EVERY suite file, compiles the tree ONCE into one test
# binary and runs all suites sequentially in it. That turns ~124 near-full
# recompiles of the compiler tree (the old per-suite loop) into one ~45s
# compile+run. The per-suite loop remains for:
#   STAGE=0                 the pinned seed predates the all-suites test main;
#   FLY_TEST_PER_SUITE=1    debugging — per-suite exes, logs and exit codes.
$perSuite = ($STAGE -eq '0') -or ($env:FLY_TEST_PER_SUITE -eq '1')

if (-not $perSuite) {
    $log = "$OUT/_oneshot.log"
    if ($SEL) {
        & $FLY --suite=$SEL --src-dir compiler -o test_all --out-dir $OUT -L $STD *> $log
    } else {
        & $FLY --suite --src-dir compiler -o test_all --out-dir $OUT -L $STD *> $log
    }
    $code = $LASTEXITCODE

    # Per-suite results from the runner's own report lines, DEDUPED by name with
    # the worst failure count kept (a self-spawning suite — see OsProcSuite —
    # re-runs the binary as a child, duplicating every report line).
    $reported = @{}
    foreach ($m in (Select-String -Path $log -Pattern '^suite (\S+): (\d+) cases, (\d+) passed, (\d+) failed')) {
        $name = $m.Matches[0].Groups[1].Value
        $nfail = [int]$m.Matches[0].Groups[4].Value
        if (-not $reported.ContainsKey($name) -or $nfail -gt $reported[$name]) { $reported[$name] = $nfail }
    }
    $pass = 0
    $fail = 0
    foreach ($name in ($reported.Keys | Sort-Object)) {
        if ($reported[$name] -eq 0) {
            Write-Host "  PASS          $name"
            $pass++
        } else {
            Write-Host "  RUN  FAIL     $name ($($reported[$name]) failed)"
            $fail++
        }
    }

    if ($reported.Count -eq 0) {
        # No report at all = the compile broke: show the diagnostics.
        Write-Host "  COMPILE FAIL  (exit $code) - $log"
        $hits = Select-String -Path $log -Pattern 'error:|broken|abort' | Select-Object -First 6
        if ($hits) { $hits | ForEach-Object { "      $($_.Line)" } }
        else { Get-Content -Tail 6 $log | ForEach-Object { "      $_" } }
        exit 1
    }

    # Failing cases, verbatim, for every red suite.
    if ($fail -gt 0) {
        Select-String -Path $log -Pattern 'FAIL\(' | Select-Object -First 12 |
            ForEach-Object { "      $($_.Line)" }
    }

    # Crash containment: a suite that dies (AV) kills the shared runner, so the
    # suites after it never report. Surface them instead of undercounting. The
    # expectation set honors the selector (matched the same way the compiler
    # matches --suite=<sel>).
    $missing = @($selected | Where-Object { -not $reported.ContainsKey($_.BaseName) })
    foreach ($m2 in $missing) {
        Write-Host "  NO REPORT     $($m2.BaseName) (run aborted before it? exit $code)"
    }

    Write-Host ([string]::new([char]0x2500, 45))
    Write-Host "  $pass passed, $fail failed, $($missing.Count) unreported (one-shot, exit $code)"
    if (($fail -eq 0) -and ($missing.Count -eq 0) -and ($code -eq 0)) { exit 0 } else { exit 1 }
}

$pass = 0
$fail = 0
foreach ($suite in $selected) {
    $name = $suite.BaseName
    $log = "$OUT/_$name.log"

    # One-shot per suite: --suite=Name compiles AND runs; fly's exit code is the
    # run's code (or the compile failure). DIRECTORY CLI (every stage): the suite
    # is discovered by name from the compiler/ tree (suite names repeat across
    # trees — ManifestSuite also exists in std/test — and compiler/ as the root
    # keeps the driver/compiler imports resolving from source).
    & $FLY --suite=$name --src-dir compiler -o "test_$name" --out-dir $OUT -L $STD *> $log
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
        # Match real diagnostics (`error:`), not the substring "error" inside
        # warnings like 'errorHandler'. If nothing matches, show the log tail.
        $hits = Select-String -Path $log -Pattern 'error:|broken|abort' | Select-Object -First 3
        if ($hits) { $hits | ForEach-Object { "      $($_.Line)" } }
        else { Get-Content -Tail 3 $log | ForEach-Object { "      $_" } }
    }
    $fail++
}

Write-Host ([string]::new([char]0x2500, 45))
Write-Host "  $pass passed, $fail failed"
if ($fail -eq 0) { exit 0 } else { exit 1 }
