# -----------------------------------------------------------------------------
# build_std.ps1 - build the standard library from std/lib sources into
# build\stage1\lib (Windows). PowerShell port of ci/linux/build_std.sh; see
# stage1.ps1 for the stage map. STAGE 1 ONLY: the stage0 reference `--lib`
# emits fly_std_lib.lib + one .fly.h per module. The SHIPPED std is this
# stage-1 (reference-ABI) build - a self-host-built std would break header
# consumers of classes with interface bases (see the Linux build_std.sh note),
# so STAGE=2 errors out on purpose (stage2.ps1 copies the stage1 std instead).
# Requires build_runtime.ps1 first (std imports fly.runtime / fly.llvm).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

# -- Stage plumbing (STAGE 1 only - see header). -------------------------------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
if ($STAGE -ne '1') {
    Write-Host "error: build_std.ps1 supports STAGE=1 only (the shipped std is the stage-1"
    Write-Host "       reference-ABI build; stage2.ps1 copies it into build\stage2\lib)."
    exit 1
}
$LIB = 'build/stage1/lib'
New-Item -ItemType Directory -Force $LIB, build/stage1/bin | Out-Null
# Hardlink the stage0 compiler as fly0.exe so the exe-path-based lib discovery
# serves build\stage1\lib (never the bootstrap's own lib); copy fallback for
# cross-volume checkouts. Named fly0 so the linked stage1 fly.exe never clobbers it.
if (-not (Test-Path 'build/stage0/bin/fly.exe' -PathType Leaf)) {
    Write-Host "error: stage0 compiler missing - run ci\windows\stage0.ps1 first."; exit 1
}
$fly0 = (Resolve-Path 'build/stage0/bin/fly.exe').Path
$FLY = 'build/stage1/bin/fly0.exe'
Remove-Item $FLY -Force -ErrorAction SilentlyContinue
try { New-Item -ItemType HardLink -Path $FLY -Target $fly0 -ErrorAction Stop | Out-Null }
catch { Copy-Item $fly0 $FLY -Force }

function Assert-LastExit($what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed (exit $LASTEXITCODE)" }
}
# Space nested generic closers (`>>` -> `> >`) so a header re-read lexes them
# (the parser fuses `>>`); idempotent, handles `>>>` via the loop.
function Split-GenericClosers($file) {
    $c = Get-Content $file -Raw
    while ($c -match '>>') { $c = $c -replace '>>', '> >' }
    Set-Content $file $c -NoNewline
}

if (-not (Test-Path "$LIB/fly_runtime_lib.lib") -or -not (Test-Path "$LIB/runtime.fly.h")) {
    Write-Host "error: runtime missing in $LIB - run build_runtime.ps1 first."
    exit 1
}

$T = 'build/tmp_std'
Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $T | Out-Null
$STD = 'std/lib'
$FILES = @(
    "$STD/assert.fly"; "$STD/str.fly"; "$STD/math.fly"
    "$STD/os/time.fly"; "$STD/os/env.fly"; "$STD/os/path.fly"; "$STD/os/io.fly"; "$STD/os/fs.fly"
    "$STD/sync.fly"; "$STD/mem.fly"; "$STD/bridge/clang.fly"
    "$STD/data/list.fly"; "$STD/data/stack.fly"; "$STD/data/queue.fly"; "$STD/data/deque.fly"
    "$STD/data/map.fly"; "$STD/data/set.fly"; "$STD/data/tree.fly"; "$STD/data/wrapper.fly"
    "$STD/os/proc.fly"
)

$DBG = @(); if ($env:FLY_DEBUG_SYMBOLS -eq '1') { $DBG += '--debug-symbols' }
Write-Host "stage${STAGE}: compiling $($FILES.Count) std files (codegen '$(if ($script:FLY_CODEGEN) { $script:FLY_CODEGEN } else { 'msvc(default)' })', link mingw) ...$(if ($DBG) { ' (+debug-symbols)' })"
& $FLY --lib @DBG @FLY_TARGET_ARGS -o "$T/fly_std_lib" @FILES
Assert-LastExit 'std --lib build'
# gnu target emits a `.a` archive; keep the `.lib` name the build references.
$emitted = if (Test-Path "$T/fly_std_lib.lib") { "$T/fly_std_lib.lib" } else { "$T/fly_std_lib.a" }
if (-not (Test-Path $emitted)) { Write-Host "error: std archive not emitted."; exit 1 }
Move-Item $emitted "$LIB/fly_std_lib.lib" -Force

# headers (nested `>>` spaced so re-reads lex them)
$hdrs = 0
Get-ChildItem "$T/*.fly.h" -ErrorAction SilentlyContinue | ForEach-Object {
    Split-GenericClosers $_.FullName
    Copy-Item $_.FullName $LIB/ -Force
    $hdrs++
}

Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "stage${STAGE}: std -> $LIB/fly_std_lib.lib (+ $hdrs *.fly.h)"
exit 0
