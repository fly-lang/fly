# -----------------------------------------------------------------------------
# build_std.ps1 - build the standard library from std/lib sources into
# build\stage$STAGE\lib (Windows). PowerShell port of ci/linux/build_std.sh; see
# stage1.ps1 for the stage map. Requires build_runtime.ps1 first (std imports
# fly.runtime / fly.llvm). Output: fly_std_lib.lib + one .fly.h per module.
#
# Who compiles it:
#   STAGE=1                      the SEED (a self-host release since 0.15). This
#                                build only serves to link the stage-1 compiler,
#                                which the seed also compiles.
#   STAGE=1 FLY_STD_BUILDER=self the stage-1 fly, right after link_fly: it
#                                replaces the seed's std in build\stage1\lib, so
#                                the stage-1 suites and stage 2 use a std built
#                                by the compiler of THIS tree.
#   STAGE=2                      the stage-1 fly, like every stage-2 artifact.
#                                This is the std that ships.
# Every builder is a self-host compiler, so the invocation is the same; only
# the binary differs.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

# -- Stage plumbing: pick the compiler and the output dir. ---------------------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
$bySeed = ($STAGE -eq '1') -and ($env:FLY_STD_BUILDER -ne 'self')
$LIB = "build/stage$STAGE/lib"
New-Item -ItemType Directory -Force $LIB, build/stage1/bin | Out-Null
if ($bySeed) {
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
    $who = 'the seed'
} else {
    $FLY = 'build/stage1/bin/fly.exe'
    if (-not (Test-Path $FLY -PathType Leaf)) {
        Write-Host "error: stage1 fly '$FLY' not found - run ci\windows\stage1.ps1 first."; exit 1
    }
    $who = 'the stage-1 fly'
}

function Assert-LastExit($what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed (exit $LASTEXITCODE)" }
}
if (-not (Test-Path "$LIB/fly_runtime_lib.lib") -or -not (Test-Path "$LIB/runtime.fly.h")) {
    Write-Host "error: runtime missing in $LIB - run build_runtime.ps1 first."
    exit 1
}

$T = 'build/tmp_std'
Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $T | Out-Null
$STD = 'std/lib'

# DIRECTORY CLI: --lib compiles the whole --src-dir - std/lib IS the library.
$DBG = @(); if ($env:FLY_DEBUG_SYMBOLS -eq '1') { $DBG += '--debug-symbols' }
Write-Host "stage${STAGE}: compiling std/lib with $who (codegen gnu, link mingw) ...$(if ($DBG) { ' (+debug-symbols)' })"
# --out-dir is required: without it the self-host writes each module's
# `.fly.h` next to its source, inside std/lib.
& $FLY --lib @DBG @FLY_TARGET_ARGS -o fly_std_lib --out-dir $T --src-dir $STD
Assert-LastExit 'std --lib build'
# The self-host emits one merged object; the linker reads it by content, so it
# is kept under the `.lib` name.
$emitted = @("$T/fly_std_lib.lib", "$T/fly_std_lib.a", "$T/fly_std_lib") | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $emitted) { Write-Host "error: std library not emitted."; exit 1 }
Move-Item $emitted "$LIB/fly_std_lib.lib" -Force

# A lib directory holds `.fly.h` and nothing else: a header carries declarations,
# and for a module that declares generics it carries their source too. Copied
# VERBATIM. Nested `>>` used to be spaced by a pass over the text here; the
# compiler now emits them spaced itself, which is the only safe place for it -
# rewriting the file would also hit the `>>` of a real right-shift inside a
# template body.
$hdrs = 0
Get-ChildItem "$T/*.fly.h" -ErrorAction SilentlyContinue | ForEach-Object {
    Copy-Item $_.FullName $LIB/ -Force
    $hdrs++
}
if ($hdrs -eq 0) { Write-Host "error: no std headers emitted."; exit 1 }

Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "stage${STAGE}: std -> $LIB/fly_std_lib.lib (+ $hdrs *.fly.h, built by $who)"
exit 0
