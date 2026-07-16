# -----------------------------------------------------------------------------
# stage0.ps1 - set up stage 0 of the staged bootstrap on Windows:
#
#   * the fork LLVM pieces (fly-lang/llvm-project release) -> build\llvm
#     LLVM-C.lib (link import lib) + LLVM-C.dll (runtime) + lld-link.exe (the
#     COFF linker the release bundles). No package manager; the MSVC toolchain
#     + Windows SDK come from ilammy/msvc-dev-cmd in the workflow.
#   * the bootstrap `fly` 0.13.8 release -> build\stage0 (bin\ + precompiled lib\)
#
# stage1.ps1 / stage2.ps1 build on top of these (see the stage map in stage1.ps1).
#
# Local use: dot-source it so LIB/PATH persist in your shell:
#     . .\ci\windows\stage0.ps1
#     .\ci\windows\stage1.ps1; .\ci\windows\stage2.ps1
#
# In CI ($GITHUB_ENV set) it appends to $GITHUB_ENV / $GITHUB_PATH instead of the
# process environment - auto-detected below.
# -----------------------------------------------------------------------------

$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\gnu_common.ps1"        # $FLY_WIN_TARGET, sysroot paths, Install-LdLld, MINGW_VERSION

# LLVM the self-host compiler links against: the project's own LLVM build
# (fly-lang/llvm-project release) rather than the stock LLVM installer. Only that
# build ships an LLVM-C.dll exporting the per-target LLVMInitialize* symbols the
# generated code references (the official/choco LLVM-C.dll omits them).
#
# The literals below are the source of truth; CI may override via env so this
# stays in sync with the cache key.
$LLVM_VERSION = if ($env:LLVM_VERSION) { $env:LLVM_VERSION } else { "20.1.8" }
$FLY_VERSION  = if ($env:FLY_VERSION)  { $env:FLY_VERSION }  else { "0.13.8" }

# Resolve everything against the PROJECT ROOT (this script lives in ci\windows\,
# two levels down) so the downloads land next to the build regardless of the
# caller's cwd, and without changing it (this script is dot-sourced locally). All
# prerequisites go under build\: LLVM in build\llvm, the bootstrap in build\stage0.
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$buildDir = Join-Path $repoRoot 'build'
$llvmLib  = Join-Path $buildDir 'llvm\lib'
$llvmBin  = Join-Path $buildDir 'llvm\bin'
$flyDir   = Join-Path $buildDir 'stage0'
$flyBin   = Join-Path $flyDir 'bin'
$flyExe   = Join-Path $flyBin 'fly.exe'

# === LLVM ====================================================================

# --- Download LLVM (fly-lang build) ------------------------------------------
# From the ~900 MB fork LLVM artifact we need only three files: LLVM-C.lib (the
# link import lib), LLVM-C.dll (loaded at runtime), and lld-link.exe (the COFF
# linker the self-contained release bundles). Skip the download when all three
# are present (the local equivalent of a CI cache hit).
if (-not (Test-Path "$llvmLib\LLVM-C.lib") -or -not (Test-Path "$llvmBin\LLVM-C.dll") -or -not (Test-Path "$llvmBin\lld-link.exe")) {
    $url = "https://github.com/fly-lang/llvm-project/releases/download/v$LLVM_VERSION-win-x64/llvm-$LLVM_VERSION-win-x64.zip"
    New-Item -ItemType Directory -Force $llvmLib, $llvmBin | Out-Null
    $zipPath = Join-Path $buildDir 'llvm.zip'
    Invoke-WebRequest -Uri $url -OutFile $zipPath
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
    try {
        foreach ($want in @(
            @{ entry = 'llvm/lib/LLVM-C.lib';   out = (Join-Path $llvmLib 'LLVM-C.lib') },
            @{ entry = 'llvm/bin/LLVM-C.dll';   out = (Join-Path $llvmBin 'LLVM-C.dll') },
            @{ entry = 'llvm/bin/lld-link.exe'; out = (Join-Path $llvmBin 'lld-link.exe') }
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
# compiler/lib/codegen/LLVMApi.fly); on Windows the toolchain maps that to the link
# name `LLVM-20.lib`. Alias LLVM-C.lib -> LLVM-20.lib: it is the import lib for
# LLVM-C.dll, which (in the fly-lang build) exports every symbol the generated
# code references. A copy avoids symlink privilege gotchas.
Copy-Item "$llvmLib\LLVM-C.lib" "$llvmLib\LLVM-20.lib" -Force

# === mingw / UCRT sysroot (mstorsjo/llvm-mingw) ==============================
# The self-contained Windows toolchain links user programs AND fly.exe itself
# against llvm-mingw's UCRT import libs + CRT startup objects + compiler-rt
# builtins — NO Visual Studio, NO Windows SDK. From the ~190 MB llvm-mingw release
# we keep only x86_64-w64-mingw32/lib/ (~62 MB) + libclang_rt.builtins-x86_64.a,
# staged under build\mingw (cache it in CI keyed on $MINGW_VERSION). The linker is
# the fork's own lld invoked as ld.lld (GNU flavour) — provisioned by Install-LdLld,
# no extra download. Set $env:MINGW_ZIP to a local zip to skip the download.
if (-not (Test-MingwSysroot)) {
    New-Item -ItemType Directory -Force $script:GNU_sysrootLib, $script:GNU_builtinsDir | Out-Null
    $mzip = if ($env:MINGW_ZIP) { $env:MINGW_ZIP } else {
        $u = "https://github.com/mstorsjo/llvm-mingw/releases/download/$script:MINGW_VERSION/llvm-mingw-$script:MINGW_VERSION-ucrt-x86_64.zip"
        $z = Join-Path $buildDir 'mingw.zip'
        Invoke-WebRequest -Uri $u -OutFile $z
        $z
    }
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $mz = [System.IO.Compression.ZipFile]::OpenRead($mzip)
    try {
        foreach ($e in $mz.Entries) {
            if ($e.FullName.EndsWith('/')) { continue }
            if ($e.FullName -match '/x86_64-w64-mingw32/lib/(.+)$') {
                $dest = Join-Path $script:GNU_sysrootLib $matches[1]
                $dd = Split-Path $dest -Parent
                if (-not (Test-Path $dd)) { New-Item -ItemType Directory -Force $dd | Out-Null }
                [System.IO.Compression.ZipFileExtensions]::ExtractToFile($e, $dest, $true)
            } elseif ($e.FullName -match '/lib/clang/[0-9]+/lib/windows/libclang_rt\.builtins-x86_64\.a$') {
                [System.IO.Compression.ZipFileExtensions]::ExtractToFile($e, $script:GNU_builtinsLib, $true)
            }
        }
    } finally { $mz.Dispose() }
    if (-not $env:MINGW_ZIP -and (Test-Path (Join-Path $buildDir 'mingw.zip'))) { Remove-Item (Join-Path $buildDir 'mingw.zip') }
}
Install-LdLld                                  # build\llvm\bin\ld.lld.exe (fork lld, GNU flavour)

# === fly bootstrap (stage 0) =================================================

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
# The build/test scripts derive their compiler from $env:STAGE (see stage1.ps1 /
# stage2.ps1) - no $FLY export needed.
if ($env:GITHUB_ENV) {
    "LIB=$llvmLib;$env:LIB" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
    $llvmBin | Out-File -FilePath $env:GITHUB_PATH -Append -Encoding utf8
} else {
    $env:LIB  = "$llvmLib;$env:LIB"
    $env:PATH = "$llvmBin;$env:PATH"
    Write-Host "stage0 ready:"
    Write-Host "  fly  $FLY_VERSION   -> $flyExe"
    Write-Host "  LLVM $LLVM_VERSION  -> LIB += $llvmLib ; PATH += $llvmBin"
}
