# -----------------------------------------------------------------------------
# build_runtime.ps1 - stage the Fly runtime for Windows (see stagelib.ps1 for the
# stage map). PowerShell counterpart of ci/linux/build_runtime.sh.
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
. .\ci\windows\stagelib.ps1

foreach ($f in 'llvm.fly.h', 'runtime.fly.h', 'fly_runtime_lib.lib') {
    if (-not (Test-Path "$SEED/$f")) {
        Write-Host "error: seed '$SEED\$f' missing - run the previous stage first (or set FLY to a valid stage0)."
        exit 1
    }
    Copy-Item "$SEED/$f" $LIB/ -Force
}

Write-Host "stage${STAGE}: runtime -> $LIB/fly_runtime_lib.lib (seeded; Fly-member rebuild is a follow-up)"
exit 0
