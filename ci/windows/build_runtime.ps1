# -----------------------------------------------------------------------------
# build_runtime.ps1 - stage the Fly runtime for Windows into build\stage$STAGE\lib.
# PowerShell counterpart of ci/linux/build_runtime.sh; see stage1.ps1 for the
# stage map. Run with STAGE=1 (seeds from the stage0 lib) or STAGE=2 (seeds
# from the stage1 lib).
#
# On Windows the runtime is SEED-ONLY for now: the bootstrap's fly_runtime_lib.lib
# (Windows C primitives + the reference-built Fly member) plus llvm.fly.h and
# runtime.fly.h are staged as-is. The Linux flow rebuilds the Fly member from
# runtime/lib/runtime.fly via `ar` member surgery; the Windows equivalent
# (lib.exe /REMOVE + merge, runtime-windows.fly as the source) is a follow-up -
# blocked on validating archive-member handling in CI (no local Windows).
# Output: $LIB/fly_runtime_lib.lib + runtime.fly.h + llvm.fly.h.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# -- Stage plumbing: pick the in/out dirs from $STAGE. -------------------------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
$LIB = "build/stage$STAGE/lib"
New-Item -ItemType Directory -Force $LIB | Out-Null
$SEED = if ($STAGE -eq '1') { 'build/stage0/lib' } else { 'build/stage1/lib' }

foreach ($f in 'llvm.fly.h', 'runtime.fly.h', 'fly_runtime_lib.lib') {
    if (-not (Test-Path "$SEED/$f")) {
        Write-Host "error: seed '$SEED\$f' missing - run the previous stage first (stage0.ps1 / stage1.ps1)."
        exit 1
    }
    Copy-Item "$SEED/$f" $LIB/ -Force
}

Write-Host "stage${STAGE}: runtime -> $LIB/fly_runtime_lib.lib (seeded; Fly-member rebuild is a follow-up)"
exit 0
