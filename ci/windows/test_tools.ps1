# -----------------------------------------------------------------------------
# test_tools.ps1 - run the UNIT suites of the tools (tools/*/test/*Suite.fly).
#
# These are the in-process counterparts of the end-to-end scripts: test_lsp.ps1
# and test_registry.ps1 drive the BUILT binaries over stdio and sockets, while
# these suites call the libraries directly and can therefore reach the corners a
# wire test cannot (before-compile state, malformed input, every branch of the
# routing table).
#
# Optional SELECTOR (first argument, or $env:FLY_TEST_SUITE): a suite name.
# Empty = every tool suite.
#
# Each suite is built AND run in one shot by `--suite=<Name>`; fly's exit code
# is the suite's. The LSP suites reach the compiler through the analyzer, so
# they get --src-dir compiler/lib and need LLVM-C.dll on PATH; the registry
# suite is std-only.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$SEL = if ($args.Count -gt 0 -and $args[0]) { $args[0] } elseif ($env:FLY_TEST_SUITE) { $env:FLY_TEST_SUITE } else { '' }

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$FLY = if ($env:FLY) { $env:FLY } else { "build/stage$STAGE/bin/fly.exe" }
if (-not (Test-Path $FLY -PathType Leaf)) { Write-Host "error: compiler '$FLY' not found."; exit 1 }
$FLY = (Resolve-Path $FLY).Path

# The analyzer's import closure reaches CodeGen, so the suite executables import
# LLVM-C.dll; they run from $OUT, not next to it.
$llvmBin = Join-Path (Get-Location) 'build\llvm\bin'
if (Test-Path (Join-Path $llvmBin 'LLVM-C.dll')) { $env:PATH = "$llvmBin;$env:PATH" }

$OUT = 'build/test'
$STD = 'std/lib'
New-Item -ItemType Directory -Force $OUT | Out-Null

# name, tool root, needs the compiler sources
$SUITES = @(
    @{ Name = 'LspProtocolSuite';    Root = 'tools/lsp';      Compiler = $true  },
    @{ Name = 'LspTransportSuite';   Root = 'tools/lsp';      Compiler = $true  },
    @{ Name = 'LspAnalyzerSuite';    Root = 'tools/lsp';      Compiler = $true  },
    @{ Name = 'RegistryHandlerSuite';Root = 'tools/registry'; Compiler = $false }
)

$passed = 0
$failed = 0
foreach ($s in $SUITES) {
    if ($SEL -and $s.Name -ne $SEL) { continue }
    $log = "$OUT/tools_$($s.Name).log"
    if ($s.Compiler) {
        & $FLY --suite=$($s.Name) --src-dir $($s.Root) --src-dir compiler/lib `
               -o "test_$($s.Name)" --out-dir $OUT -L $STD *> $log
    } else {
        & $FLY --suite=$($s.Name) --src-dir $($s.Root) `
               -o "test_$($s.Name)" --out-dir $OUT -L $STD *> $log
    }
    $rc = $LASTEXITCODE
    $line = (Get-Content $log | Where-Object { $_ -match "^suite $($s.Name):" } | Select-Object -Last 1)
    if ($rc -eq 0) {
        Write-Host "  PASS  $($s.Name)  $line"
        $passed++
    } else {
        Write-Host "  FAIL  $($s.Name)  $line"
        Get-Content $log | Where-Object { $_ -match 'FAIL|error:' } | Select-Object -First 10 | ForEach-Object { Write-Host "        $_" }
        $failed++
    }
}

Write-Host ''
Write-Host "  $passed passed, $failed failed"
if ($failed -ne 0) { exit 1 }
exit 0
