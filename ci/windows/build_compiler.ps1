# -----------------------------------------------------------------------------
# build_compiler.ps1 - compile the compiler (monolithic: the whole compiler + the
# driver, entry compiler/lib/driver/Driver.fly) into a
# merged object at build\stage$STAGE\driver; link_fly.ps1 then links fly.exe.
# PowerShell port of ci/linux/build_compiler.sh; see stage1.ps1 for the stage map.
#
#   STAGE=1  stage0 compiles the driver; its in-process link usually fails on
#            the LLVM C-API symbols (no auto -lLLVM when LLVMApi is
#            header-consumed) but emits the object first. If it DOES succeed,
#            the prelinked fly.exe is kept for link_fly.ps1 to install.
#   STAGE=2  the stage-1 fly.exe recompiles the driver with -c (clean object).
#
# MONOLITHIC: the compiler is compiled from source INTO this object (`--src-dir compiler`),
# not linked as a static archive - see the note by the compile step below.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

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

if (-not (Test-Path "$LIB/fly_std_lib.lib") -or -not (Test-Path "$LIB/fly_runtime_lib.lib")) {
    Write-Host "error: std/runtime missing in $LIB - run build_runtime.ps1 + build_std.ps1 first."; exit 1
}

# MONOLITHIC build: the compiler is compiled FROM SOURCE into the driver object
# (`--src-dir compiler` resolves fly.compiler.* from compiler/lib source; std stays an
# external archive). There is NO fly_compiler_lib.lib static archive anymore.
# Rationale: as a static lib, the compiler's GENERIC INSTANTIATIONS (List<ASTNode>
# ...) were COMDAT-deduped by the linker against the driver's own copies, causing
# a use-after-free of a parsed module's List fields (the `fly build` Windows crash,
# root-caused 2026-07-19). Merging the compiler in removes the cross-archive dedup.
# The driver is always source (never a header).
Get-ChildItem "$LIB/*.fly.h" -ErrorAction SilentlyContinue | ForEach-Object {
    if (Select-String -Path $_.FullName -Pattern 'namespace fly.driver' -Quiet) { Remove-Item $_.FullName -Force }
}

# -- Emit the merged driver+compiler object. ------------------------------------
# Kept in build\stage$STAGE\driver (with emit.log) for link_fly.ps1 + debugging.
$D = "build/stage$STAGE/driver"
Remove-Item $D -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $D | Out-Null
$DBG = @(); if ($env:FLY_DEBUG_SYMBOLS -eq '1') { $DBG += '--debug-symbols' }
Write-Host "stage${STAGE}: compiling driver + compiler (monolithic, from source) ...$(if ($DBG) { ' (+debug-symbols)' })"
# DIRECTORY CLI (seed and self-host alike): no positional — the entry (the
# single main(), compiler/lib/driver/Driver.fly) is discovered from --src-dir
# and its import closure pulls the whole compiler.
if ($STAGE -eq '1') {
    # stage0 reference: no -c; the in-process link may fail (tolerated), the
    # per-source object is emitted first. --target keeps the object gnu COFF.
    & $FLY --src-dir compiler `
        @DBG @FLY_TARGET_ARGS -o fly --out-dir $D > "$D/emit.log" 2>&1
} else {
    # self-host: -c emits a clean object, no link attempt.
    & $FLY --src-dir compiler `
        @DBG @FLY_TARGET_ARGS -c -o Driver --out-dir $D > "$D/emit.log" 2>&1
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
