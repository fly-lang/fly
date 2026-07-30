# -----------------------------------------------------------------------------
# build_runtime.ps1 - build the Fly runtime for Windows into build\stage$STAGE\lib.
# PowerShell counterpart of ci/linux/build_runtime.sh; see stage1.ps1 for the
# stage map.
#
# Windows targets x86_64-w64-windows-gnu (llvm-mingw / UCRT), so the runtime is
# COMPILED FROM runtime/lib/RuntimeWindows.fly with the stage0 reference compiler
# for that triple — NOT copied from the MSVC seed. stage0 emits the runtime as
# flat C-ABI, unmangled symbols with NO MSVCRT/OLDNAMES directive (CRT-neutral),
# which is exactly what links against the mingw/UCRT sysroot. (The self-host
# compiler mis-emits these definitions with the ordinary Fly ABI — see the
# selfhost-runtime-cabi note — so the runtime is stage0-built at BOTH stages until
# that is fixed; STAGE=2 re-runs this same stage0 recompile.)
#
# Output: $LIB/fly_runtime_lib.lib + runtime.fly.h + llvm.fly.h.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

# -- Stage plumbing: pick the out dir from $STAGE. -----------------------------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
$LIB = "build/stage$STAGE/lib"
New-Item -ItemType Directory -Force $LIB, build/stage1/bin | Out-Null

function Assert-LastExit($what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed (exit $LASTEXITCODE)" }
}
function Split-GenericClosers($file) {
    $c = Get-Content $file -Raw
    while ($c -match '>>') { $c = $c -replace '>>', '> >' }
    Set-Content $file $c -NoNewline
}

# -- Compiler: STAGE 1 uses the stage0 reference (hardlinked as fly0.exe so its
#    <exe>\..\lib discovery serves build\stage1\lib); STAGE 2 SELF-HOSTS with the
#    stage1 fly.exe. The self-host emits `fly.runtime` defs with the correct flat
#    C-ABI (unmangled wrappers — see CodeGenModule.emitRuntimeCABIWrapper), so its
#    runtime archive is link-compatible with the reference's. ---------------------
if ($STAGE -eq '1') {
    if (-not (Test-Path 'build/stage0/bin/fly.exe' -PathType Leaf)) {
        Write-Host "error: stage0 compiler missing - run ci\windows\stage0.ps1 first."; exit 1
    }
    $fly0 = (Resolve-Path 'build/stage0/bin/fly.exe').Path
    $FLY = 'build/stage1/bin/fly0.exe'
    Remove-Item $FLY -Force -ErrorAction SilentlyContinue
    try { New-Item -ItemType HardLink -Path $FLY -Target $fly0 -ErrorAction Stop | Out-Null }
    catch { Copy-Item $fly0 $FLY -Force }
} else {
    $FLY = 'build/stage1/bin/fly.exe'
    if (-not (Test-Path $FLY -PathType Leaf)) {
        Write-Host "error: stage1 fly '$FLY' not found - run ci\windows\stage1.ps1 first."; exit 1
    }
}

# llvm.fly.h (the fly.llvm bridge header) is a build input, not source; it ships
# with the bootstrap lib. Stage it so the runtime (and later std/compiler) resolve
# fly.llvm.* against it.
$llvmHdr = 'build/stage0/lib/llvm.fly.h'
if (-not (Test-Path $llvmHdr)) { Write-Host "error: $llvmHdr missing - run stage0.ps1 first."; exit 1 }
Copy-Item $llvmHdr "$LIB/llvm.fly.h" -Force

# -- Compile RuntimeWindows.fly → fly_runtime_lib.lib (gnu triple). ------------
$T = 'build/tmp_runtime'
Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $T | Out-Null
$DBG = @(); if ($env:FLY_DEBUG_SYMBOLS -eq '1') { $DBG += '--debug-symbols' }
Write-Host "stage${STAGE}: compiling runtime/lib/RuntimeWindows.fly (codegen gnu, link mingw) ...$(if ($DBG) { ' (+debug-symbols)' })"
# DIRECTORY CLI: --lib compiles the whole --src-dir; stage the one runtime
# source into the temp dir so exactly that file is the library (its
# same-namespace siblings RuntimeLinux/RuntimeMacos must stay out).
Copy-Item runtime/lib/RuntimeWindows.fly "$T/RuntimeWindows.fly" -Force
& $FLY --lib @DBG @FLY_TARGET_ARGS -o "$T/fly_runtime_lib" -L $LIB --src-dir $T
Assert-LastExit 'runtime --lib build'
# stage0 --lib emits a `.a`/`.lib` ARCHIVE; the self-host emits ONE merged OBJECT
# (`fly_runtime_lib`, no extension). Both go to `fly_runtime_lib.lib` — ld.lld links
# a bare COFF object or an archive by content, not by the `.lib` name.
$emitted = @("$T/fly_runtime_lib.lib", "$T/fly_runtime_lib.a", "$T/fly_runtime_lib") | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $emitted) { Write-Host "error: runtime library not emitted."; exit 1 }
Move-Item $emitted "$LIB/fly_runtime_lib.lib" -Force

# runtime.fly.h — std/compiler compile against this. The compiler names the header
# after the source (RuntimeWindows.fly.h); canonicalise to runtime.fly.h.
$genHdr = "$T/RuntimeWindows.fly.h"
if (Test-Path $genHdr) {
    Split-GenericClosers $genHdr
    Copy-Item $genHdr "$LIB/runtime.fly.h" -Force
} elseif ($STAGE -eq '1') {
    Write-Host "error: runtime header not emitted."; exit 1
} else {
    # STAGE 2: std/compiler ship from stage1 and already carry runtime.fly.h; a
    # self-host that doesn't re-emit it is fine — keep the stage1 header if present.
    if (-not (Test-Path "$LIB/runtime.fly.h") -and (Test-Path 'build/stage1/lib/runtime.fly.h')) {
        Copy-Item 'build/stage1/lib/runtime.fly.h' "$LIB/runtime.fly.h" -Force
    }
}

# -- TLS stub -> fly_tls_stub.lib + TlsStub.fly.h ------------------------------
#
# The stub is built on EVERY platform and is ALWAYS linked. Its header carries the
# canonical tls_* declarations that std/lib/net/tls.fly compiles against, so that
# module builds even where there is no TLS backend at all (Windows today).
#
# It lives in a SEPARATE archive from fly_runtime_lib on purpose: a real backend
# (runtime/lib/TlsSchannel.fly here, TlsOpenSSL.fly on Linux) defines the same
# seven symbols and is linked AHEAD of this one under --tls, so archive lazy
# extraction never pulls the stub member. Putting either half inside the main
# runtime archive would defeat that, since the main member is always extracted.
#
# The header keeps its generated name (TlsStub.fly.h): loadHeadersFromDir parses
# every *.fly.h in the lib dir regardless of name, and a namespace may legitimately
# be split across two headers — probed before this was written.
$T2 = 'build/tmp_tls'
Remove-Item $T2 -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $T2 | Out-Null
Write-Host "stage${STAGE}: compiling runtime/lib/TlsStub.fly ..."
Copy-Item runtime/lib/TlsStub.fly "$T2/TlsStub.fly" -Force
& $FLY --lib @DBG @FLY_TARGET_ARGS -o "$T2/fly_tls_stub" -L $LIB --src-dir $T2
Assert-LastExit 'tls stub --lib build'
$emittedTls = @("$T2/fly_tls_stub.lib", "$T2/fly_tls_stub.a", "$T2/fly_tls_stub") | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $emittedTls) { Write-Host "error: tls stub library not emitted."; exit 1 }
Move-Item $emittedTls "$LIB/fly_tls_stub.lib" -Force
$tlsHdr = "$T2/TlsStub.fly.h"
if (Test-Path $tlsHdr) {
    Split-GenericClosers $tlsHdr
    Copy-Item $tlsHdr "$LIB/TlsStub.fly.h" -Force
} elseif (-not (Test-Path "$LIB/TlsStub.fly.h")) {
    Write-Host "error: tls stub header not emitted."; exit 1
}
Remove-Item $T2 -Recurse -Force -ErrorAction SilentlyContinue

# -- TLS backend -> fly_tls_lib.lib (Schannel, --tls only) ---------------------
#
# Schannel ships with Windows, so unlike OpenSSL there is no "is it installed?"
# question — but this archive is STILL gated behind --tls, for two reasons: the
# gate is what keeps the stub/real selection a pure link-order decision on every
# platform, and it keeps -lsecur32 -lcrypt32 off the command line of every hello
# world that never asked for TLS.
#
# The generated header is deliberately NOT copied: TlsStub.fly.h is the canonical
# declaration of the tls_* primitives, and emitting both would declare the same
# functions twice in namespace fly.runtime.
Remove-Item $T2 -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $T2 | Out-Null
Write-Host "stage${STAGE}: compiling runtime/lib/TlsSchannel.fly (real backend, --tls only) ..."
Copy-Item runtime/lib/TlsSchannel.fly "$T2/TlsSchannel.fly" -Force
& $FLY --lib @DBG @FLY_TARGET_ARGS -o "$T2/fly_tls_lib" -L $LIB --src-dir $T2
Assert-LastExit 'tls backend --lib build'
$emittedReal = @("$T2/fly_tls_lib.lib", "$T2/fly_tls_lib.a", "$T2/fly_tls_lib") | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $emittedReal) { Write-Host "error: tls backend library not emitted."; exit 1 }
Move-Item $emittedReal "$LIB/fly_tls_lib.lib" -Force
Remove-Item $T2 -Recurse -Force -ErrorAction SilentlyContinue

Remove-Item $T -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "stage${STAGE}: runtime -> $LIB/fly_runtime_lib.lib (+ runtime.fly.h, llvm.fly.h, fly_tls_stub.lib, fly_tls_lib.lib)"
exit 0
