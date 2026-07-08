# -----------------------------------------------------------------------------
# build_driver.ps1 - compile the driver (driver/lib, entry Driver.fly) and link
# fly.exe: driver + fly_compiler_lib.lib + std + runtime + LLVM-C. PowerShell
# port of ci/linux/build_driver.sh; see stagelib.ps1 for the stage map.
#
#   STAGE=1  stage0 compiles the driver; its in-process link fails on the LLVM
#            C-API symbols (no auto -lLLVM when LLVMApi is header-consumed) but
#            emits the object first - detected by file, then linked externally.
#   STAGE=2  the stage-1 fly.exe recompiles the driver with -c (clean object).
#
# External link = the fork's lld-link.exe (COFF), mirroring the reference
# ToolChain: /defaultlib:libcmt /defaultlib:synchronization /defaultlib:kernel32,
# with the MSVC dev environment supplying %LIB%. FLY_BUNDLE_LLVM=1 ships
# LLVM-C.dll + lld-link.exe next to fly.exe (the loader searches the exe dir -
# no rpath on Windows). Validated in CI only (no local Windows).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. .\ci\windows\stagelib.ps1

$OUT = 'build/bin'
New-Item -ItemType Directory -Force $OUT, build/lib | Out-Null

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
$T = 'build/tmp_driver'
Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $T | Out-Null
Write-Host "stage${STAGE}: compiling driver ..."
if ($STAGE -eq '1') {
    # stage0 reference: no -c; the in-process link may fail (tolerated), the
    # per-source object is emitted first.
    & $FLY driver/lib/Driver.fly "$CDIR/fly_compiler_lib.lib" --src-dir driver/lib -L $CDIR `
        -o fly --out-dir $T > "$T/emit.log" 2>&1
} else {
    # self-host: -c emits a clean object, no link attempt.
    & $FLY driver/lib/Driver.fly --src-dir driver/lib -L $CDIR `
        -c -o Driver --out-dir $T > "$T/emit.log" 2>&1
}
$OBJ = @("$T/Driver", "$T/Driver.fly.o", "$T/Driver.fly.obj") | Where-Object { Test-Path $_ } | Select-Object -First 1
$PRELINKED = Test-Path "$T/fly.exe"   # stage-1 in-process link may have succeeded
if (-not $OBJ -and -not $PRELINKED) {
    Write-Host "error: driver object not emitted; see $T\emit.log:"
    Get-Content "$T/emit.log" | Select-String 'error:' | Select-Object -First 5 | ForEach-Object { Write-Host "      $_" }
    exit 1
}

# -- Link fly.exe with the fork's lld-link (skipped if the in-process link already
#    produced it). LLVM-C.lib supplies the LLVM C-API; %LIB% (MSVC dev env) the CRT.
if (-not $PRELINKED) {
    $llvmRoot = if (Test-Path 'build/llvm') { (Resolve-Path 'build/llvm').Path } else { $null }
    if (-not $llvmRoot) { Write-Host "error: build\llvm (fork LLVM) not found - run install_prerequisites.ps1."; exit 1 }
    $lldLink = Join-Path $llvmRoot 'bin\lld-link.exe'
    if (-not (Test-Path $lldLink)) { Write-Host "error: $lldLink not found."; exit 1 }
    if (-not $env:LIB) {
        Write-Host "error: %LIB% is empty - run from an MSVC developer environment (CI: ilammy/msvc-dev-cmd)."; exit 1
    }
    Write-Host "stage${STAGE}: linking fly.exe (fork lld-link) ..."
    & $lldLink "/out:$T/fly.exe" $OBJ "$CDIR/fly_compiler_lib.lib" `
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

# -- Atomic install: only replace bin\fly.exe on success (the stage-1 binary stays
#    intact if anything above threw).
Move-Item "$T/fly.exe" "$OUT/fly.exe" -Force
Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "stage${STAGE}: fly -> $OUT/fly.exe (libs from $LIB)"
exit 0
