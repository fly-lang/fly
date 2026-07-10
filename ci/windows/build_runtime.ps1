# -----------------------------------------------------------------------------
# build_runtime.ps1 - build the Fly runtime from runtime/lib/runtime-windows.fly
# into build\stage$STAGE\lib (Windows). PowerShell counterpart of
# ci/linux/build_runtime.sh; see stage1.ps1 for the stage map. Run with STAGE=1
# (the stage0 bootstrap compiles) or STAGE=2 (the stage1 fly recompiles).
#
# Like the Linux flow, this RECOMPILES the runtime from source rather than
# shipping the bootstrap archive as-is: the seed's Fly member is stale (0.13.8
# ABI - e.g. str_slot_get/time_monotonic, renamed since), and linking the current
# std/compiler against it corrupts the heap. The only thing taken from the seed
# is llvm.fly.h (a generated bridge header with no source in-tree).
#
#   runtime-windows.fly  the Win32/UCRT backend (NOT runtime.fly, which is POSIX).
#                        Compiled with an EMPTY --src-dir so the same-namespace
#                        scan doesn't also pull runtime.fly / runtime-macos.fly.
#   fly_atomic.obj       C-primitive member: the __atomic_*_4 libcalls the Fly
#                        atomic_*_i32 helpers lower to (Clang emits these; MSVC/
#                        UCRT don't ship them - on Linux they come from libgcc).
#                        Mirrors the reference runtime's C-primitive members.
#
# Output: $LIB/fly_runtime_lib.lib (fresh Fly member + atomic shim) + runtime.fly.h
# + llvm.fly.h. Requires the MSVC dev environment (cl.exe / lib.exe on PATH; CI:
# ilammy/msvc-dev-cmd).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# -- Stage plumbing: pick the compiler and the in/out dirs from $STAGE. --------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
$LIB = "build/stage$STAGE/lib"
New-Item -ItemType Directory -Force $LIB | Out-Null
if ($STAGE -eq '1') {
    $SEED = 'build/stage0/lib'          # llvm.fly.h from the bootstrap's precompiled lib
    if (-not (Test-Path 'build/stage0/bin/fly.exe' -PathType Leaf)) {
        Write-Host "error: stage0 compiler missing - run ci\windows\stage0.ps1 first."; exit 1
    }
    # Hardlink the stage0 compiler as fly0.exe so <exe>\..\lib = build\stage1\lib
    # (copy fallback for cross-volume checkouts); named fly0 so the linked stage1
    # fly.exe never clobbers it.
    New-Item -ItemType Directory -Force build/stage1/bin | Out-Null
    $fly0 = (Resolve-Path 'build/stage0/bin/fly.exe').Path
    $FLY = 'build/stage1/bin/fly0.exe'
    Remove-Item $FLY -Force -ErrorAction SilentlyContinue
    try { New-Item -ItemType HardLink -Path $FLY -Target $fly0 -ErrorAction Stop | Out-Null }
    catch { Copy-Item $fly0 $FLY -Force }
} else {
    $SEED = 'build/stage1/lib'          # llvm.fly.h from the stage1 build
    $FLY = 'build/stage1/bin/fly.exe'   # the fly linked by stage1
    if (-not (Test-Path $FLY -PathType Leaf)) {
        Write-Host "error: stage1 fly '$FLY' not found - run ci\windows\stage1.ps1 first."; exit 1
    }
}

function Assert-LastExit($what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed (exit $LASTEXITCODE)" }
}
# Space nested generic closers (`>>` -> `> >`) so a header re-read lexes them.
function Split-GenericClosers($file) {
    $c = Get-Content $file -Raw
    while ($c -match '>>') { $c = $c -replace '>>', '> >' }
    Set-Content $file $c -NoNewline
}

# llvm.fly.h has no source in-tree (generated bridge header) - always seed it.
if (-not (Test-Path "$SEED/llvm.fly.h")) {
    Write-Host "error: seed '$SEED\llvm.fly.h' missing - run the previous stage first (stage0.ps1 / stage1.ps1)."
    exit 1
}
Copy-Item "$SEED/llvm.fly.h" $LIB/ -Force

$T = 'build/tmp_runtime'
Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $T | Out-Null

# --- C-primitive member: the 32-bit __atomic_* libcalls the Fly atomic_*_i32 ---
# helpers lower to (seq_cst over the MSVC _Interlocked* intrinsics; x86_64 makes
# them naturally sequentially consistent).
$atomicC = Join-Path $T 'fly_atomic.c'
@'
/* fly_atomic.c - the 32-bit __atomic_* libcalls the Fly runtime's atomic_*_i32
 * helpers lower to. Clang/GCC emit these as libcalls; MSVC/UCRT do not ship them
 * (on Linux they come from libgcc). Provide seq_cst implementations over the MSVC
 * Interlocked intrinsics. Mirrors the C-primitive members the reference runtime
 * archive carries alongside the Fly member. */
#include <intrin.h>
unsigned int __atomic_load_4(const volatile void *p, int mo) {
    (void)mo; return (unsigned int)_InterlockedCompareExchange((volatile long *)p, 0, 0);
}
void __atomic_store_4(volatile void *p, unsigned int v, int mo) {
    (void)mo; _InterlockedExchange((volatile long *)p, (long)v);
}
unsigned int __atomic_fetch_add_4(volatile void *p, unsigned int v, int mo) {
    (void)mo; return (unsigned int)_InterlockedExchangeAdd((volatile long *)p, (long)v);
}
int __atomic_compare_exchange_4(volatile void *p, void *expected, unsigned int desired,
                                int weak, int smo, int fmo) {
    (void)weak; (void)smo; (void)fmo;
    long exp = *(long *)expected;
    long old = _InterlockedCompareExchange((volatile long *)p, (long)desired, exp);
    if (old == exp) { return 1; }
    *(long *)expected = old; return 0;
}
'@ | Set-Content $atomicC -Encoding ascii
Push-Location $T
cl /nologo /c /O2 fly_atomic.c | Out-Null
$clExit = $LASTEXITCODE
Pop-Location
if ($clExit -ne 0) { Write-Host "error: cl compiling fly_atomic.c failed (exit $clExit)"; exit 1 }

# --- Compile the runtime Fly member. --src-dir $T (empty of .fly) so the ---------
# same-namespace scan doesn't pull runtime.fly / runtime-macos.fly (three
# definitions of every C-ABI symbol, wrong-platform code). Like build_runtime.sh,
# the invocation differs by producer: the stage0 reference `--lib` emits the
# archive itself; the self-host `--lib` emits one merged object.
Write-Host "stage${STAGE}: compiling runtime/lib/runtime-windows.fly ..."
if ($STAGE -eq '1') {
    & $FLY --lib -o "$T/fly_runtime_lib" --src-dir $T runtime/lib/runtime-windows.fly
} else {
    & $FLY --lib -o fly_runtime_lib --out-dir $T --src-dir $T runtime/lib/runtime-windows.fly
}
Assert-LastExit 'runtime --lib build'

# The producer emits either an archive (reference: fly_runtime_lib.lib) or a bare
# merged object (self-host). Collect whichever, plus the atomic shim, into the
# final archive with lib.exe (the Windows equivalent of the Linux `ar` merge).
$rtPieces = @()
if (Test-Path "$T/fly_runtime_lib.lib") { $rtPieces += "$T/fly_runtime_lib.lib" }
Get-ChildItem "$T/*.obj", "$T/fly_runtime_lib" -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -ne 'fly_atomic.obj' } |
    ForEach-Object { $rtPieces += $_.FullName }
if ($rtPieces.Count -eq 0) { Write-Host "error: runtime object/archive not emitted."; exit 1 }

lib /nologo "/OUT:$LIB/fly_runtime_lib.lib" @rtPieces "$T/fly_atomic.obj" | Out-Null
Assert-LastExit 'runtime archive (lib.exe)'

# header (nested `>>` spaced so re-reads lex them) -> runtime.fly.h
$rtHdr = Get-ChildItem "$T/runtime-windows.fly.h", "$T/runtime.fly.h" -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $rtHdr) { Write-Host "error: runtime header (.fly.h) not emitted."; exit 1 }
Copy-Item $rtHdr.FullName "$LIB/runtime.fly.h" -Force
Split-GenericClosers "$LIB/runtime.fly.h"

Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "stage${STAGE}: runtime -> $LIB/fly_runtime_lib.lib (+ runtime.fly.h, llvm.fly.h)"
exit 0
