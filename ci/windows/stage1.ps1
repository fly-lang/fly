# -----------------------------------------------------------------------------
# stage1.ps1 - stage 1 of the Rust-style staged bootstrap (Windows mirror of
# ci/linux/stage1.sh):
#
#   stage0  downloaded bootstrap compiler + precompiled lib (build\stage0, set
#           up by stage0.ps1 - which also fetches the fork LLVM pieces).
#   stage1  THIS SCRIPT: stage0 builds runtime(seed) -> std -> compiler ->
#           driver FROM IN-TREE SOURCES and links the first self-host fly.exe,
#           all into build\stage1. The compiler is built against the in-tree
#           std headers so its symbol references match the std the binary links.
#   stage2  stage2.ps1: build\stage1\bin\fly.exe rebuilds the shipped artifacts
#           into build\stage2 (the self-hosting fixpoint check).
#
# Each build_*.ps1 derives its compiler and dirs from $env:STAGE. At stage 1
# the stage0 binary is hardlinked to build\stage1\bin\fly0.exe so its
# GetModuleFileName-based <exe>\..\lib discovery serves build\stage1\lib, never
# the bootstrap's own lib. FLY_BUNDLE_LLVM=1 is honoured by link_fly.ps1.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
$env:STAGE = '1'

foreach ($step in 'build_runtime', 'build_std', 'build_compiler', 'link_fly') {
    & ".\ci\windows\$step.ps1"
    if ($LASTEXITCODE -ne 0) { Write-Host "error: $step.ps1 failed (exit $LASTEXITCODE)"; exit 1 }
}

# The tools (fly-lsp, fly-registry) are built and tested at STAGE 2, not here —
# see the tail of stage2.ps1. They are PRODUCTS of the toolchain, so the compiler
# that builds them should be the one that ships: stage2's fly.exe, the
# self-hosting fixpoint. Building them here would use the stage1 binary, which
# the seed produced.

Write-Host "stage1: done - build\stage1\bin\fly.exe"
exit 0
