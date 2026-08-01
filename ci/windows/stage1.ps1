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
#           Build and test are INTERLEAVED: the seed (fly0.exe) runs the
#           runtime and std suites against the freshly built build\stage1\lib
#           BEFORE the compiler is built, so a library defect is caught before
#           a compiler build is spent on it. Both suite sets run after
#           build_std: the runtime suites import fly.assert/fly.mem, so the
#           std archive must exist for their link (-L std/lib alone is
#           declarations-only).
#   stage2  stage2.ps1: build\stage1\bin\fly.exe rebuilds the shipped artifacts
#           into build\stage2 (the self-hosting fixpoint check).
#
# Each build_*.ps1 derives its compiler and dirs from $env:STAGE. At stage 1
# the stage0 binary is hardlinked to build\stage1\bin\fly0.exe so its
# GetModuleFileName-based <exe>\..\lib discovery serves build\stage1\lib, never
# the bootstrap's own lib. The test steps must run that same fly0.exe -
# build\stage1\bin\fly.exe does not exist until link_fly. FLY_STAGE1_TESTS=0
# skips them (fast local builds). FLY_BUNDLE_LLVM=1 is honoured by link_fly.ps1.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
$env:STAGE = '1'

$fly0 = 'build\stage1\bin\fly0.exe'   # seed hardlink, created by build_runtime.ps1
$stage1Tests = if ($env:FLY_STAGE1_TESTS) { $env:FLY_STAGE1_TESTS } else { '1' }

$steps = @('build_runtime', 'build_std', 'test_runtime', 'test_std', 'build_compiler', 'link_fly')
foreach ($step in $steps) {
    if ($step -like 'test_*') {
        if ($stage1Tests -eq '0') { continue }
        $env:FLY = $fly0
        # Per-suite, not one-shot: the bare `--suite` all-in-one-binary run is
        # a SELF-HOST semantic - the reference seed builds it but reports only
        # the first suite (exit 0, everything else unreported).
        if ($step -eq 'test_std') { $env:FLY_TEST_PER_SUITE = '1' }
    } else {
        Remove-Item env:FLY -ErrorAction SilentlyContinue
    }
    & ".\ci\windows\$step.ps1"
    Remove-Item env:FLY_TEST_PER_SUITE -ErrorAction SilentlyContinue
    if ($LASTEXITCODE -ne 0) { Write-Host "error: $step.ps1 failed (exit $LASTEXITCODE)"; exit 1 }
}
Remove-Item env:FLY -ErrorAction SilentlyContinue

# The tools (fly-lsp, fly-registry) are built and tested at STAGE 2, not here —
# see the tail of stage2.ps1. They are PRODUCTS of the toolchain, so the compiler
# that builds them should be the one that ships: stage2's fly.exe, the
# self-hosting fixpoint. Building them here would use the stage1 binary, which
# the seed produced.

Write-Host "stage1: done - build\stage1\bin\fly.exe"
exit 0
