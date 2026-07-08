# -----------------------------------------------------------------------------
# build_compiler.ps1 - build the compiler library from compiler/lib sources into
# build\stage1\compiler (fly_compiler_lib.lib + one .fly.h per module; build-only,
# never shipped). PowerShell port of ci/linux/build_compiler.sh. STAGE 1 ONLY:
# the self-host cannot yet compile its own sources. Requires build_runtime.ps1 +
# build_std.ps1 (stage 1) first - the compiler is compiled against the IN-TREE
# std/runtime headers so its symbol references match the std the final binary
# links. See stagelib.ps1 for the stage map.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
$env:STAGE = '1'
. .\ci\windows\stagelib.ps1

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
