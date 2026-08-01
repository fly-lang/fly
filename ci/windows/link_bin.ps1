# -----------------------------------------------------------------------------
# link_bin.ps1 - link ONE Fly object into an executable with the fork's ld.lld
# (GNU flavour) against the bundled mingw/UCRT sysroot, std and runtime.
#
# Extracted from link_fly.ps1 so a SECOND executable (fly-lsp) can be linked the
# same way. link_fly.ps1 is now a thin caller; everything platform-specific lives
# here and in gnu_common.ps1.
#
# Weak symbols (generic specializations, vtables, init_ctors) carry a COMDAT
# (selection Any), so ld.lld dedups them natively - no --allow-multiple-definition.
# MONOLITHIC: the compiler is compiled from source INTO the object, never linked
# as a separate archive - a compiler static lib let the linker COMDAT-dedup its
# generic instantiations and produced the `fly build` use-after-free.
#
#   -Obj      the .o to link
#   -Out      the .exe path to produce
#   -WithLLVM link LLVM-C.lib too (needed by anything reaching CodeGen)
# -----------------------------------------------------------------------------
param(
    [Parameter(Mandatory = $true)][string]$Obj,
    [Parameter(Mandatory = $true)][string]$Out,
    [switch]$WithLLVM
)
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
$LIB = "build/stage$STAGE/lib"

if (-not (Test-Path $Obj)) { Write-Host "error: $Obj missing."; exit 1 }
if (-not (Test-Path "$LIB/fly_std_lib.lib") -or -not (Test-Path "$LIB/fly_runtime_lib.lib")) {
    Write-Host "error: std/runtime missing in $LIB - run build_runtime.ps1 + build_std.ps1 first."; exit 1
}

Assert-LdLld                      # build\llvm\bin\ld.lld.exe (extracted by stage0 from the LLVM artifact)
if (-not (Test-MingwSysroot)) {
    Write-Host "error: mingw/UCRT sysroot missing under build\mingw - run ci\windows\stage0.ps1."; exit 1
}

# The TLS stub is ALWAYS linked: std/lib/net/tls.fly references the tls_* symbols
# unconditionally, so without it every link fails with "undefined symbol:
# tls_available". Windows has no real backend yet, so the stub is all there is —
# tlsAvailable() reports false, honestly.
$objs = @($Obj, "$LIB/fly_std_lib.lib", "$LIB/fly_runtime_lib.lib")
if (Test-Path "$LIB/fly_tls_stub.lib") { $objs += "$LIB/fly_tls_stub.lib" }
if ($WithLLVM) {
    $llvmRoot = if (Test-Path 'build/llvm') { (Resolve-Path 'build/llvm').Path } else { $null }
    if (-not $llvmRoot) { Write-Host "error: build\llvm (fork LLVM) not found - run ci\windows\stage0.ps1."; exit 1 }
    $llvmC = Join-Path $llvmRoot 'lib\LLVM-C.lib'
    if (-not (Test-Path $llvmC)) { Write-Host "error: $llvmC not found."; exit 1 }
    $objs += $llvmC
}

New-Item -ItemType Directory -Force (Split-Path -Parent $Out) | Out-Null
$parts = Get-MingwLinkParts
$args = @('-m', 'i386pep') + $parts.LibDirs + $parts.Pre + $objs + $parts.Post + @('-o', $Out)
& $script:GNU_ldLld @args
if ($LASTEXITCODE -ne 0) { throw "link of $Out failed (exit $LASTEXITCODE)" }
exit 0
