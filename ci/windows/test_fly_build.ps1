# -----------------------------------------------------------------------------
# test_fly_build.ps1 - `fly build` builds Fly itself (0.15.2, plan "level 2").
#
# The stage's fly runs `fly build --profile dist` on the repository: the root
# Manifest.fly describes runtime, TLS archives, std, compiler and tools, and
# everything must come out in target\dist. Then:
#   1. the fly it built reports the version Manifest.fly declares;
#   2. that fly compiles and runs a program against the std and runtime built
#      next to it;
#   3. a scratch project built with `fly build` reads its own manifest through
#      the generated fly.meta.package module (name() and version()).
# release/0.16.X bootstraps with `fly build`: this is what it will rely on.
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

$version = & "$PSScriptRoot\gen_package.ps1"

# -- the toolchain, built by fly build -----------------------------------------
Remove-Item 'target/dist' -Recurse -Force -ErrorAction SilentlyContinue
& $FLY build --profile dist *> "$OUT/build.log"
if ($LASTEXITCODE -ne 0) {
    Write-Host "  FAIL  fly build --profile dist (exit $LASTEXITCODE)"; Get-Content "$OUT/build.log" | Select-Object -Last 20; exit 1
}
foreach ($a in 'fly.exe', 'fly-lsp.exe', 'fly-registry.exe', 'fly_std_lib.lib', 'fly_runtime_lib.lib', 'fly_tls_stub.lib') {
    if (-not (Test-Path "target/dist/$a")) { Write-Host "  FAIL  target\dist\$a missing"; exit 1 }
}
Write-Host '  ok    fly build --profile dist built runtime, TLS, std, fly, fly-lsp, fly-registry'

# -- 1. its version --------------------------------------------------------------
$v = & target/dist/fly.exe --version 2>&1 | Out-String
if ($v -notmatch [regex]::Escape("fly version $version ")) { Write-Host "  FAIL  target\dist\fly.exe --version: $v (want $version)"; exit 1 }
Write-Host "  ok    the built fly reports $version (Manifest.fly)"

# -- 2. it compiles and runs a program on its own std and runtime ------------------
$P = "$OUT/hello"
New-Item -ItemType Directory -Force $P | Out-Null
@'
import fly.os.io
import fly.str

void main() {
    io.printLn(str.concat("hello ", str.convertD(0.25)))
}
'@ | Set-Content "$P/main.fly" -Encoding UTF8
& target/dist/fly.exe --src-dir $P -o hello --out-dir $P --runtime-lib-dir target/dist -L target/dist --mingw-sysroot build/mingw *> "$P/compile.log"
if ($LASTEXITCODE -ne 0) { Write-Host '  FAIL  the built fly compiling a program'; Get-Content "$P/compile.log" | Select-Object -Last 10; exit 1 }
$run = & "$P/hello.exe" 2>&1 | Out-String
if ($run.Trim() -ne 'hello 0.25') { Write-Host "  FAIL  program built by the built fly printed: $run"; exit 1 }
Write-Host '  ok    the built fly compiles and runs a program on the std and runtime built with it'

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
