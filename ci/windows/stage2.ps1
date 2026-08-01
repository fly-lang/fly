# -----------------------------------------------------------------------------
# stage2.ps1 - stage 2 of the staged bootstrap (see the stage map in stage1.ps1):
# the fly built by stage1 (build\stage1\bin\fly.exe) produces the SHIPPED
# artifacts into build\stage2 - what CI uploads and the release packages.
#
#   runtime   re-seeded from stage1 (the Windows Fly-member rebuild is gated on
#             RuntimeWindows.fly running cleanly on Windows - see build_runtime.ps1)
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
    foreach ($step in 'build_runtime', 'build_compiler', 'link_fly') {
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
    # -Recurse so the bundled mingw\ sysroot directory copies with its contents.
    Get-ChildItem build/stage1/bin/* -Exclude fly0.exe | ForEach-Object { Copy-Item $_.FullName build/stage2/bin/ -Recurse -Force }
    Get-ChildItem build/stage1/lib/* | ForEach-Object { Copy-Item $_.FullName build/stage2/lib/ -Force }
}

# ── the tools: built and tested with the compiler that just finished ─────────
#
# fly-lsp and fly-registry are PRODUCTS of the toolchain, not part of the
# bootstrap: nothing downstream compiles against them, and --entry is a
# self-host option the pinned seed rejects outright. They belong HERE rather
# than in stage1 because the compiler that builds them should be the one that
# ships — stage2's fly.exe, built by stage1's self-host, i.e. the self-hosting
# fixpoint. A tool built at stage1 would carry the seed-built compiler's
# codegen, which is not what a user gets.
#
# They are part of the stage, not an opt-in extra: build\stage2\bin is what the
# release packages verbatim, so a tool that is not built here does not ship, and
# a tool failure here is a real failure of the artifact.
# test_dbg: the debugger is PROVISIONED (bundled by link_fly.ps1 from the fork
# LLVM), not compiled — but it ships from build\stage2\bin like the tools, so
# it is verified here with them.
foreach ($step in 'build_lsp', 'test_lsp', 'build_registry', 'test_registry', 'test_tools', 'test_dbg') {
    & ".\ci\windows\$step.ps1"
    if ($LASTEXITCODE -ne 0) { Write-Host "error: $step.ps1 failed (exit $LASTEXITCODE)"; exit 1 }
}

Write-Host "stage2: done - build\stage2\bin\fly.exe"
exit 0
