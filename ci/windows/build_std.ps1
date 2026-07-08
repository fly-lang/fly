# -----------------------------------------------------------------------------
# build_std.ps1 - build the standard library from std/lib sources (Windows).
# PowerShell port of ci/linux/build_std.sh; see stagelib.ps1 for the stage map.
# STAGE 1 ONLY: the stage0 reference `--lib` emits fly_std_lib.lib + one .fly.h
# per module. The SHIPPED std is this stage-1 (reference-ABI) build — a
# self-host-built std would break header consumers of classes with interface
# bases (see the Linux build_std.sh note), so STAGE=2 errors out on purpose.
# Requires build_runtime.ps1 first (std imports fly.runtime / fly.llvm).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. .\ci\windows\stagelib.ps1

if ($STAGE -ne '1') {
    Write-Host "error: build_std.ps1 supports STAGE=1 only (the shipped std is the stage-1"
    Write-Host "       reference-ABI build; a self-host std needs the class-ABI unification)."
    exit 1
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

Write-Host "stage${STAGE}: compiling $($FILES.Count) std files ..."
& $FLY --lib -o "$T/fly_std_lib" @FILES
Assert-LastExit 'std --lib build'
Move-Item "$T/fly_std_lib.lib" "$LIB/fly_std_lib.lib" -Force

# headers (nested `>>` spaced so re-reads lex them)
$hdrs = 0
Get-ChildItem "$T/*.fly.h" -ErrorAction SilentlyContinue | ForEach-Object {
    Split-GenericClosers $_.FullName
    Copy-Item $_.FullName $LIB/ -Force
    $hdrs++
}

# The SHIPPED std is this stage-1 build: install into build\lib too.
New-Item -ItemType Directory -Force build/lib | Out-Null
Copy-Item "$LIB/fly_std_lib.lib" build/lib/ -Force
Get-ChildItem "$LIB/*.fly.h" | ForEach-Object { Copy-Item $_.FullName build/lib/ -Force }

Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "stage${STAGE}: std -> $LIB/fly_std_lib.lib (+ $hdrs *.fly.h)"
exit 0
