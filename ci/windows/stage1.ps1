# -----------------------------------------------------------------------------
# stage1.ps1 - stage 1 of the Rust-style staged bootstrap (Windows mirror of
# ci/linux/stage1.sh):
#
#   stage0  the downloaded seed + its precompiled lib (build\stage0, set up by
#           stage0.ps1 - which also fetches LLVM). From 0.16 the seed is the
#           0.15.2 self-host release, the first one that can build Fly with
#           `fly build`.
#   stage1  THIS SCRIPT: the seed runs `fly build` on the root Manifest.fly -
#           runtime, TLS archives, std, compiler, one target at a time - and
#           the results are laid out in build\stage1. THEN the suites run with
#           build\stage1\bin\fly.exe, the compiler this stage just produced.
#   stage2  stage2.ps1: build\stage1\bin\fly.exe rebuilds the shipped artifacts
#           into build\stage2 (the self-hosting fixpoint check).
#
# `fly build` (0.16): up to 0.15 this stage drove the seed with one hand-written
# `fly --lib` / `fly -c` per artifact (build_runtime, build_std, build_compiler)
# and linked fly.exe in link_bin.ps1. Now the Manifest describes the artifacts
# and the seed's own Builder compiles and links them, the same way any Fly
# project is built. Profile `stage1` (`stage1-debug` with FLY_DEBUG_SYMBOLS=1),
# output target\<profile>; see the profiles in Manifest.fly.
#
# One target per `fly build --target`, laid out after each: the seed runs from
# the hardlink build\stage1\bin\fly0.exe, so its <exe>\..\lib is build\stage1\lib
# (never the seed's own lib, whose std headers would clash with the in-tree
# std), and every target finds there the headers of the ones before it -
# runtime.fly.h for TLS and std, the std headers for the compiler.
#
# The seed (0.15.2) knows neither `dir` nor `rpaths` and writes every target
# flat into target\<profile>, but expands `@out/lib` - where the targets find
# the libraries before them - to target\<profile>\lib: that directory is kept a
# mirror of build\stage1\lib. The stage-1 fly honours `dir`, so the std it
# rebuilds lands in target\<profile>\lib.
#
# After the link, the std is built AGAIN by the fly just produced: the seed's
# std only served to link that fly; the stage-1 suites and stage 2 use a std
# built by the tree's own compiler.
#
# FLY_STAGE1_TESTS=0 skips the test steps (fast local builds; CI sets it and
# runs the same three suites from its own "Test stage 1" step). The tools
# (fly-lsp, fly-registry) are built at stage 2, by the fly that ships.
# FLY_BUNDLE_LLVM=1 is honoured by link_fly.ps1 (bundle only, FLY_LINKED=1).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
$env:STAGE = '1'

$stage1Tests = if ($env:FLY_STAGE1_TESTS) { $env:FLY_STAGE1_TESTS } else { '1' }

$BIN = 'build/stage1/bin'
$LIB = 'build/stage1/lib'
$FLY_PROFILE = if ($env:FLY_DEBUG_SYMBOLS -eq '1') { 'stage1-debug' } else { 'stage1' }
$T = "target/$FLY_PROFILE"
$TLIB = "$T/lib"

if (-not (Test-Path 'build/stage0/bin/fly.exe' -PathType Leaf)) {
    Write-Host "error: stage0 compiler missing - run ci\windows\stage0.ps1 first."; exit 1
}
if (-not (Test-Path 'build/stage0/lib/llvm.fly.h')) {
    Write-Host "error: build\stage0\lib\llvm.fly.h missing - run stage0.ps1 first."; exit 1
}
Remove-Item $BIN, $LIB, $T -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $BIN, $LIB, $TLIB | Out-Null
$FLY0 = "$BIN/fly0.exe"
try { New-Item -ItemType HardLink -Path $FLY0 -Target (Resolve-Path 'build/stage0/bin/fly.exe').Path -ErrorAction Stop | Out-Null }
catch { Copy-Item 'build/stage0/bin/fly.exe' $FLY0 -Force }
# The LLVM C API declarations every Fly compiler build needs.
Copy-Item 'build/stage0/lib/llvm.fly.h' "$LIB/llvm.fly.h" -Force
Copy-Item 'build/stage0/lib/llvm.fly.h' "$TLIB/llvm.fly.h" -Force
# Where the target just built left its output: flat for the seed, lib\ for the
# stage-1 fly (switched before the rebuild below).
$script:SRC = $T

# Invoke-FlyBuild FLY KEY [-Force] - `fly build` one Manifest target.
function Invoke-FlyBuild([string]$fly, [string]$key, [switch]$Force) {
    $extra = @(); if ($Force) { $extra += '--force' }
    Write-Host "stage1: fly build --profile $FLY_PROFILE --target $key ($(Split-Path $fly -Leaf))"
    & $fly build --profile $FLY_PROFILE --target $key @extra
    if ($LASTEXITCODE -ne 0) { Write-Host "error: fly build --target $key failed (exit $LASTEXITCODE)"; exit 1 }
}

# Install-File SRC NAME - a file into build\stage1\lib and into its
# target\<profile>\lib mirror (unless SRC already is the mirror's copy).
function Install-File([string]$src, [string]$name) {
    Copy-Item $src "$LIB/$name" -Force
    $mirror = Join-Path (Resolve-Path $TLIB).Path $name
    if ((Resolve-Path $src).Path -ne $mirror) {
        Copy-Item $src $mirror -Force
    }
}

# Install-Artifact NAME - the built library NAME -> build\stage1\lib\NAME.
function Install-Artifact([string]$name) {
    if (-not (Test-Path "$script:SRC/$name")) { Write-Host "error: $script:SRC\$name was not built."; exit 1 }
    Install-File "$script:SRC/$name" $name
}

# Install-Header SRC DST - a generated header into build\stage1\lib (and the
# mirror), with nested generic closers spaced (`>>` -> `> >`) as
# build_runtime.ps1 does.
function Install-Header([string]$src, [string]$dst) {
    if (-not (Test-Path "$script:SRC/$src")) { Write-Host "error: $script:SRC\$src was not generated."; exit 1 }
    $c = Get-Content "$script:SRC/$src" -Raw
    while ($c -match '>>') { $c = $c -replace '>>', '> >' }
    Set-Content "$LIB/$dst" $c -NoNewline
    Set-Content "$TLIB/$dst" $c -NoNewline
}

# Install-StdHeaders - every std header the std target generated (not the
# runtime, TLS and LLVM ones, installed under their own names).
function Install-StdHeaders {
    $n = 0
    $skip = @('runtime.fly.h', 'llvm.fly.h')
    Get-ChildItem "$script:SRC/*.fly.h" |
        Where-Object { ($_.Name -cnotmatch '^(Runtime|Tls)[A-Z]') -and ($skip -notcontains $_.Name) } |
        ForEach-Object {
            Install-File $_.FullName $_.Name
            $n++
        }
    if ($n -eq 0) { Write-Host "error: no std headers in $script:SRC."; exit 1 }
    Write-Host "stage1: std -> $LIB (fly_std_lib.lib + $n headers)"
}

# -- runtime and TLS archives, built by the seed ---------------------------------
Invoke-FlyBuild $FLY0 'runtime-windows'
Install-Artifact 'fly_runtime_lib.lib'
Install-Header 'RuntimeWindows.fly.h' 'runtime.fly.h'
Invoke-FlyBuild $FLY0 'tls-stub'
Install-Artifact 'fly_tls_stub.lib'
Install-Header 'TlsStub.fly.h' 'TlsStub.fly.h'
Invoke-FlyBuild $FLY0 'tls-schannel'
Install-Artifact 'fly_tls_lib.lib'

# -- std and the compiler, built by the seed ---------------------------------------
Invoke-FlyBuild $FLY0 'std'
Install-Artifact 'fly_std_lib.lib'
Install-StdHeaders
Invoke-FlyBuild $FLY0 'fly'
if (-not (Test-Path "$T/fly.exe")) { Write-Host "error: $T\fly.exe was not linked."; exit 1 }
Copy-Item "$T/fly.exe" "$BIN/fly.exe" -Force
$env:FLY_LINKED = '1'
try { & '.\ci\windows\link_fly.ps1' } finally { Remove-Item Env:FLY_LINKED -ErrorAction SilentlyContinue }
if ($LASTEXITCODE -ne 0) { Write-Host "error: link_fly.ps1 failed (exit $LASTEXITCODE)"; exit 1 }

# -- the std again, built by the stage-1 fly (which honours `dir`) --------------------
Invoke-FlyBuild "$BIN/fly.exe" 'std' -Force
$script:SRC = $TLIB
Install-Artifact 'fly_std_lib.lib'
Install-StdHeaders

# -- the stage-1 suites, bottom-up (runtime -> std -> compiler) ---------------------
# No $env:FLY override: at STAGE=1 the test scripts default to
# build\stage1\bin\fly.exe. test_compiler is the largest set and the one most
# likely to catch a stage-1 codegen defect.
if ($stage1Tests -ne '0') {
    foreach ($step in 'test_runtime', 'test_std', 'test_compiler') {
        & ".\ci\windows\$step.ps1"
        if ($LASTEXITCODE -ne 0) { Write-Host "error: $step.ps1 failed (exit $LASTEXITCODE)"; exit 1 }
    }
}

Write-Host "stage1: done - build\stage1\bin\fly.exe"
exit 0
