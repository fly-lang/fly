# -----------------------------------------------------------------------------
# build_registry.ps1 - build the fly-registry package server into
# build\stage$STAGE\bin.
#
# The THIRD executable in the tree, and the cheapest: unlike fly-lsp it does not
# reach into the compiler at all - it is std only (fly.net, fly.net.http,
# fly.targz, fly.os.*). So no --src-dir compiler/lib, and no -WithLLVM on the
# link: nothing here can reach CodeGen.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

# Written back into the environment for link_bin.ps1 (see build_lsp.ps1).
$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$env:STAGE = $STAGE
$OUT = "build/stage$STAGE/bin"
$LIB = "build/stage$STAGE/lib"
$D = "build/stage$STAGE/registry"
New-Item -ItemType Directory -Force $OUT, $D | Out-Null

# Built by THIS STAGE'S own fly.exe: fly-registry is a product of the toolchain,
# not part of the bootstrap (same rationale as build_lsp.ps1), and --entry is a
# self-host option the pinned seed rejects.
$FLY = "build/stage$STAGE/bin/fly.exe"
if (-not (Test-Path $FLY)) { Write-Host "error: compiler '$FLY' not found - run link_fly.ps1 for this stage first."; exit 1 }
if (-not (Test-Path 'tools/registry/lib/FlyRegistry.fly')) { Write-Host 'error: tools/registry/lib/FlyRegistry.fly not found.'; exit 1 }

$DBG = @(); if ($env:FLY_DEBUG_SYMBOLS -eq '1') { $DBG = @('--debug-symbols') }
$TARGET = @(); if ($env:FLY_TARGET_ARGS) { $TARGET = $env:FLY_TARGET_ARGS -split ' ' }

Write-Host "stage${STAGE}: compiling tools/registry ..."
& $FLY --entry tools/registry/lib/FlyRegistry.fly --src-dir tools/registry `
       @DBG @TARGET -c -o FlyRegistry --out-dir $D -L $LIB
if ($LASTEXITCODE -ne 0) { throw "fly-registry compile failed (exit $LASTEXITCODE)" }

$OBJ = @("$D/FlyRegistry", "$D/FlyRegistry.o", "$D/FlyRegistry.fly.o", "$D/FlyRegistry.fly.obj") |
       Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $OBJ) { Write-Host "error: fly-registry object not emitted in $D."; exit 1 }
if ($OBJ -ne "$D/FlyRegistry.o") { Move-Item $OBJ "$D/FlyRegistry.o" -Force }

& "$PSScriptRoot\link_bin.ps1" -Obj "$D/FlyRegistry.o" -Out "$OUT/fly-registry.exe"
if ($LASTEXITCODE -ne 0) { throw "fly-registry link failed (exit $LASTEXITCODE)" }

Write-Host "stage${STAGE}: fly-registry -> $OUT/fly-registry.exe"
exit 0
