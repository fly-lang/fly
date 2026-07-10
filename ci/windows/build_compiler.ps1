# -----------------------------------------------------------------------------
# build_compiler.ps1 - build the compiler library from compiler/lib sources into
# build\stage1\compiler (fly_compiler_lib.lib + one .fly.h per module; build-only,
# never shipped). PowerShell port of ci/linux/build_compiler.sh. STAGE 1 ONLY:
# the self-host cannot yet compile its own sources. Requires build_runtime.ps1 +
# build_std.ps1 (stage 1) first - the compiler is compiled against the IN-TREE
# std/runtime headers so its symbol references match the std the final binary
# links. See stage1.ps1 for the stage map.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# -- Stage plumbing (STAGE forced to 1 - see header). --------------------------
$LIB = 'build/stage1/lib'
$CDIR = 'build/stage1/compiler'
New-Item -ItemType Directory -Force build/stage1/bin | Out-Null
if (-not (Test-Path 'build/stage0/bin/fly.exe' -PathType Leaf)) {
    Write-Host "error: stage0 compiler missing - run ci\windows\stage0.ps1 first."; exit 1
}
# Hardlink the stage0 compiler as fly0.exe so <exe>\..\lib = build\stage1\lib.
$fly0 = (Resolve-Path 'build/stage0/bin/fly.exe').Path
$FLY = 'build/stage1/bin/fly0.exe'
Remove-Item $FLY -Force -ErrorAction SilentlyContinue
try { New-Item -ItemType HardLink -Path $FLY -Target $fly0 -ErrorAction Stop | Out-Null }
catch { Copy-Item $fly0 $FLY -Force }

function Assert-LastExit($what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed (exit $LASTEXITCODE)" }
}
function Split-GenericClosers($file) {
    $c = Get-Content $file -Raw
    while ($c -match '>>') { $c = $c -replace '>>', '> >' }
    Set-Content $file $c -NoNewline
}

if (-not (Test-Path "$LIB/fly_std_lib.lib")) {
    Write-Host "error: std missing in $LIB - run build_runtime.ps1 + build_std.ps1 (stage 1) first."
    exit 1
}
New-Item -ItemType Directory -Force $CDIR | Out-Null

# -Filter '*.fly' can also match '*.fly.h' on Windows (8.3 short-name quirk):
# guard with an exact -like check.
$FILES = Get-ChildItem -Recurse compiler/lib -Filter '*.fly' |
    Where-Object { $_.Name -like '*.fly' -and $_.Name -notlike '*.fly.h' } |
    Sort-Object FullName | ForEach-Object { $_.FullName }
Write-Host "stage1: compiling $($FILES.Count) compiler/lib files ..."
& $FLY --lib -o "$CDIR/fly_compiler_lib" @FILES
Assert-LastExit 'compiler --lib build'

# headers (nested `>>` spaced so re-reads lex them)
Get-ChildItem "$CDIR/*.fly.h" | ForEach-Object { Split-GenericClosers $_.FullName }

$hdrs = (Get-ChildItem "$CDIR/*.fly.h").Count
Write-Host "stage1: compiler -> $CDIR/fly_compiler_lib.lib (+ $hdrs *.fly.h)"
exit 0
