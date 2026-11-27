# -----------------------------------------------------------------------------
# test_heap.ps1 - object heap guard (fly-lang/fly#235).
# Mirror of ci/linux/test_heap.sh.
#
# Every class object lives in the runtime's object heap, which knows the live
# ones: at the end of main the program reports on stderr the objects it never
# deleted, by class, then releases them (an object before what it owns, a cycle
# once). The suites cannot see this - the report is the program's own output
# after its last statement, and a suite that leaks still passes.
#
# Each probe under ci/heap_probes\<name>\ is one property in miniature:
#   main.fly     the program;
#   expect.txt   line 1: the exit code it must end with;
#   stderr.txt   what stderr must be, exactly (absent: nothing at all);
#   stdout.txt   what stdout must be, exactly (absent: nothing at all);
#   suite.txt    optional: line 1 names a suite in the probe's sources; the probe
#                is then built and run by `fly --suite=<name>` (its output: the
#                case results on stdout, the per-case leak warnings on stderr)
#                instead of as a program with a main.
# Line endings are ignored.
#
# STAGE selects the compiler under test (default 2, the one that ships).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$env:STAGE = $STAGE
$FLY = if ($env:FLY) { $env:FLY } else { "build/stage$STAGE/bin/fly.exe" }
if (-not (Test-Path $FLY -PathType Leaf)) { Write-Host "error: compiler '$FLY' not found."; exit 1 }
$FLY = (Resolve-Path $FLY).Path
# The compiler loads LLVM-C.dll at run time and reaches ld.lld from build\llvm\bin.
$llvmBin = Join-Path (Get-Location) 'build\llvm\bin'
if ((Test-Path $llvmBin) -and ($env:PATH -notlike "*$llvmBin*")) { $env:PATH = "$llvmBin;$env:PATH" }

$OUT = 'build/test/heap'
New-Item -ItemType Directory -Force $OUT | Out-Null

# norm - a file's text with CR dropped and the trailing newlines trimmed
function Get-Norm([string]$path) {
    if (-not (Test-Path $path)) { return '' }
    $t = Get-Content $path -Raw
    if ($null -eq $t) { return '' }
    return $t.Replace("`r", '').TrimEnd("`n")
}

$failed = 0
$count = 0
foreach ($probe in Get-ChildItem 'ci/heap_probes' -Directory | Sort-Object Name) {
    $name = $probe.Name
    $count++
    $wantCode = [int](@(Get-Content (Join-Path $probe.FullName 'expect.txt'))[0])
    $wantErr = Get-Norm (Join-Path $probe.FullName 'stderr.txt')
    $wantOut = Get-Norm (Join-Path $probe.FullName 'stdout.txt')
    $dir = Join-Path $OUT $name
    Remove-Item $dir -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force $dir | Out-Null
    $suiteFile = Join-Path $probe.FullName 'suite.txt'
    if (Test-Path $suiteFile) {
        $suite = (@(Get-Content $suiteFile)[0]).Trim()
        & $FLY @FLY_TARGET_ARGS "--suite=$suite" --src-dir $probe.FullName --out-dir $dir > "$dir/stdout.log" 2> "$dir/stderr.log"
        $rc = $LASTEXITCODE
    } else {
        & $FLY @FLY_TARGET_ARGS --src-dir $probe.FullName -o "$name.exe" --out-dir $dir *> "$dir/compile.log"
        $exe = Join-Path $dir "$name.exe"
        if ($LASTEXITCODE -ne 0 -or -not (Test-Path $exe)) {
            Write-Host "  FAIL  $name - does not compile"
            Get-Content "$dir/compile.log" | Select-Object -Last 5 | ForEach-Object { Write-Host "        $_" }
            $failed++
            continue
        }
        & $exe > "$dir/stdout.log" 2> "$dir/stderr.log"
        $rc = $LASTEXITCODE
    }
    $gotErr = Get-Norm "$dir/stderr.log"
    $gotOut = Get-Norm "$dir/stdout.log"
    if ($rc -ne $wantCode) {
        Write-Host "  FAIL  $name - exit $rc, expected $wantCode"
        $failed++
    } elseif ($gotErr -cne $wantErr) {
        Write-Host "  FAIL  $name - stderr differs; expected:"
        $wantErr -split "`n" | ForEach-Object { Write-Host "        $_" }
        Write-Host "        got:"
        $gotErr -split "`n" | Select-Object -First 10 | ForEach-Object { Write-Host "        $_" }
        $failed++
    } elseif ($gotOut -cne $wantOut) {
        Write-Host "  FAIL  $name - stdout differs; expected:"
        $wantOut -split "`n" | ForEach-Object { Write-Host "        $_" }
        Write-Host "        got:"
        $gotOut -split "`n" | Select-Object -First 10 | ForEach-Object { Write-Host "        $_" }
        $failed++
    } else {
        Write-Host "  ok    $name"
    }
}

if ($count -eq 0) { Write-Host 'error: no probes under ci/heap_probes'; exit 1 }
if ($failed -gt 0) { Write-Host "test_heap: $failed of $count probe(s) failed"; exit 1 }
Write-Host "test_heap: all checks passed ($count probes)"
exit 0
