# -----------------------------------------------------------------------------
# install_prerequisites.ps1 - fetch and configure everything the self-host
# compiler build needs on Windows: the LLVM the compiler links against, and the
# bootstrap `fly` compiler that compiles the self-host sources.
#
# Single source of truth for the "Install LLVM (fly-lang build)" and "Download
# fly binary" steps of .github/workflows/build-windows.yml, so a local build
# reproduces CI exactly. The workflow calls this script; run it yourself to do
# the same locally.
#
# Local use: dot-source it so LIB/PATH/FLY persist in your shell, then build:
#     . .\ci\windows\install_prerequisites.ps1
#     .\ci\windows\build_compiler.ps1
#
# In CI ($GITHUB_ENV set) it appends to $GITHUB_ENV / $GITHUB_PATH instead of the
# process environment - auto-detected below.
# -----------------------------------------------------------------------------

$ErrorActionPreference = 'Stop'

# LLVM the self-host compiler links against: the project's own LLVM build
# (fly-lang/llvm-project release) rather than the stock LLVM installer. Only that
# build ships an LLVM-C.dll exporting the per-target LLVMInitialize* symbols the
# generated code references (the official/choco LLVM-C.dll omits them). This
# mirrors Linux, where apt's full libLLVM-20.so exports everything.
#
# Bootstrap compiler release used to compile the self-host sources.
#
# The literals below are the source of truth; CI may override via env so this
# stays in sync with the cache key (same pattern as $FLY in build_compiler.ps1).
$LLVM_VERSION = if ($env:LLVM_VERSION) { $env:LLVM_VERSION } else { "20.1.8" }
$FLY_VERSION  = if ($env:FLY_VERSION)  { $env:FLY_VERSION }  else { "0.13.7" }

# Resolve everything against the PROJECT ROOT (this script lives in ci\windows\,
# two levels down) so the downloads land next to the build regardless of the
# caller's cwd, and without changing it (this script is dot-sourced locally). All
# prerequisites go under build\ (alongside the build\bin output, so a single dir
# is disposable): LLVM in build\llvm, the bootstrap compiler in build\bootstrap.
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$buildDir = Join-Path $repoRoot 'build'
$llvmLib  = Join-Path $buildDir 'llvm\lib'
$llvmBin  = Join-Path $buildDir 'llvm\bin'
$flyDir   = Join-Path $buildDir 'bootstrap'
$flyBin   = Join-Path $flyDir 'bin'
$flyExe   = Join-Path $flyBin 'fly.exe'

# === LLVM ====================================================================

# --- Download LLVM (fly-lang build) ------------------------------------------
# We only need two files from the ~900 MB LLVM artifact: LLVM-C.lib (the link
# import lib) and LLVM-C.dll (loaded at runtime). Skip the download when both are
# already present - the local equivalent of the CI cache hit.
if (-not (Test-Path "$llvmLib\LLVM-C.lib") -or -not (Test-Path "$llvmBin\LLVM-C.dll")) {
    $url = "https://github.com/fly-lang/llvm-project/releases/download/v$LLVM_VERSION-win-x64/llvm-$LLVM_VERSION-win-x64.zip"
    New-Item -ItemType Directory -Force $llvmLib, $llvmBin | Out-Null
    $zipPath = Join-Path $buildDir 'llvm.zip'
    Invoke-WebRequest -Uri $url -OutFile $zipPath
    # Extract only LLVM-C.lib and LLVM-C.dll; the full archive is huge.
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
    try {
        foreach ($want in @(
            @{ entry = 'llvm/lib/LLVM-C.lib'; out = (Join-Path $llvmLib 'LLVM-C.lib') },
            @{ entry = 'llvm/bin/LLVM-C.dll'; out = (Join-Path $llvmBin 'LLVM-C.dll') }
        )) {
            $e = $zip.GetEntry($want.entry)
            if (-not $e) { throw "missing $($want.entry) in LLVM archive" }
            [System.IO.Compression.ZipFileExtensions]::ExtractToFile($e, $want.out, $true)
        }
    } finally { $zip.Dispose() }
    Remove-Item $zipPath
}

# --- Configure LLVM for the linker -------------------------------------------
# The self-host compiler declares its backend as `libLLVM-20.so` (see
# compiler/codegen/LLVMApi.fly); on Windows the toolchain maps that to the link
# name `LLVM-20.lib`. Alias LLVM-C.lib -> LLVM-20.lib: it is the import lib for
# LLVM-C.dll, which (in the fly-lang build) exports every symbol the generated
# code references, incl. the per-target LLVMInitialize* functions. A copy avoids
# symlink privilege gotchas.
Copy-Item "$llvmLib\LLVM-C.lib" "$llvmLib\LLVM-20.lib" -Force

# === fly bootstrap ===========================================================

# --- Download fly binary ------------------------------------------------------
# Skip if already present (local convenience; a fresh CI runner never has it).
if (-not (Test-Path $flyExe)) {
    $url = "https://github.com/fly-lang/fly/releases/download/v$FLY_VERSION/fly-$FLY_VERSION-win-x64.zip"
    New-Item -ItemType Directory -Force $buildDir | Out-Null
    $zipPath = Join-Path $buildDir 'fly.zip'
    Invoke-WebRequest -Uri $url -OutFile $zipPath
    Expand-Archive $zipPath -DestinationPath $flyDir -Force
    Remove-Item $zipPath
}

# === Environment =============================================================
# The toolchain propagates %LIB% as /libpath: entries to lld-link, so put the
# LLVM lib dir on LIB; put its bin on PATH so LLVM-C.dll loads at runtime.
#
# Use an absolute $FLY path (not a bare `fly` on PATH): released compilers derive
# their stdlib dir from the executable path, and a bare name resolved from a cwd
# containing this `fly\` directory breaks that lookup. build_compiler.ps1 /
# test_compiler.ps1 honour $FLY.
if ($env:GITHUB_ENV) {
    "LIB=$llvmLib;$env:LIB" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
    "FLY=$flyExe"           | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
    $llvmBin | Out-File -FilePath $env:GITHUB_PATH -Append -Encoding utf8
    $flyBin  | Out-File -FilePath $env:GITHUB_PATH -Append -Encoding utf8
} else {
    $env:LIB  = "$llvmLib;$env:LIB"
    $env:FLY  = $flyExe
    $env:PATH = "$llvmBin;$flyBin;$env:PATH"
    Write-Host "Prerequisites configured for this session:"
    Write-Host "  LLVM $LLVM_VERSION  -> LIB += $llvmLib ; PATH += $llvmBin"
    Write-Host "  fly  $FLY_VERSION   -> FLY  = $flyExe ; PATH += $flyBin"
    Write-Host "Note: dot-source this script (. .\ci\windows\install_prerequisites.ps1) for LIB/PATH/FLY to persist in your shell before running .\ci\windows\build_compiler.ps1"
}
