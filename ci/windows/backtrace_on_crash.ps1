# -----------------------------------------------------------------------------
# backtrace_on_crash.ps1 - Windows equivalent of build-linux.yml's gdb
# "Backtrace on Crash". A self-host suite compile that faults (heap corruption
# 0xC0000374 / access violation 0xC0000005) otherwise leaves only a bare exit
# code. Here we re-run crash-prone suites under cdb and symbolize each fly.exe
# frame with llvm-symbolizer against the DWARF the debug build embeds
# (FLY_DEBUG_SYMBOLS=1) - turning `fly+0x1234` into
# `Fn @ compiler/lib/.../File.fly:NN`.
#
# TWO capture modes:
#   * PAGEHEAP (admin, e.g. the CI runner): Full Page Heap (gflags /p /enable
#     fly.exe /full) puts each allocation on its own page with a trailing guard
#     page, so a buffer OVERFLOW faults on the exact WRITE instruction. The top
#     fly frame is then the DEFECT itself, not the later free that noticed it.
#   * _NO_DEBUG_HEAP (no admin): a debugger normally forces the NT debug heap,
#     whose validation MASKS the corruption (the compile takes a non-fatal error
#     path and never faults). _NO_DEBUG_HEAP=1 keeps the normal heap under cdb so
#     the fault fires as second chance - but only at the free that DETECTS the
#     corruption, one step removed from the overflow.
#
# NOTE: the compiler ships as one `--lib` unit, so its DWARF is a single compile
# unit - FUNCTION names are reliable, the file:line can point at the wrong .fly.
# Non-fatal: always exits 0 (diagnostic on an already-failed step).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Continue'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$FLY = if ($env:FLY) { $env:FLY } else { 'build/stage2/bin/fly.exe' }
if (-not (Test-Path $FLY -PathType Leaf)) { Write-Host "backtrace: '$FLY' not found; nothing to do."; exit 0 }
$FLY = (Resolve-Path $FLY).Path
$IMG = Split-Path $FLY -Leaf   # 'fly.exe' - the image name Page Heap keys on
$STD = 'std/lib'

function Find-Tool($name, $extraPaths) {
    $c = (Get-Command $name -ErrorAction SilentlyContinue).Source
    if ($c) { return $c }
    foreach ($p in $extraPaths) {
        $hit = Get-ChildItem $p -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($hit) { return $hit.FullName }
    }
    return $null
}

$sdkDbg = "${env:ProgramFiles(x86)}\Windows Kits\10\Debuggers\x64"
$cdb = Find-Tool 'cdb.exe' @("$sdkDbg\cdb.exe")
if (-not $cdb) {
    Write-Host "backtrace: installing WinDbg (cdb) ..."
    choco install windbg -y --no-progress | Out-Null
    $cdb = Find-Tool 'cdb.exe' @("C:\Program Files*\Windows Kits\10\Debuggers\x64\cdb.exe")
}
$gflags = Find-Tool 'gflags.exe' @("$sdkDbg\gflags.exe", "C:\Program Files*\Windows Kits\10\Debuggers\x64\gflags.exe")
$sym = Find-Tool 'llvm-symbolizer.exe' @(
    "$env:VCToolsInstallDir\bin\Host*\x64\llvm-symbolizer.exe",
    "C:\Program Files\LLVM\bin\llvm-symbolizer.exe",
    "C:\Program Files (x86)\LLVM\bin\llvm-symbolizer.exe")
Write-Host "backtrace: fly=$FLY"
Write-Host "backtrace: cdb=$cdb ; gflags=$gflags ; llvm-symbolizer=$sym"
if (-not $cdb) { Write-Host "backtrace: cdb not found; skipping."; exit 0 }
if (-not $sym) { Write-Host "backtrace: llvm-symbolizer not found - frames will show fly+RVA only." }

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
        # Under Page Heap / Verifier cdb can't name the exe and prints it as
        # `image<base>+0xRVA` instead of `fly+0xRVA`; match both (the +0xRVA is
        # the module-relative address either way).
        if ($line -match '(?:fly|image[0-9a-fA-F]+)\+0x([0-9A-Fa-f]+)') {
            $rva = "0x$($matches[1])"
            $txt = "fly+$rva"
            if ($sym) {
                $r = & $sym --obj=$FLY --relative-address $rva 2>&1
                $fn  = ($r | Select-Object -First 1)
                $src = ($r | Select-Object -Skip 1 -First 1)
                if ($fn) { $txt = "{0,-58} {1}" -f $fn, $src }
            }
            if ($txt -ne $prev) { Write-Host "    $txt"; $prev = $txt; $seen++ }
        }
    }
}

$OUT = 'build/test'; New-Item -ItemType Directory -Force $OUT | Out-Null
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

# Suites seen faulting in CI go first; then any others so a shifted crash set is
# still caught. Cap how many we dissect.
$prone = @('SemaNodeSuite','SymbolTableSuite','SemaTypeSuite','DiagnosticsNegativeSuite',
           'CodeGenStringSuite','CodeGenLLVMSuite','CodeGenDriverSuite','DiagnosticRenderSuite',
           'FrontendOptionsSuite','InputFileSuite')
$all = Get-ChildItem -Recurse -Filter *Suite.fly compiler/test
$ordered = @()
foreach ($n in $prone) { $ordered += ($all | Where-Object { $_.BaseName -eq $n }) }
$ordered += ($all | Where-Object { $prone -notcontains $_.BaseName } | Sort-Object FullName)

$MAX = 3
$captured = 0
try {
    foreach ($suite in $ordered) {
        if ($captured -ge $MAX) { break }
        $name = $suite.BaseName
        $logf = "$LOG/_$name.cdb.txt"   # NB: not $log - PS vars are case-insensitive ($LOG)
        # -logo writes cdb's own (newline-clean) session log; explicit arg array
        # keeps each token a distinct argv entry so cdb never folds fly's own
        # -o/-L into its option parsing. cdb's console goes to $null.
        $dbgArgs = $cdbHead + @('-logo',$logf,'-c',$cdbCmd, $FLY, $suite.FullName,
                                '--test','-o',"t_$name",'--out-dir',$OUT,'-L',$STD)
        & $cdb @dbgArgs *> $null
        $out = Get-Content $logf -ErrorAction SilentlyContinue
        # Skip a run where cdb mis-launched fly (rare arg race) - not a real fault.
        if ($out | Select-String 'multiple input files require') { continue }
        $evt = $out | Select-String 'code c00003|code c00004|Access violation|Critical error detected|VERIFIER STOP' | Select-Object -First 1
        if (-not $evt) { continue }   # took the non-fatal error path this run

        $captured++
        Write-Host "`n============================================================"
        Write-Host " CRASH in $name  [$(if ($pageHeap) { 'PageHeap/Verifier' } else { 'free-detect site' })]"
        Write-Host "============================================================"
        ($out | Select-String 'VERIFIER STOP|corrupted start stamp|block already freed|Block size|ExceptionCode:|Critical error detected|Access violation') |
            Select-Object -First 4 |
            ForEach-Object { $t = $_.Line.Trim(); if ($t.Length -gt 160) { $t = $t.Substring(0,160) }; "  $t" }
        Write-Host "  --- fly frames (symbolized; function reliable, file:line approximate) ---"
        Symbolize-Stack $out
    }
} finally {
    if ($pageHeap) { & $gflags /p /disable $IMG 2>&1 | Out-Null }  # always un-set the IFEO key
}

if ($captured -eq 0) {
    Write-Host "backtrace: no suite faulted under the debugger this run (heap-layout dependent)."
}
exit 0
