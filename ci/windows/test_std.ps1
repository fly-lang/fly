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
# Optional SELECTOR (first argument, or $env:FLY_TEST_SUITE): a suite name, a
# qualified ns.SuiteName, or a NAMESPACE (its whole subtree) — see
# test_compiler.ps1. Empty = every std suite.
param([string]$Suite = '')

$ErrorActionPreference = 'Continue'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$OUT = "build/test"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

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

# Seed link extras. The self-host driver links these by itself; the reference
# seed does not know them, so under the seed (stage1's interleaved pass) a
# suite (TlsSuite, HttpSuite, ...) would not link. Both drivers DO link every
# archive at the top level of a -L dir, so stage them in a scratch dir:
#   - fly_tls_stub: tls_* primitives (NOT fly_tls_lib next to it - that is the
#     real backend with Schannel system deps, and must not race the stub);
#   - mingw compiler-rt builtins: the stage-1 runtime is gnu-flavoured fly code
#     (___chkstk_ms, __atomic_*) and the seed links only its MSVC builtins;
#   - ws2_32/winhttp import libs: runtime net_* landed after the seed's cut.
$EXTRA_L = @()
$flyLibDir = Join-Path (Split-Path -Parent $FLY) '..\lib'
$extras = @(
    (Join-Path $flyLibDir 'fly_tls_stub.lib'),
    (Join-Path $flyLibDir 'fly_tls_stub.a'),
    'build\mingw\builtins\libclang_rt.builtins-x86_64.a',
    'build\mingw\lib\libws2_32.a',
    'build\mingw\lib\libwinhttp.a'
)
$extrasDir = "$OUT/_seed_link"
foreach ($f in $extras) {
    if (Test-Path $f -PathType Leaf) {
        New-Item -ItemType Directory -Force $extrasDir | Out-Null
        Copy-Item $f $extrasDir -Force
        $EXTRA_L = @('-L', $extrasDir)
    }
}

$tests = Get-ChildItem -Recurse -Filter *Suite.fly std/test | Sort-Object FullName

# ── Selector ──────────────────────────────────────────────────────────────────
# Same matching as the compiler's --suite=<sel> (bare name / ns.Name /
# namespace subtree), so the per-suite loop filters identically.
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

$selected = @($tests | Where-Object { Test-SuiteSelMatch $_.FullName $_.BaseName $SEL })
if ($selected.Count -eq 0) {
    Write-Host "error: no suite or namespace matches '$SEL' under std/test"
    exit 1
}

# ── Run mode ──────────────────────────────────────────────────────────────────
# ONE-SHOT (default for the self-host stages, like test_compiler.ps1): one bare
# `--suite` build compiles the std tree ONCE into a single test binary running
# all suites — unblocked by the B036 fix (identity-strong specialization keys).
# Report lines are DEDUPED by suite name (worst result wins): OsProcSuite's
# spawnSuccessTest re-runs the whole binary as a child BY DESIGN, so every
# suite reports twice in the merged log. Per-suite loop kept for STAGE=0 (the
# pinned seed) and FLY_TEST_PER_SUITE=1 (debugging).
$oneShot = ($STAGE -ne '0') -and ($env:FLY_TEST_PER_SUITE -ne '1')

if ($oneShot) {
    # --target: suites are gnu/UCRT like everything else this bootstrap builds —
    # from 0.13.14 the seed links windows-gnu itself (LinkWindowsGNU, bundled
    # mingw sysroot); explicit so the run never depends on the driver's default.
    $log = "$OUT/_std_oneshot.log"
    if ($SEL) {
        & $FLY --suite=$SEL --target x86_64-w64-windows-gnu --src-dir std -o std_all --out-dir $OUT -L $STD @EXTRA_L *> $log
    } else {
        & $FLY --suite --target x86_64-w64-windows-gnu --src-dir std -o std_all --out-dir $OUT -L $STD @EXTRA_L *> $log
    }
    $code = $LASTEXITCODE

    # Dedup by suite name, keeping the WORST failure count (parent + self-spawn
    # child both report every suite; a suite red in either run must stay red).
    $reported = @{}
    foreach ($m in (Select-String -Path $log -Pattern '^suite (\S+): (\d+) cases, (\d+) passed, (\d+) failed')) {
        $name = $m.Matches[0].Groups[1].Value
        $nfail = [int]$m.Matches[0].Groups[4].Value
        if (-not $reported.ContainsKey($name) -or $nfail -gt $reported[$name]) { $reported[$name] = $nfail }
    }
    $pass = 0
    $fail = 0
    foreach ($name in ($reported.Keys | Sort-Object)) {
        if ($reported[$name] -eq 0) { Write-Host "  PASS          $name"; $pass++ }
        else { Write-Host "  RUN  FAIL     $name ($($reported[$name]) failed)"; $fail++ }
    }

    if ($reported.Count -eq 0) {
        Write-Host "  COMPILE FAIL  (exit $code) - $log"
        $hits = Select-String -Path $log -Pattern 'error:|broken|abort' | Select-Object -First 6
        if ($hits) { $hits | ForEach-Object { "      $($_.Line)" } }
        else { Get-Content -Tail 6 $log | ForEach-Object { "      $_" } }
        exit 1
    }

    if ($fail -gt 0) {
        Select-String -Path $log -Pattern 'FAIL\(' | Select-Object -First 12 |
            ForEach-Object { "      $($_.Line)" }
    }

    # A crashing suite kills the shared runner: surface the suites left silent.
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
foreach ($t in $selected) {
    $name = $t.BaseName
    $log = "$OUT/_std_$name.log"

    # One-shot: --suite compiles AND runs; fly's exit code is the run's code (or
    # the compile failure). DIRECTORY CLI (every stage): the suite is discovered
    # by name from the source root — `std`, not std/test, so the fly.meta SOURCE
    # under std/lib/meta stays pullable (suite names also repeat across trees:
    # ManifestSuite exists in compiler/test too).
    # --target: gnu/UCRT, matching the bootstrap's build steps (see one-shot above).
    & $FLY --suite=$name --target x86_64-w64-windows-gnu --src-dir std -o "std_$name" --out-dir $OUT -L $STD @EXTRA_L *> $log
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
