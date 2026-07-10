# -----------------------------------------------------------------------------
# build_driver.ps1 - compile the driver (driver/lib, entry Driver.fly) into a
# merged object at build\stage$STAGE\driver; link_fly.ps1 then links fly.exe.
# PowerShell port of ci/linux/build_driver.sh; see stage1.ps1 for the stage map.
#
#   STAGE=1  stage0 compiles the driver; its in-process link usually fails on
#            the LLVM C-API symbols (no auto -lLLVM when LLVMApi is
#            header-consumed) but emits the object first. If it DOES succeed,
#            the prelinked fly.exe is kept for link_fly.ps1 to install.
#   STAGE=2  the stage-1 fly.exe recompiles the driver with -c (clean object).
#
# The compiler archive + headers always come from build\stage1\compiler
# (build-only, never shipped).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# -- Stage plumbing: pick the compiler and the in/out dirs from $STAGE. --------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
$LIB = "build/stage$STAGE/lib"
$CDIR = 'build/stage1/compiler'
if ($STAGE -eq '1') {
    if (-not (Test-Path 'build/stage0/bin/fly.exe' -PathType Leaf)) {
        Write-Host "error: stage0 compiler missing - run ci\windows\stage0.ps1 first."; exit 1
    }
    # Hardlink the stage0 compiler as fly0.exe so <exe>\..\lib = build\stage1\lib.
    New-Item -ItemType Directory -Force build/stage1/bin | Out-Null
    $fly0 = (Resolve-Path 'build/stage0/bin/fly.exe').Path
    $FLY = 'build/stage1/bin/fly0.exe'
    Remove-Item $FLY -Force -ErrorAction SilentlyContinue
    try { New-Item -ItemType HardLink -Path $FLY -Target $fly0 -ErrorAction Stop | Out-Null }
    catch { Copy-Item $fly0 $FLY -Force }
} else {
    $FLY = 'build/stage1/bin/fly.exe'
    if (-not (Test-Path $FLY -PathType Leaf)) {
        Write-Host "error: stage1 fly '$FLY' not found - run ci\windows\stage1.ps1 first."; exit 1
    }
}

if (-not (Test-Path "$CDIR/fly_compiler_lib.lib")) {
    Write-Host "error: $CDIR\fly_compiler_lib.lib missing - run build_compiler.ps1 first."; exit 1
}
if (-not (Test-Path "$LIB/fly_std_lib.lib") -or -not (Test-Path "$LIB/fly_runtime_lib.lib")) {
    Write-Host "error: std/runtime missing in $LIB - run build_runtime.ps1 + build_std.ps1 first."; exit 1
}

# The driver is always compiled FROM SOURCE - never consumed as a header.
Get-ChildItem "$CDIR/*.fly.h", "$LIB/*.fly.h" -ErrorAction SilentlyContinue | ForEach-Object {
    if (Select-String -Path $_.FullName -Pattern 'namespace fly.driver' -Quiet) { Remove-Item $_.FullName -Force }
}

# -- Emit the merged driver object (-L serves the compiler headers). ------------
# Kept in build\stage$STAGE\driver (with emit.log) for link_fly.ps1 + debugging.
$D = "build/stage$STAGE/driver"
Remove-Item $D -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $D | Out-Null
Write-Host "stage${STAGE}: compiling driver ..."
if ($STAGE -eq '1') {
    # stage0 reference: no -c; the in-process link may fail (tolerated), the
    # per-source object is emitted first.
    & $FLY driver/lib/Driver.fly "$CDIR/fly_compiler_lib.lib" --src-dir driver/lib -L $CDIR `
        -o fly --out-dir $D > "$D/emit.log" 2>&1
} else {
    # self-host: -c emits a clean object, no link attempt.
    & $FLY driver/lib/Driver.fly --src-dir driver/lib -L $CDIR `
        -c -o Driver --out-dir $D > "$D/emit.log" 2>&1
}
$OBJ = @("$D/Driver", "$D/Driver.fly.o", "$D/Driver.fly.obj") | Where-Object { Test-Path $_ } | Select-Object -First 1
$PRELINKED = Test-Path "$D/fly.exe"   # stage-1 in-process link may have succeeded
if (-not $OBJ -and -not $PRELINKED) {
    Write-Host "error: driver object not emitted; see $D\emit.log:"
    Get-Content "$D/emit.log" | Select-String 'error:' | Select-Object -First 5 | ForEach-Object { Write-Host "      $_" }
    exit 1
}
if ($OBJ) { Move-Item $OBJ "$D/Driver.o" -Force }

Write-Host "stage${STAGE}: driver -> $D/$(if ($PRELINKED) { 'fly.exe (prelinked)' } else { 'Driver.o' })"
exit 0
