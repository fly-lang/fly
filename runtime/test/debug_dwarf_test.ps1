# End-to-end test: compile a Fly program with --debug-symbols and verify that
# llvm-dwarfdump reports valid DWARF info (compile unit, variables, line info).
#
# Supports Windows (PowerShell 5.1+).
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File runtime\test\debug_dwarf_test.ps1 [fly-binary] [llvm-dwarfdump]
#
# CMake passes both paths automatically via add_test().

param(
    [string]$FlyBin        = "",
    [string]$LlvmDwarfDump = ""
)

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir  = Join-Path $ScriptDir "..\..\cmake-build-relwithdebinfo"

if (-not $FlyBin) { $FlyBin = Join-Path $BuildDir "bin\fly.exe" }

# Resolve llvm-dwarfdump: prefer the path passed by CMake, then search PATH.
if ($LlvmDwarfDump -like "*-NOTFOUND" -or -not (Test-Path $LlvmDwarfDump -ErrorAction SilentlyContinue)) {
    $LlvmDwarfDump = ""
    foreach ($candidate in @("llvm-dwarfdump", "llvm-dwarfdump-20", "llvm-dwarfdump-19", "llvm-dwarfdump-18")) {
        $found = Get-Command $candidate -ErrorAction SilentlyContinue
        if ($found) { $LlvmDwarfDump = $found.Source; break }
    }
}

# ── Prerequisites ──────────────────────────────────────────────────────────────
if (-not (Test-Path $FlyBin)) {
    Write-Host "SKIP: fly binary not found at $FlyBin"
    exit 0
}

if (-not $LlvmDwarfDump) {
    Write-Host "SKIP: llvm-dwarfdump not found in PATH"
    exit 0
}

$Work = Join-Path $env:TEMP "fly_debug_test_$PID"
New-Item -ItemType Directory -Path $Work -Force | Out-Null

try {

# ── Fly source ─────────────────────────────────────────────────────────────────
$Source = @"
void main() {
    int a = 10
    int b = 20
    int c = a + b
}
"@
Set-Content -Path "$Work\dbg_test.fly" -Value $Source -Encoding UTF8

# ── Compile with debug symbols ─────────────────────────────────────────────────
# --debug-symbols emits DWARF without the verbose DebugLog that --debug adds,
# so stderr stays readable and is worth showing on failure. fly compiles the
# WORK directory (--src-dir); the runtime auto-links from <bin>\..\lib.
$compileOut = & $FlyBin --debug-symbols --src-dir "$Work" -o "$Work\dbg_test.exe" 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "FAIL: compilation failed"
    Write-Host $compileOut
    exit 1
}
Write-Host "Compile: OK"

# ── DWARF content assertions ───────────────────────────────────────────────────
$DwarfInfo = & $LlvmDwarfDump "--debug-info" "$Work\dbg_test.exe" 2>&1 | Out-String
$Pass = $true

if ($DwarfInfo -notmatch "DW_TAG_compile_unit") {
    Write-Host "FAIL: no DW_TAG_compile_unit"
    $Pass = $false
}

if ($DwarfInfo -notmatch "DW_TAG_subprogram") {
    Write-Host "FAIL: no DW_TAG_subprogram (function debug info missing)"
    $Pass = $false
}

if ($DwarfInfo -notmatch "DW_TAG_variable") {
    Write-Host "FAIL: no DW_TAG_variable (local variable debug info missing)"
    $Pass = $false
}

$DwarfLine = & $LlvmDwarfDump "--debug-line" "$Work\dbg_test.exe" 2>&1 | Out-String
if ($DwarfLine -notmatch "Line table") {
    Write-Host "FAIL: no line table in .debug_line"
    $Pass = $false
}

if ($Pass) {
    Write-Host "PASS: DWARF debug info test succeeded"
    exit 0
} else {
    exit 1
}

} finally {
    Remove-Item -Recurse -Force $Work -ErrorAction SilentlyContinue
}
