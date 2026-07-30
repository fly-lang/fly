# -----------------------------------------------------------------------------
# gnu_common.ps1 - shared constants + helpers for the Windows GNU/UCRT (gnullvm)
# toolchain. Dot-sourced by the ci\windows\*.ps1 scripts.
#
# Fly targets x86_64-w64-windows-gnu (llvm-mingw / UCRT), NOT -msvc: the whole
# point is a self-contained toolchain that needs no Visual Studio. The pieces:
#   * the fork LLVM already ships lld (build\llvm\bin\lld-link.exe); the SAME
#     binary is the GNU-flavour linker when invoked as `ld.lld` (LLD self-selects
#     the flavour from argv[0]), so we provision build\llvm\bin\ld.lld.exe as a
#     copy of it - no extra download for the linker.
#   * the mingw import libs + CRT startup objects + compiler-rt builtins come
#     from mstorsjo/llvm-mingw, staged under build\mingw (fetched by stage0.ps1,
#     cached like the LLVM fetch). These are the CRT-neutral, redistributable
#     equivalents of the MSVC CRT + Windows SDK import libs.
# -----------------------------------------------------------------------------

# The GNU/UCRT triple used for LINKING (the mingw sysroot, ld.lld -m i386pep) and,
# when full-gnu CODEGEN is enabled, for compilation too.
$script:FLY_WIN_TARGET = 'x86_64-w64-windows-gnu'

# CODEGEN and LINK are both pure gnu/UCRT — no MSVC anywhere. Fly compiles with
# $FLY_WIN_TARGET (x86_64-w64-windows-gnu, Rust-gnullvm-style). LLVM's X86 backend
# then emits the GNU stack-probe `___chkstk_ms` (provided by compiler-rt builtins)
# and NO `_fltused` marker, so the mingw sysroot satisfies the link on its own —
# no CRT-glue shim is needed. The old MSVC codegen path (compile as
# x86_64-pc-windows-msvc), which DID need such a shim because it emits `__chkstk`
# + `_fltused`, has been removed: it was the last MSVC-ism in the pipeline.
# $FLY_TARGET_ARGS is spliced into each compile (@FLY_TARGET_ARGS) as an EXPLICIT
# --target so the build never silently depends on the driver's default.
$script:FLY_TARGET_ARGS = @('--target', $script:FLY_WIN_TARGET)

# llvm-mingw release the sysroot is cut from (mstorsjo/llvm-mingw). Keep in sync
# with the CI cache key. UCRT + x86_64.
$script:MINGW_VERSION = if ($env:MINGW_VERSION) { $env:MINGW_VERSION } else { '20260616' }

# Resolve paths against the project root (this script lives in ci\windows\).
$script:GNU_repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$script:GNU_buildDir = Join-Path $script:GNU_repoRoot 'build'
$script:GNU_llvmBin  = Join-Path $script:GNU_buildDir 'llvm\bin'
# Sysroot layout (populated by Install-MingwSysroot):
#   build\mingw\lib      - mingw import libs + crt2/crtbegin/crtend objects
#   build\mingw\builtins - libclang_rt.builtins-x86_64.a (compiler-rt)
$script:GNU_sysroot     = Join-Path $script:GNU_buildDir 'mingw'
$script:GNU_sysrootLib  = Join-Path $script:GNU_sysroot 'lib'
$script:GNU_builtinsDir = Join-Path $script:GNU_sysroot 'builtins'
$script:GNU_lldLink     = Join-Path $script:GNU_llvmBin 'lld-link.exe'
$script:GNU_ldLld       = Join-Path $script:GNU_llvmBin 'ld.lld.exe'
$script:GNU_builtinsLib = Join-Path $script:GNU_builtinsDir 'libclang_rt.builtins-x86_64.a'

# Ensure build\llvm\bin\ld.lld.exe exists (a copy of the fork lld-link.exe). The
# GNU-flavour driver is the same binary; the copy lets us fork it by that name.
function Install-LdLld {
    if (-not (Test-Path $script:GNU_lldLink)) {
        throw "gnu_common: $script:GNU_lldLink not found - run ci\windows\stage0.ps1 (LLVM fetch) first."
    }
    if (-not (Test-Path $script:GNU_ldLld)) {
        Copy-Item $script:GNU_lldLink $script:GNU_ldLld -Force
    }
}

# True when the sysroot is already staged (used to skip the fetch, CI-cache style).
function Test-MingwSysroot {
    return (Test-Path (Join-Path $script:GNU_sysrootLib 'crt2.o')) -and (Test-Path $script:GNU_builtinsLib)
}

# The mingw link inputs, in the order lld expects: startup objects, the program
# and libraries go in between (added by the caller), then the terminators. This
# returns @{ Pre = @(...); Post = @(...); LibDirs = @(...) } so callers can splice
# their own objects/libs in the middle. Mirrors the proven scratchpad link line.
function Get-MingwLinkParts {
    $L = $script:GNU_sysrootLib
    return @{
        LibDirs = @("-L$L", "-L$script:GNU_builtinsDir")
        Pre     = @((Join-Path $L 'crt2.o'), (Join-Path $L 'crtbegin.o'))
        # -lmingw32 brackets the builtins archive; the Win32 import libs follow.
        Post    = @(
            '-lmingw32', $script:GNU_builtinsLib, '-lmoldname', '-lmingwex', '-lmsvcrt',
            '-ladvapi32', '-lshell32', '-luser32', '-lkernel32', '-lntdll', '-lws2_32', '-lwinhttp', '-lsynchronization', '-lmingw32',
            (Join-Path $L 'crtend.o')
        )
    }
}
