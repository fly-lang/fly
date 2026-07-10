# -----------------------------------------------------------------------------
# link_fly.ps1 - link fly.exe for the current stage: the driver object
# (build_driver.ps1) + fly_compiler_lib.lib + std + runtime + LLVM-C ->
# build\stage$STAGE\bin\fly.exe. PowerShell port of ci/linux/link_fly.sh.
#
# External link = the fork's lld-link.exe (COFF), mirroring the reference
# ToolChain: /defaultlib:libcmt /defaultlib:synchronization /defaultlib:kernel32,
# with the MSVC dev environment supplying %LIB%. If the stage-1 in-process link
# already produced a prelinked fly.exe, it is installed as-is.
#
# FLY_BUNDLE_LLVM=1 ships LLVM-C.dll + lld-link.exe next to fly.exe (the loader
# searches the exe dir - no rpath on Windows). Validated in CI only (no local
# Windows).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# -- Stage plumbing: everything comes from the stage dirs. ---------------------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
$OUT = "build/stage$STAGE/bin"
$LIB = "build/stage$STAGE/lib"
$CDIR = 'build/stage1/compiler'
$D = "build/stage$STAGE/driver"
New-Item -ItemType Directory -Force $OUT, $LIB | Out-Null

function Assert-LastExit($what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed (exit $LASTEXITCODE)" }
}

$OBJ = "$D/Driver.o"
$PRELINKED = Test-Path "$D/fly.exe"
if (-not $PRELINKED -and -not (Test-Path $OBJ)) {
    Write-Host "error: $D\Driver.o missing - run build_driver.ps1 first."; exit 1
}
if (-not (Test-Path "$CDIR/fly_compiler_lib.lib")) {
    Write-Host "error: $CDIR\fly_compiler_lib.lib missing - run build_compiler.ps1 first."; exit 1
}
if (-not (Test-Path "$LIB/fly_std_lib.lib") -or -not (Test-Path "$LIB/fly_runtime_lib.lib")) {
    Write-Host "error: std/runtime missing in $LIB - run build_runtime.ps1 + build_std.ps1 first."; exit 1
}

# -- Link fly.exe with the fork's lld-link (skipped if the in-process link already
#    produced it). LLVM-C.lib supplies the LLVM C-API; %LIB% (MSVC dev env) the CRT.
if ($PRELINKED) {
    Write-Host "stage${STAGE}: using the prelinked fly.exe from $D"
    Copy-Item "$D/fly.exe" "$OUT/fly.exe" -Force
} else {
    $llvmRoot = if (Test-Path 'build/llvm') { (Resolve-Path 'build/llvm').Path } else { $null }
    if (-not $llvmRoot) { Write-Host "error: build\llvm (fork LLVM) not found - run ci\windows\stage0.ps1."; exit 1 }
    $lldLink = Join-Path $llvmRoot 'bin\lld-link.exe'
    if (-not (Test-Path $lldLink)) { Write-Host "error: $lldLink not found."; exit 1 }
    if (-not $env:LIB) {
        Write-Host "error: %LIB% is empty - run from an MSVC developer environment (CI: ilammy/msvc-dev-cmd)."; exit 1
    }
    Write-Host "stage${STAGE}: linking fly.exe (fork lld-link) ..."
    # Weak symbols (generic specializations, vtables, init_ctors) are emitted
    # with a COMDAT (selection Any) by both compilers, so lld-link dedups them
    # natively on COFF — no /force:multiple needed. NOTE: this requires a
    # bootstrap 0.13.8 cut AFTER the COMDAT fix in fly/ CodeGen; an older
    # bootstrap emits comdat-less weak defs and this link dies on duplicates.
    & $lldLink "/out:$OUT/fly.exe" $OBJ "$CDIR/fly_compiler_lib.lib" `
        "$LIB/fly_std_lib.lib" "$LIB/fly_runtime_lib.lib" (Join-Path $llvmRoot 'lib\LLVM-C.lib') `
        /defaultlib:libcmt /defaultlib:synchronization /defaultlib:kernel32
    Assert-LastExit 'driver link'
}

# -- Bundle (FLY_BUNDLE_LLVM=1): LLVM-C.dll + lld-link.exe next to fly.exe. -------
if ($env:FLY_BUNDLE_LLVM -eq '1') {
    $llvmRoot = (Resolve-Path 'build/llvm').Path
    Copy-Item (Join-Path $llvmRoot 'bin\LLVM-C.dll') "$OUT/LLVM-C.dll" -Force
    Copy-Item (Join-Path $llvmRoot 'bin\lld-link.exe') "$OUT/lld-link.exe" -Force
}

Write-Host "stage${STAGE}: fly -> $OUT/fly.exe (libs from $LIB)"
exit 0
