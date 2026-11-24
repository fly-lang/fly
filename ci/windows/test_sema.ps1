# -----------------------------------------------------------------------------
# test_sema.ps1 - semantic-check guard for the self-host compiler.
# Mirror of ci/linux/test_sema.sh.
#
# The suites only ever compile VALID programs, so they cannot see a check that
# has gone missing: a compiler that stops rejecting `int n = aLong` still passes
# every one of them. That is how the self-host came to accept programs the seed
# refuses (narrowing, private access, abstract instantiation, ...) while the
# whole pipeline stayed green - the tree itself was protected only because the
# SEED type-checks it at stage 1.
#
# Each probe under ci/sema_probes is one rule in miniature:
#   reject\<name>\main.fly    must NOT compile, and
#   reject\<name>\expect.txt  holds a fragment of the diagnostic it must get.
#   accept\<name>\main.fly    must compile, link and exit 0 - the edge cases a
#                             check must keep allowing (a literal that fits, a
#                             shadowing declaration, an overload that differs).
#   warn\<name>\main.fly      must compile with the warnings its expect.txt lists.
#
# A reject probe fails three ways, and they are told apart on purpose:
#   ACCEPTED   the compiler took the program - the check is gone;
#   INTERNAL   it died in the backend verifier instead of diagnosing - the
#              program reached codegen, which is the same hole with a crash on top;
#   WRONG MSG  it was rejected, but not for the reason the probe is about.
#
# STAGE selects the compiler under test (default 2, the one that ships).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$env:STAGE = $STAGE
$FLY = if ($env:FLY) { $env:FLY } else { "build/stage$STAGE/bin/fly.exe" }
if (-not (Test-Path $FLY -PathType Leaf)) { Write-Host "error: compiler '$FLY' not found."; exit 1 }
$FLY = (Resolve-Path $FLY).Path

$OUT = 'build/test/sema'
New-Item -ItemType Directory -Force $OUT | Out-Null
$failed = 0
$count = 0

foreach ($probe in Get-ChildItem 'ci/sema_probes/reject' -Directory | Sort-Object Name) {
    $name = $probe.Name
    $count++
    $expect = (Get-Content (Join-Path $probe.FullName 'expect.txt') -Raw).Trim()
    $log = Join-Path $OUT "reject_$name.log"
    & $FLY @FLY_TARGET_ARGS --src-dir $probe.FullName --no-output *> $log
    $rc = $LASTEXITCODE
    $text = Get-Content $log -Raw
    if ($null -eq $text) { $text = '' }
    if ($text.Contains('backend module verification failed')) {
        Write-Host "  FAIL  reject/$name - INTERNAL error instead of a diagnostic"; $failed++
    } elseif ($rc -eq 0) {
        Write-Host "  FAIL  reject/$name - ACCEPTED (expected: $expect)"; $failed++
    } elseif (-not $text.Contains($expect)) {
        Write-Host "  FAIL  reject/$name - WRONG MSG (expected: $expect)"
        Get-Content $log | Select-Object -First 3 | ForEach-Object { Write-Host "        $_" }
        $failed++
    } else {
        Write-Host "  ok    reject/$name"
    }
}

# -- seed verdicts ------------------------------------------------------------
# The same reject probes, judged by the SEED (build\stage0). The seed compiles
# the whole tree at stage 1, so what it accepts and refuses is part of the
# language's effective definition - and every difference from the self-host must
# be a recorded decision, not an accident. A probe the seed is known to accept
# carries a seed-accepts.txt saying why; anything else must be refused by both.
# A seed that starts accepting or refusing something else fails here, which is
# exactly the moment a seed re-cut changed the language. From 0.16 the seed is
# the 0.15.2 self-host release, which enforces every rule the tree checks: no
# probe carries a seed-accepts.txt (release/0.15.X recorded the rules 0.14.9 did
# not have yet).
$SEED = 'build/stage0/bin/fly.exe'
$SEEDLIB = 'build/stage1/lib'
if ((Test-Path $SEED -PathType Leaf) -and (Test-Path $SEEDLIB)) {
    $SEED = (Resolve-Path $SEED).Path
    $SEEDLIB = (Resolve-Path $SEEDLIB).Path
    foreach ($probe in Get-ChildItem 'ci/sema_probes/reject' -Directory | Sort-Object Name) {
        $name = $probe.Name
        $count++
        $known = Test-Path (Join-Path $probe.FullName 'seed-accepts.txt')
        $log = Join-Path $OUT "seed_$name.log"
        & $SEED --src-dir $probe.FullName --no-output -L $SEEDLIB *> $log
        $accepted = ($LASTEXITCODE -eq 0)
        if ($accepted -and -not $known) {
            Write-Host "  FAIL  seed/$name - the seed ACCEPTS it and no seed-accepts.txt records why"; $failed++
        } elseif (-not $accepted -and $known) {
            Write-Host "  FAIL  seed/$name - seed-accepts.txt is stale: the seed now REJECTS it"; $failed++
        } elseif ($known) {
            Write-Host "  ok    seed/$name (recorded divergence: seed accepts)"
        } else {
            Write-Host "  ok    seed/$name"
        }
    }
} else {
    Write-Host "  skip  seed verdicts - build\stage0 or build\stage1\lib not present"
}

foreach ($probe in Get-ChildItem 'ci/sema_probes/accept' -Directory | Sort-Object Name) {
    $name = $probe.Name
    $count++
    $dir = Join-Path $OUT "accept_$name"
    New-Item -ItemType Directory -Force $dir | Out-Null
    $exe = Join-Path $dir "$name.exe"
    Remove-Item -ErrorAction SilentlyContinue $exe
    & $FLY @FLY_TARGET_ARGS --src-dir $probe.FullName -o "$name.exe" --out-dir $dir *> "$dir/compile.log"
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $exe)) {
        Write-Host "  FAIL  accept/$name - REJECTED"
        Get-Content "$dir/compile.log" | Select-Object -First 5 | ForEach-Object { Write-Host "        $_" }
        $failed++; continue
    }
    & $exe *> "$dir/run.log"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  FAIL  accept/$name - exit $LASTEXITCODE"; $failed++
    } else {
        Write-Host "  ok    accept/$name"
    }
}

# warn\<name>\main.fly must compile, link and exit 0, and its compile log must
# hold every line of expect.txt; a line starting with '!' must NOT appear (the
# cases a warning must stay quiet about). Warnings never stop a build, so like a
# missing check, a missing warning is invisible to every suite.
foreach ($probe in Get-ChildItem 'ci/sema_probes/warn' -Directory -ErrorAction SilentlyContinue | Sort-Object Name) {
    $name = $probe.Name
    $count++
    $dir = Join-Path $OUT "warn_$name"
    New-Item -ItemType Directory -Force $dir | Out-Null
    $exe = Join-Path $dir "$name.exe"
    Remove-Item -ErrorAction SilentlyContinue $exe
    & $FLY @FLY_TARGET_ARGS --src-dir $probe.FullName -o "$name.exe" --out-dir $dir *> "$dir/compile.log"
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $exe)) {
        Write-Host "  FAIL  warn/$name - REJECTED (a warning must not stop the build)"
        Get-Content "$dir/compile.log" | Select-Object -First 5 | ForEach-Object { Write-Host "        $_" }
        $failed++; continue
    }
    $log = Get-Content "$dir/compile.log" -Raw
    if ($null -eq $log) { $log = '' }
    $bad = ''
    foreach ($line in Get-Content (Join-Path $probe.FullName 'expect.txt')) {
        if (-not $line) { continue }
        if ($line.StartsWith('!')) {
            if ($log.Contains($line.Substring(1))) { $bad = "unexpected: $($line.Substring(1))"; break }
        } elseif (-not $log.Contains($line)) {
            $bad = "missing: $line"; break
        }
    }
    if ($bad) {
        Write-Host "  FAIL  warn/$name - $bad"; $failed++
    } else {
        Write-Host "  ok    warn/$name"
    }
}
# Seed-compiled sources: std/lib, std/test, runtime/lib and compiler/lib are
# compiled by the seed (string literals RAW) as well as by the self-host (escapes
# decoded, from 0.15). A literal holding a backslash, or a raw literal between
# three double quotes, would mean one thing to each, and nothing else would
# notice: the stage 3 fixpoint compares two self-host builds. Build such bytes
# from their value instead (Escapes.byteString, mem.writeByte).
$count++
$seedLit = @(Get-ChildItem 'std/lib', 'std/test', 'runtime/lib', 'compiler/lib' -Recurse -Filter *.fly |
    Select-String -Pattern '^[^/]*("[^"]*\|""")')
if ($seedLit.Count -gt 0) {
    Write-Host "  FAIL  seed-literals - a literal the seed reads differently:"
    $seedLit | Select-Object -First 5 | ForEach-Object {
        Write-Host "        $($_.Path):$($_.LineNumber): $($_.Line.Trim())"
    }
    $failed++
} else {
    Write-Host "  ok    seed-literals"
}
if ($failed -gt 0) { Write-Host "test_sema: $failed of $count probe(s) failed"; exit 1 }
Write-Host "test_sema: all $count probes behave as specified"
exit 0
