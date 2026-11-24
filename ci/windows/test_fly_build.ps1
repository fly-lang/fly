# -----------------------------------------------------------------------------
# test_fly_build.ps1 - the toolchain `fly build` built is one of its own.
#
# From 0.16 stage 2 IS `fly build --profile dist --out-dir build\stage2` on the
# root Manifest.fly (runtime, TLS archives, std, compiler, tools and the LLVM
# pieces a release ships), so build\stage<N> must be a complete release laid
# out as bin\ and lib\. Then, with nothing on PATH but the system directory (no
# build\llvm\bin, no option pointing into build\):
#   1. the stage's fly starts and reports the version Manifest.fly declares;
#   2. that fly compiles and runs a program with what lies next to it alone:
#      the std and runtime in lib\, the linker and the mingw sysroot in bin\;
#   3. a scratch project built with `fly build` reads its own manifest through
#      the generated fly.meta.package module (name() and version()).
# Run from stage2.ps1 (STAGE=2 by default). Exit 0 = all checks passed.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$FLY = (Resolve-Path "build/stage$STAGE/bin/fly.exe").Path
$OUT = 'build/test/fly_build'
Remove-Item $OUT -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $OUT | Out-Null

# The version Manifest.fly declares (`string version = "x.y.z"`, its single source).
$m = Select-String -Path 'Manifest.fly' -Pattern '^\s*string\s+version\s*=\s*"([0-9]+\.[0-9]+\.[0-9]+)"' | Select-Object -First 1
if (-not $m) { Write-Host '  FAIL  Manifest.fly has no `string version = "x.y.z"` line'; exit 1 }
$version = $m.Matches[0].Groups[1].Value

# -- the layout of the release ----------------------------------------------------
$R = "build/stage$STAGE"
$want = @('bin/fly.exe', 'bin/fly-lsp.exe', 'bin/fly-registry.exe', 'bin/LLVM-C.dll', 'bin/ld.lld.exe', 'bin/lldb.exe',
          'bin/lldb-dap.exe', 'bin/mingw/lib/crt2.o', 'lib/fly_std_lib.lib', 'lib/fly_runtime_lib.lib',
          'lib/fly_tls_stub.lib', 'lib/fly_tls_lib.lib', 'lib/llvm.fly.h', 'lib/str.fly.h')
foreach ($a in $want) {
    if (-not (Test-Path "$R/$a")) { Write-Host "  FAIL  $R/$a missing"; exit 1 }
}
Write-Host "  ok    $R holds bin\ (fly, tools, LLVM, mingw) and lib\ (archives, headers)"

# Invoke-Bare COMMAND - run a command in a fresh pwsh whose PATH is the system
# directory only, so nothing is found through build\llvm\bin; returns its output
# with the exit code on the last line.
function Invoke-Bare([string]$cmd) {
    $script = "`$env:PATH = '$env:SystemRoot\System32'; $cmd; `"rc=`$LASTEXITCODE`""
    return (pwsh -NoProfile -Command $script 2>&1 | Out-String)
}

# -- 1. it starts on its own and reports its version -------------------------------
$v = Invoke-Bare "& '$FLY' --version"
if ($v -notmatch [regex]::Escape("fly version $version ")) {
    Write-Host "  FAIL  $R/bin/fly.exe --version with a bare PATH: $v (want $version)"; exit 1
}
Write-Host "  ok    the stage's fly starts with a bare PATH and reports $version (Manifest.fly)"

# -- 2. it compiles and runs a program with what lies next to it --------------------
$P = (New-Item -ItemType Directory -Force "$OUT/hello").FullName
@'
import fly.os.io
import fly.str

void main() {
    io.printLn(str.concat("hello ", str.convertD(0.25)))
}
'@ | Set-Content "$P/main.fly" -Encoding UTF8
$c = Invoke-Bare "& '$FLY' --src-dir '$P' -o hello --out-dir '$P'"
if ($c -notmatch 'rc=0') { Write-Host "  FAIL  the stage's fly compiling a program with no options:"; Write-Host $c; exit 1 }
$run = Invoke-Bare "& '$P/hello.exe'"
if ($run -notmatch '(?m)^hello 0\.25\s*$') { Write-Host "  FAIL  program built by the stage's fly printed: $run"; exit 1 }
Write-Host "  ok    the stage's fly compiles and runs a program with no options: std, runtime, linker, sysroot of its own"

# -- 3. a project reads its own manifest (fly.meta.package) -------------------------
$J = (New-Item -ItemType Directory -Force "$OUT/proj").FullName
New-Item -ItemType Directory -Force "$J/src" | Out-Null
@'
import fly.meta.*

class Manifest : fly.meta.Manifest {
    string name = "pkgdemo"
    string version = "3.2.1"
    Target[] targets = { new Target("pkgdemo", "src/main.fly", "") }
}
'@ | Set-Content "$J/Manifest.fly" -Encoding UTF8
@'
import fly.os.io
import fly.str
import fly.meta.package

void main() {
    io.printLn(str.concat(package.name(), str.concat(" ", package.version())))
}
'@ | Set-Content "$J/src/main.fly" -Encoding UTF8
Push-Location $J
try {
    & $FLY build *> "$J/build.log"
    $rc = $LASTEXITCODE
} finally { Pop-Location }
if ($rc -ne 0) { Write-Host '  FAIL  fly build of a project'; Get-Content "$J/build.log" | Select-Object -Last 10; exit 1 }
$pr = & "$J/target/debug/pkgdemo.exe" 2>&1 | Out-String
if ($pr.Trim() -ne 'pkgdemo 3.2.1') { Write-Host "  FAIL  fly.meta.package: the program printed '$($pr.Trim())'"; exit 1 }
Write-Host '  ok    a program reads its manifest: fly.meta.package name() and version()'

Write-Host 'test_fly_build: all checks passed'
exit 0
