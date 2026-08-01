# -----------------------------------------------------------------------------
# test_runtime.ps1 - run every runtime/test/*Suite.fly (Windows). PowerShell port
# of test_runtime.sh. These are `suite`/`case` programs exercising the Fly runtime
# (fly.runtime + the fly.os wrappers). Each is compiled against the std + runtime
# archives via -L and driven with --suite: fly builds the suite executable AND
# runs it, exiting with the run's code; the per-case FAIL(<code>): <msg> report
# lands in the captured log.
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
# suite would not link. Both drivers DO link every archive at the top level of
# a -L dir, so stage them in a scratch dir:
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

$pass = 0
$fail = 0
$found = 0
$tests = Get-ChildItem -Filter *Suite.fly runtime/test | Sort-Object FullName
foreach ($t in $tests) {
    $name = $t.BaseName
    # Skip foreign-platform suites (their osname/arch assertions target another OS).
    if ($name -like '*Linux*' -or $name -like '*Macos*') { continue }
    $found++
    $log = "$OUT/_rt_$name.log"

    # One-shot: --suite compiles AND runs; fly's exit code is the run's code (or
    # the compile failure). DIRECTORY CLI (every stage): the suite is discovered
    # by name from the runtime/test root.
    # --target: suites are gnu/UCRT like everything else this bootstrap builds —
    # from 0.13.14 the seed links windows-gnu itself (LinkWindowsGNU, bundled
    # mingw sysroot), and the explicit flag keeps the run independent of the
    # driver's default (the seed's default is still -msvc).
    & $FLY --suite=$name --target x86_64-w64-windows-gnu --src-dir runtime/test -o "rt_$name" --out-dir $OUT -L $STD @EXTRA_L *> $log
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
if ($found -eq 0) { Write-Host "  no runtime/test/*Suite.fly found for this platform" }
Write-Host "  $pass passed, $fail failed"
if ($fail -eq 0) { exit 0 } else { exit 1 }