# -----------------------------------------------------------------------------
# link_fly.ps1 - link fly.exe for the current stage: the driver object
# (build_compiler.ps1, compiler merged in) + std + runtime + LLVM-C ->
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
. "$PSScriptRoot\gnu_common.ps1"

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
    Write-Host "error: $D\Driver.o missing - run build_compiler.ps1 first."; exit 1
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
    Install-LdLld                     # provision build\llvm\bin\ld.lld.exe (fork lld, GNU flavour)
    if (-not (Test-MingwSysroot)) {
        Write-Host "error: mingw/UCRT sysroot missing under build\mingw - run ci\windows\stage0.ps1."; exit 1
    }
    $llvmC = Join-Path $llvmRoot 'lib\LLVM-C.lib'
    if (-not (Test-Path $llvmC)) { Write-Host "error: $llvmC not found."; exit 1 }

    Write-Host "stage${STAGE}: linking fly.exe (fork ld.lld, mingw/UCRT) ..."
    # gnu/mingw link: ld.lld -m i386pep with the mingw CRT startup objects + import
    # libs (from build\mingw) instead of the MSVC CRT. No %LIB% / vcvars needed.
    # ld.lld keeps .debug_* sections by default, so FLY_DEBUG_SYMBOLS needs no
    # extra link flag (the objects carry DWARF iff they were built with it).
    # Weak symbols (generic specializations, vtables, init_ctors) carry a COMDAT
    # (selection Any), so ld.lld dedups them natively — no --allow-multiple-definition.
    # MONOLITHIC: the compiler is inside $OBJ (compiled from source by build_compiler),
    # NOT a separate fly_compiler_lib.lib archive - this avoids the linker COMDAT-dedup
    # of the compiler's generic instantiations that caused the `fly build` UAF crash.
    $parts = Get-MingwLinkParts
    $args = @('-m', 'i386pep') + $parts.LibDirs + $parts.Pre + @(
        $OBJ, "$LIB/fly_std_lib.lib", "$LIB/fly_runtime_lib.lib", $llvmC
    ) + $parts.Post + @('-o', "$OUT/fly.exe")
    & $script:GNU_ldLld @args
    Assert-LastExit 'driver link'
}

# -- Bundle (FLY_BUNDLE_LLVM=1): LLVM-C.dll (loaded at runtime) + ld.lld.exe (the
#    linker fly forks to link USER programs in mingw mode) next to fly.exe. The
#    mingw/UCRT sysroot itself is shipped by the release packaging (see task list).
if ($env:FLY_BUNDLE_LLVM -eq '1') {
    $llvmRoot = (Resolve-Path 'build/llvm').Path
    Install-LdLld
    Copy-Item (Join-Path $llvmRoot 'bin\LLVM-C.dll') "$OUT/LLVM-C.dll" -Force
    Copy-Item $script:GNU_ldLld "$OUT/ld.lld.exe" -Force
    # Ship the mingw/UCRT sysroot next to fly.exe so it links USER programs with no
    # external toolchain (ToolChain.getMingwSysrootDir → <exe_dir>/mingw). Copy once.
    if ((Test-Path 'build/mingw') -and -not (Test-Path "$OUT/mingw/lib/crt2.o")) {
        Copy-Item 'build/mingw' "$OUT/mingw" -Recurse -Force
    }
}

Write-Host "stage${STAGE}: fly -> $OUT/fly.exe (libs from $LIB)"
exit 0
