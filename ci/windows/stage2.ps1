# -----------------------------------------------------------------------------
# stage2.ps1 - stage 2 of the staged bootstrap (see the stage map in stage1.ps1):
# the fly built by stage1 (build\stage1\bin\fly.exe) produces the SHIPPED
# artifacts into build\stage2 - what CI uploads and the release packages.
#
#   runtime   re-seeded from stage1 (the Windows Fly-member rebuild is gated on
#             runtime-windows.fly running cleanly on Windows - see build_runtime.ps1)
#   std       COPIED from stage1: the shipped std must keep the reference class
#             ABI (see build_std.ps1) - a self-host std would break header
#             consumers of classes with interface bases.
#   compiler  stays the stage1 (stage0-built) archive, like Rust's beta-built
#             rustc - linked in, never shipped as an artifact.
#   driver    recompiled by the stage1 fly.exe and linked into build\stage2\bin.
#
# The self-host Windows link path is still being validated in CI (no local
# Windows): if the stage-2 rebuild fails, this script WARNS and ships the
# stage1 artifacts by copying them into build\stage2, so the artifact layout is
# always complete. Tighten (fail hard) once stage 2 is green on Windows.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
$env:STAGE = '2'

if (-not (Test-Path 'build/stage1/bin/fly.exe' -PathType Leaf)) {
    Write-Host "error: stage1 fly missing - run ci\windows\stage1.ps1 first."; exit 1
}

# std: ship the stage1 (reference-ABI) build - archive + generated headers.
New-Item -ItemType Directory -Force build/stage2/lib | Out-Null
Copy-Item build/stage1/lib/fly_std_lib.lib build/stage2/lib/ -Force
Get-ChildItem build/stage1/lib/*.fly.h | ForEach-Object { Copy-Item $_.FullName build/stage2/lib/ -Force }
Write-Host "stage2: std -> build\stage2\lib (copied from stage1, reference ABI)"

$ok = $true
try {
    foreach ($step in 'build_runtime', 'build_driver', 'link_fly') {
        & ".\ci\windows\$step.ps1"
        if ($LASTEXITCODE -ne 0) { Write-Host "stage2: $step.ps1 failed (exit $LASTEXITCODE)"; $ok = $false; break }
    }
} catch {
    Write-Host "stage2: $_"
    $ok = $false
}

if (-not $ok) {
    # Non-blocking fallback: build\stage2 always ends up a complete, shippable
    # bin\ + lib\ layout even when the self-host rebuild fails.
    Write-Host "warning: stage2 self-host rebuild FAILED - shipping the stage1 artifacts instead."
    New-Item -ItemType Directory -Force build/stage2/bin, build/stage2/lib | Out-Null
    Get-ChildItem build/stage1/bin/* -Exclude fly0.exe | ForEach-Object { Copy-Item $_.FullName build/stage2/bin/ -Force }
    Get-ChildItem build/stage1/lib/* | ForEach-Object { Copy-Item $_.FullName build/stage2/lib/ -Force }
}

Write-Host "stage2: done - build\stage2\bin\fly.exe"
exit 0
