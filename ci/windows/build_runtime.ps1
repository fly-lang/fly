# -----------------------------------------------------------------------------
# build_runtime.ps1 - build the Fly runtime for Windows into build\stage$STAGE\lib.
# PowerShell counterpart of ci/linux/build_runtime.sh; see stage1.ps1 for the
# stage map.
#
# Windows targets x86_64-w64-windows-gnu (llvm-mingw / UCRT), so the runtime is
# COMPILED FROM runtime/lib/RuntimeWindows.fly with the stage0 reference compiler
# for that triple — NOT copied from the MSVC seed. stage0 emits the runtime as
# flat C-ABI, unmangled symbols with NO MSVCRT/OLDNAMES directive (CRT-neutral),
# which is exactly what links against the mingw/UCRT sysroot. (The self-host
# compiler mis-emits these definitions with the ordinary Fly ABI — see the
# selfhost-runtime-cabi note — so the runtime is stage0-built at BOTH stages until
# that is fixed; STAGE=2 re-runs this same stage0 recompile.)
#
# Output: $LIB/fly_runtime_lib.lib + runtime.fly.h + llvm.fly.h.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

# -- Stage plumbing: pick the out dir from $STAGE. -----------------------------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
$LIB = "build/stage$STAGE/lib"
New-Item -ItemType Directory -Force $LIB, build/stage1/bin | Out-Null

function Assert-LastExit($what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed (exit $LASTEXITCODE)" }
}
function Split-GenericClosers($file) {
    $c = Get-Content $file -Raw
    while ($c -match '>>') { $c = $c -replace '>>', '> >' }
    Set-Content $file $c -NoNewline
}

# -- Compiler: STAGE 1 uses the stage0 reference (hardlinked as fly0.exe so its
#    <exe>\..\lib discovery serves build\stage1\lib); STAGE 2 SELF-HOSTS with the
#    stage1 fly.exe. The self-host emits `fly.runtime` defs with the correct flat
#    C-ABI (unmangled wrappers — see CodeGenModule.emitRuntimeCABIWrapper), so its
#    runtime archive is link-compatible with the reference's. ---------------------
if ($STAGE -eq '1') {
    if (-not (Test-Path 'build/stage0/bin/fly.exe' -PathType Leaf)) {
        Write-Host "error: stage0 compiler missing - run ci\windows\stage0.ps1 first."; exit 1
    }
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

# llvm.fly.h (the fly.llvm bridge header) is a build input, not source; it ships
# with the bootstrap lib. Stage it so the runtime (and later std/compiler) resolve
# fly.llvm.* against it.
$llvmHdr = 'build/stage0/lib/llvm.fly.h'
if (-not (Test-Path $llvmHdr)) { Write-Host "error: $llvmHdr missing - run stage0.ps1 first."; exit 1 }
Copy-Item $llvmHdr "$LIB/llvm.fly.h" -Force

# -- Compile RuntimeWindows.fly → fly_runtime_lib.lib (gnu triple). ------------
$T = 'build/tmp_runtime'
Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $T | Out-Null
$DBG = @(); if ($env:FLY_DEBUG_SYMBOLS -eq '1') { $DBG += '--debug-symbols' }
Write-Host "stage${STAGE}: compiling runtime/lib/RuntimeWindows.fly (codegen gnu, link mingw) ...$(if ($DBG) { ' (+debug-symbols)' })"
& $FLY --lib @DBG @FLY_TARGET_ARGS -o "$T/fly_runtime_lib" -L $LIB --src-dir $T runtime/lib/RuntimeWindows.fly
Assert-LastExit 'runtime --lib build'
# stage0 --lib emits a `.a`/`.lib` ARCHIVE; the self-host emits ONE merged OBJECT
# (`fly_runtime_lib`, no extension). Both go to `fly_runtime_lib.lib` — ld.lld links
# a bare COFF object or an archive by content, not by the `.lib` name.
$emitted = @("$T/fly_runtime_lib.lib", "$T/fly_runtime_lib.a", "$T/fly_runtime_lib") | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $emitted) { Write-Host "error: runtime library not emitted."; exit 1 }
Move-Item $emitted "$LIB/fly_runtime_lib.lib" -Force

# runtime.fly.h — std/compiler compile against this. The compiler names the header
# after the source (RuntimeWindows.fly.h); canonicalise to runtime.fly.h.
$genHdr = "$T/RuntimeWindows.fly.h"
if (Test-Path $genHdr) {
    Split-GenericClosers $genHdr
    Copy-Item $genHdr "$LIB/runtime.fly.h" -Force
} elseif ($STAGE -eq '1') {
    Write-Host "error: runtime header not emitted."; exit 1
} else {
    # STAGE 2: std/compiler ship from stage1 and already carry runtime.fly.h; a
    # self-host that doesn't re-emit it is fine — keep the stage1 header if present.
    if (-not (Test-Path "$LIB/runtime.fly.h") -and (Test-Path 'build/stage1/lib/runtime.fly.h')) {
        Copy-Item 'build/stage1/lib/runtime.fly.h' "$LIB/runtime.fly.h" -Force
    }
}

Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "stage${STAGE}: runtime -> $LIB/fly_runtime_lib.lib (+ runtime.fly.h, llvm.fly.h)"
exit 0
