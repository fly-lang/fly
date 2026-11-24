# -----------------------------------------------------------------------------
# build_stage2.ps1 - build stage 2 (0.16): the stage-1 fly runs
#
#   fly build --profile dist[-debug] --out-dir build\stage2 --keep-objects
#
# on the root Manifest.fly: runtime, TLS archives, std, compiler, fly-lsp and
# fly-registry, plus the LLVM pieces stage 0 fetched, laid out as the release
# (bin\ and lib\). `dist-debug` when FLY_DEBUG_SYMBOLS=1. The executables'
# objects (--keep-objects) are moved to build\stage2\obj, outside what the
# release packages: stage3.ps1 compares obj\fly.o with its own.
#
# Shared by stage2.ps1 and repro_check.ps1 (which builds stage 2 twice).
# Exit 0 = build\stage2 complete.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$FLY1 = 'build/stage1/bin/fly.exe'
if (-not (Test-Path $FLY1 -PathType Leaf)) {
    Write-Host "error: stage1 fly missing - run ci\windows\stage1.ps1 first."; exit 1
}
$FLY_PROFILE = if ($env:FLY_DEBUG_SYMBOLS -eq '1') { 'dist-debug' } else { 'dist' }

Remove-Item 'build/stage2' -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "stage2: fly build --profile $FLY_PROFILE --out-dir build\stage2 (the stage-1 fly) ..."
$sw = [Diagnostics.Stopwatch]::StartNew()
& $FLY1 build --profile $FLY_PROFILE --out-dir build/stage2 --keep-objects
if ($LASTEXITCODE -ne 0) { Write-Host "error: stage2 fly build failed (exit $LASTEXITCODE)"; exit 1 }
Write-Host ("stage2: built in {0:n0}s" -f $sw.Elapsed.TotalSeconds)

# The stage is only meaningful if it produced ITS OWN compiler.
foreach ($need in 'build/stage2/bin/fly.exe', 'build/stage2/bin/fly-lsp.exe', 'build/stage2/bin/fly-registry.exe',
                  'build/stage2/bin/fly.o', 'build/stage2/lib/fly_std_lib.lib') {
    if (-not (Test-Path $need -PathType Leaf)) {
        Write-Host "error: stage2 build reported success but $need is missing."; exit 1
    }
}
New-Item -ItemType Directory -Force 'build/stage2/obj' | Out-Null
Get-ChildItem 'build/stage2/bin/*.o' | Move-Item -Destination 'build/stage2/obj' -Force
exit 0
