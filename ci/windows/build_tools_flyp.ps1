# -----------------------------------------------------------------------------
# build_tools_flyp.ps1 - build the `flyp` package manager (Fly port, tools/flyp).
# Windows PowerShell port of build_tools_flyp.sh.
#
# Runs AFTER build_compiler.ps1 (which produced build/lib + build/bin/fly.exe).
# flyp is a plain Fly program: it imports the std and the tools/flyp modules and
# shells out to the sibling `fly` compiler at run time, so it links only the std
# archives (no LLVM) and ships next to fly.exe in bin/. The self-host fly cannot
# compile flyp (no generic --src-dir pull), so a COPY of the bootstrap $FLY run
# from build/bin drives the build — <exe>\..\lib then resolves to build/lib and
# flyp links the same std that ships.
# -----------------------------------------------------------------------------

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$OUT = "build/bin"
$LIB = "build/lib"

# Bootstrap compiler via $FLY (default `fly` on PATH), resolved to an absolute
# path for the argv[0]-based stdlib lookup. Same handling as build_compiler.ps1.
$FLY = if ($env:FLY) { $env:FLY } else { "fly" }
if ($FLY -notmatch '[\\/]') {
    $resolved = Get-Command $FLY -CommandType Application -ErrorAction SilentlyContinue |
                Select-Object -First 1
    if (-not $resolved) {
        Write-Host "error: bootstrap compiler '$FLY' not found on PATH."
        Write-Host "       `$env:FLY = 'C:\path\to\fly\build\bin\fly.exe'"
        exit 1
    }
    $FLY = $resolved.Source
}
if (-not (Test-Path $FLY -PathType Leaf)) {
    Write-Host "error: FLY='$FLY' is not an executable file."
    exit 1
}

# The compiled std must already exist (build_compiler.ps1 runs first).
if (-not (Test-Path "$LIB/fly_std_lib.lib")) {
    Write-Host "error: $LIB/fly_std_lib.lib not found - run ci\windows\build_compiler.ps1 first."
    exit 1
}

# Build flyp with a bootstrap copy run from build/bin so <exe>\..\lib == build/lib.
# A distinct copy name avoids clobbering the installed build/bin/fly.exe; the
# running copy is never the output file, so no staging dir is needed.
$BOOT = "$OUT/_flyp_cc.exe"
Copy-Item $FLY $BOOT -Force
& $BOOT tools/flyp/Flyp.fly `
    --src-dir tools/flyp `
    -o flyp --out-dir $OUT
if ($LASTEXITCODE -ne 0) { Remove-Item $BOOT -Force -ErrorAction SilentlyContinue; throw "flyp build failed: exit $LASTEXITCODE" }
Remove-Item $BOOT -Force -ErrorAction SilentlyContinue
Remove-Item "$OUT/*.o" -Force -ErrorAction SilentlyContinue

if (-not (Test-Path "$OUT/flyp.exe")) {
    Write-Host "error: flyp.exe was not built at $OUT/flyp.exe."
    exit 1
}
Write-Host "flyp -> $OUT/flyp.exe"
