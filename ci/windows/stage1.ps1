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
#           THEN the suites run - with build\stage1\bin\fly.exe, the compiler
#           this stage just produced, not with the seed.
#   stage2  stage2.ps1: build\stage1\bin\fly.exe rebuilds the shipped artifacts
#           into build\stage2 (the self-hosting fixpoint check).
#
# Build first, test after (0.13.15): the suites used to run BEFORE the compiler,
# driven by the seed (fly0.exe) via --suite, to catch a library defect before a
# compiler build was spent on it. That cost a user-facing test RUNNER in the
# reference that nothing else used, and forced std/test to stay seed-compatible.
# The reference's --test/--suite are gone; every stage now tests with its own
# compiler (the STAGE=N contract the test_*.ps1 scripts already document), so
# these steps simply need build\stage1\bin\fly.exe to exist - hence they follow
# link_fly and need no $env:FLY override.
#
# Each build_*.ps1 derives its compiler and dirs from $env:STAGE. At stage 1
# the stage0 binary is hardlinked to build\stage1\bin\fly0.exe so its
# GetModuleFileName-based <exe>\..\lib discovery serves build\stage1\lib, never
# the bootstrap's own lib. FLY_STAGE1_TESTS=0 skips the test steps (fast local
# builds; CI sets it and runs the same three suites from its own "Test stage 1"
# step instead, which is continue-on-error - a stage-1 test failure must not
# abort the pipeline before stage2 is built and uploaded, whereas here it is
# fatal). FLY_BUNDLE_LLVM=1 is honoured by link_fly.ps1.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
$env:STAGE = '1'

$stage1Tests = if ($env:FLY_STAGE1_TESTS) { $env:FLY_STAGE1_TESTS } else { '1' }

# No $env:FLY override: at STAGE=1 the test scripts already default to
# build\stage1\bin\fly.exe, which link_fly has produced by the time they run.
# Tests are bottom-up (runtime -> std -> compiler): the runtime underpins std,
# which the compiler and driver both build on, so a failure is reported at the
# lowest broken layer first. test_compiler is here - not just in CI's "Test
# stage 1" step - because the compiler suites are the LARGEST set and the ones
# most likely to catch a stage1 codegen defect; leaving them out meant a local
# stage1 built a self-host and never exercised its own compiler.
$steps = @('build_runtime', 'build_std', 'build_compiler', 'link_fly',
           'test_runtime', 'test_std', 'test_compiler')
foreach ($step in $steps) {
    if (($step -like 'test_*') -and ($stage1Tests -eq '0')) { continue }
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
