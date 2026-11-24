# -----------------------------------------------------------------------------
# repro_check.ps1 - the shipped artifacts are REPRODUCIBLE (plan item 1.4).
#
# Rebuilds stage 2 from scratch with the same stage-1 compiler and compares every
# file of build\stage2\{bin,lib} byte for byte with the first build. Two clean
# builds from the same sources must give the same bits: a difference means the
# output depends on something other than the sources (a link timestamp, an
# address or hash order leaking into codegen, a path of the build machine).
# Stage 3 only checks the compiler OBJECT; this covers what the release ships:
# fly.exe, fly-lsp.exe, fly-registry.exe, the runtime and std archives and
# headers, and the bundled toolchain.
#
# Run after stage2.ps1 (and before anything else writes into build\stage2). The
# first build is set aside in build\stage2_ref, the second one takes its place;
# on success the reference is removed, on a mismatch both are kept and the
# differing files are listed. Only the BUILD steps of stage 2 run again: the
# tests already passed on the first build. Exit 0 = reproducible.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
$env:STAGE = '2'

if (-not (Test-Path 'build/stage2/bin/fly.exe' -PathType Leaf)) {
    Write-Host "error: no stage 2 to compare - run ci\windows\stage2.ps1 first."; exit 1
}

$ref = 'build/stage2_ref'
Remove-Item $ref -Recurse -Force -ErrorAction SilentlyContinue
Move-Item 'build/stage2' $ref
New-Item -ItemType Directory -Force build/stage2/lib | Out-Null

Write-Host "repro: rebuilding stage 2 from scratch ..."
foreach ($step in 'build_runtime', 'build_std', 'build_compiler', 'link_fly', 'build_lsp', 'build_registry') {
    try {
        & ".\ci\windows\$step.ps1" *> "build/repro_$step.log"
    } catch {
        Write-Host "error: repro $step.ps1 failed: $_ (see build\repro_$step.log)"; exit 1
    }
    if ($LASTEXITCODE -ne 0) { Write-Host "error: repro $step.ps1 failed (exit $LASTEXITCODE, see build\repro_$step.log)"; exit 1 }
}

# Every file of either tree, by path relative to the stage dir.
function Get-Tree([string]$root) {
    $map = @{}
    foreach ($sub in 'bin', 'lib') {
        $dir = Join-Path $root $sub
        if (-not (Test-Path $dir)) { continue }
        $base = (Resolve-Path $root).Path
        Get-ChildItem $dir -Recurse -File | ForEach-Object {
            $rel = $_.FullName.Substring($base.Length + 1)
            $map[$rel] = (Get-FileHash $_.FullName -Algorithm SHA256).Hash
        }
    }
    return $map
}

$a = Get-Tree $ref
$b = Get-Tree 'build/stage2'
$bad = @()
foreach ($k in $a.Keys) {
    if (-not $b.ContainsKey($k)) { $bad += "only in the first build:  $k" }
    elseif ($a[$k] -ne $b[$k]) { $bad += "differs:                  $k" }
}
foreach ($k in $b.Keys) {
    if (-not $a.ContainsKey($k)) { $bad += "only in the second build: $k" }
}

if ($bad.Count -gt 0) {
    Write-Host "repro: NOT reproducible - $($bad.Count) of $($a.Count) files:"
    $bad | Sort-Object | ForEach-Object { Write-Host "  $_" }
    Write-Host "repro: first build kept in $ref, second in build\stage2."
    exit 1
}
Remove-Item $ref -Recurse -Force
Write-Host "repro: $($a.Count) files of build\stage2\{bin,lib} identical across two clean builds."
exit 0
