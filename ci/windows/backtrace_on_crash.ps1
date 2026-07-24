# -----------------------------------------------------------------------------
# backtrace_on_crash.ps1 - Windows equivalent of build-linux.yml's gdb
# "Backtrace on Crash". Given a TARGET exe that faults, run it under cdb, capture
# the fault, and symbolize each frame with llvm-symbolizer. The self-host emits no
# DWARF, so RVAs resolve to FUNCTION names (from the COFF symbol table), not
# file:line.
#
# Usage (from ci/windows or the repo root):
#   pwsh ci/windows/backtrace_on_crash.ps1 <target.exe> [args...]
#   $env:CRASH_EXE = 'build/test/SomeSuite.exe'; pwsh ci/windows/backtrace_on_crash.ps1
# With no target it prints usage and exits 0 (it is a diagnostic on an
# already-failed step, so it never fails the build).
#
# Requires (install with winget; run from an ADMIN shell for Page Heap):
#   winget install --id Microsoft.WinDbg -e                   # cdb / gflags
#   winget install --id LLVM.LLVM -e                          # llvm-symbolizer
#   winget install --id Microsoft.WindowsSDK.10.0.26100 -e    # gflags (Page Heap; admin to use)
#
# TWO capture modes:
#   * PAGEHEAP (admin, e.g. the CI runner): Full Page Heap (gflags /p /enable
#     <img> /full) puts each allocation on its own page with a trailing guard
#     page, so a buffer OVERFLOW faults on the exact WRITE instruction. The top
#     target frame is then the DEFECT itself, not the later free that noticed it.
#   * _NO_DEBUG_HEAP (no admin): a debugger normally forces the NT debug heap,
#     whose validation MASKS the corruption (the run takes a non-fatal error path
#     and never faults). _NO_DEBUG_HEAP=1 keeps the normal heap under cdb so the
#     fault fires as second chance - but only at the free that DETECTS the
#     corruption, one step removed from the overflow.
#
# NOTE: the compiler/runtime ship as `--lib` units with no DWARF, so FUNCTION
# names are reliable but file:line is absent.
# -----------------------------------------------------------------------------
param(
    [string]$Target = $env:CRASH_EXE,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$TargetArgs = @()
)
$ErrorActionPreference = 'Continue'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

if (-not $Target) {
    Write-Host "backtrace: no target exe given. Usage: backtrace_on_crash.ps1 <target.exe> [args...]"
    Write-Host "          (or set `$env:CRASH_EXE). Nothing to do."
    exit 0
}
if (-not (Test-Path $Target -PathType Leaf)) {
    Write-Host "backtrace: target '$Target' not found; nothing to do."
    exit 0
}
$TARGET = (Resolve-Path $Target).Path
$IMG = Split-Path $TARGET -Leaf                              # 'foo.exe' - the image Page Heap keys on
$MODNAME = [IO.Path]::GetFileNameWithoutExtension($TARGET)   # 'foo' - the module prefix cdb prints on frames

# A fly-compiled target loads LLVM-C.dll (pulled via fly.compiler.*). Put
# build/llvm/bin on PATH so both it and cdb launching the target find the DLL
# (else Windows pops a STATUS_DLL_NOT_FOUND dialog and the run hangs). Mirrors
# test_compiler.ps1.
$llvmBin = Join-Path (Get-Location) 'build\llvm\bin'
if (Test-Path (Join-Path $llvmBin 'LLVM-C.dll')) { $env:PATH = "$llvmBin;$env:PATH" }

function Find-Tool($name, $extraPaths) {
    $c = (Get-Command $name -ErrorAction SilentlyContinue).Source
    if ($c) { return $c }
    foreach ($p in $extraPaths) {
        # -Force so the ACL-restricted %ProgramFiles%\WindowsApps (winget's WinDbg
        # MSIX) is enumerated too.
        $hit = Get-ChildItem $p -Force -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($hit) { return $hit.FullName }
    }
    return $null
}

# winget's Microsoft.WinDbg ships cdb/gflags under its MSIX package's amd64 dir.
$winDbgApp = 'C:\Program Files\WindowsApps\Microsoft.WinDbg_*_x64__*\amd64'

$sdkDbg = "${env:ProgramFiles(x86)}\Windows Kits\10\Debuggers\x64"
$cdb = Find-Tool 'cdb.exe' @("$sdkDbg\cdb.exe", "$winDbgApp\cdb.exe")
if (-not $cdb) {
    # cdb missing — install WinDbg via winget, but only if winget itself is present.
    $winget = (Get-Command winget.exe -ErrorAction SilentlyContinue).Source
    if ($winget) {
        Write-Host "backtrace: cdb not found; installing WinDbg via winget ..."
        & $winget install --id Microsoft.WinDbg -e --silent `
            --accept-source-agreements --accept-package-agreements 2>&1 | Out-Null
        # cdb from winget's Microsoft.WinDbg lands in its WindowsApps MSIX dir, not the SDK.
        $cdb = Find-Tool 'cdb.exe' @(
            "$sdkDbg\cdb.exe",
            "$winDbgApp\cdb.exe",
            "C:\Program Files*\Windows Kits\10\Debuggers\x64\cdb.exe")
    } else {
        Write-Host "backtrace: cdb not found and winget unavailable; skipping install."
    }
}
$gflags = Find-Tool 'gflags.exe' @("$sdkDbg\gflags.exe", "$winDbgApp\gflags.exe", "C:\Program Files*\Windows Kits\10\Debuggers\x64\gflags.exe")
$sym = Find-Tool 'llvm-symbolizer.exe' @(
    "$env:VCToolsInstallDir\bin\Host*\x64\llvm-symbolizer.exe",
    "C:\Program Files\LLVM\bin\llvm-symbolizer.exe",
    "C:\Program Files (x86)\LLVM\bin\llvm-symbolizer.exe")
Write-Host "backtrace: target=$TARGET"
Write-Host "backtrace: cdb=$cdb ; gflags=$gflags ; llvm-symbolizer=$sym"
if (-not $cdb) { Write-Host "backtrace: cdb not found; skipping."; exit 0 }
if (-not $sym) { Write-Host "backtrace: llvm-symbolizer not found - frames will show $MODNAME+RVA only." }

# --- Pick the capture mode ---------------------------------------------------
$isAdmin = $false
try {
    $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
              ).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)
} catch {}
$env:_NO_DEBUG_HEAP = '1'
$pageHeap = $false
if ($isAdmin -and $gflags) {
    & $gflags /p /enable $IMG /full 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) {
        $pageHeap = $true
        Write-Host "backtrace: Full Page Heap ENABLED for $IMG -> fault lands on the overflowing WRITE (exact line)."
    }
}
if (-not $pageHeap) {
    Write-Host "backtrace: Page Heap unavailable (admin=$isAdmin, gflags=$([bool]$gflags)); using _NO_DEBUG_HEAP -> reports the FREE that DETECTS the corruption."
}

function Symbolize-Stack($lines) {
    $seen = 0; $prev = ''
    foreach ($line in $lines) {
        if ($seen -ge 30) { break }
        # cdb `kb`/`kn` frames end with the Call Site column. For a DWARF-less
        # self-host image cdb can't name the module, so it prints the bare module
        # RVA there (e.g. `0x1b870`), or `<module>+0xRVA` when it can, or
        # `image<base>+0xRVA` under Page Heap. Take the trailing Call Site token
        # (after the last ':') and match those three forms; system frames like
        # `KERNEL32!..+0x17` are already named by cdb and are skipped here.
        $callsite = (($line -split ':')[-1]).Trim()
        $rva = $null
        if ($callsite -match "^(?:$([regex]::Escape($MODNAME))\+)?0x([0-9A-Fa-f]+)$") {
            $rva = "0x$($matches[1])"
        } elseif ($callsite -match '^image[0-9a-fA-F]+\+0x([0-9A-Fa-f]+)$') {
            $rva = "0x$($matches[1])"
        }
        if ($rva) {
            $txt = "$MODNAME+$rva"
            if ($sym) {
                $r = & $sym --obj=$TARGET --relative-address $rva 2>&1
                $fn  = ($r | Select-Object -First 1)
                $src = ($r | Select-Object -Skip 1 -First 1)
                if ($fn) { $txt = "{0,-58} {1}" -f $fn, $src }
            }
            if ($txt -ne $prev) { Write-Host "    $txt"; $prev = $txt; $seen++ }
        }
    }
}

$LOG = 'build/crashdumps'; New-Item -ItemType Directory -Force $LOG | Out-Null

# cdb command + flags depend on the mode:
#  * PageHeap: the fault is a noncontinuable Application-Verifier STOP (int 3).
#    Do NOT pass -g (it would ignore the int 3 and let the process terminate
#    before we can look). `!avrf` dumps the offending block's allocation stack
#    AND its free stack(s) - a double-free / use-after-free shows the two frees.
#  * free-detect: the fault is a 2nd-chance heap-corruption exception; -g -G is
#    fine and `kn` gives the crashing thread.
if ($pageHeap) {
    $cdbHead = @('-G')
    $cdbCmd  = "g; .echo ===FLYCRASH===; .lastevent; .echo ===AVRF===; !avrf; .echo ===STACK===; kb 60; q"
} else {
    $cdbHead = @('-g','-G')
    $cdbCmd  = "g; .echo ===FLYCRASH===; .lastevent; .echo ===EXR===; .exr -1; .echo ===STACK===; kn 100; q"
}

# Run the TARGET exe under cdb. -logo writes cdb's own (newline-clean) session
# log; cdb's console goes to $null.
$logf = "$LOG/$MODNAME.cdb.txt"
$captured = 0
try {
    $dbgArgs = $cdbHead + @('-logo',$logf,'-c',$cdbCmd, $TARGET) + $TargetArgs
    & $cdb @dbgArgs *> $null
    $out = Get-Content $logf -ErrorAction SilentlyContinue
    $evt = $out | Select-String 'code c00003|code c00004|Access violation|Critical error detected|VERIFIER STOP' | Select-Object -First 1
    if ($evt) {
        $captured = 1
        Write-Host "`n============================================================"
        Write-Host " CRASH in $IMG  [$(if ($pageHeap) { 'PageHeap/Verifier' } else { 'fault / free-detect site' })]"
        Write-Host "============================================================"
        ($out | Select-String 'VERIFIER STOP|corrupted start stamp|block already freed|Block size|ExceptionCode:|Critical error detected|Access violation') |
            Select-Object -First 4 |
            ForEach-Object { $t = $_.Line.Trim(); if ($t.Length -gt 160) { $t = $t.Substring(0,160) }; "  $t" }
        Write-Host "  --- frames (symbolized to function names; self-host emits no DWARF, so no file:line) ---"
        Symbolize-Stack $out
    }
} finally {
    if ($pageHeap) { & $gflags /p /disable $IMG 2>&1 | Out-Null }  # always un-set the IFEO key
}

if ($captured -eq 0) {
    Write-Host "backtrace: $IMG did NOT fault under the debugger this run (heap-layout dependent);"
    Write-Host "          cdb's debug heap can mask the corruption. Re-run, or from an ADMIN shell for Full"
    Write-Host "          Page Heap (needs gflags from the Windows SDK)."
}
exit 0
