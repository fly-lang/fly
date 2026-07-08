# -----------------------------------------------------------------------------
# stagelib.ps1 - shared stage plumbing for the Rust-style bootstrap (dot-sourced
# by the build_*.ps1 scripts). PowerShell port of ci/linux/stagelib.sh.
#
#   stage0  downloaded bootstrap compiler (build\bootstrap) + its precompiled lib
#   stage1  stage0 builds runtime(seed), std and the compiler FROM IN-TREE SOURCES
#           into build\stage1, then links a first bin\fly.exe (build_driver.ps1)
#   stage2  that fly.exe rebuilds the driver into the SHIPPED build\bin + build\lib
#
# Set $env:STAGE = '1' | '2' (default 2). Each stage compiles with its own world:
#   stage1: $FLY is hardlinked (copy fallback) into build\stage1\bin so the
#           GetModuleFileName-based <exe>\..\lib discovery serves build\stage1\lib,
#           never the bootstrap's own lib. $SEED = the stage0 lib.
#   stage2: $FLY = build\bin\fly.exe, lib = build\lib, $SEED = build\stage1\lib.
#
# Exports: $STAGE, $FLY (compiler to run), $LIB (its lib = output dir),
#          $CDIR (stage-1 compiler artifacts, build-only), $SEED (seed source).
# -----------------------------------------------------------------------------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$S1 = 'build/stage1'
$CDIR = "$S1/compiler"

function Resolve-Fly($name) {
    if ($name -notmatch '[\\/]') {
        $r = Get-Command $name -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($r) { return $r.Source } else { return $null }
    }
    return $name
}

if ($STAGE -eq '1') {
    $fly0 = Resolve-Fly $(if ($env:FLY) { $env:FLY } else { 'fly' })
    if (-not $fly0 -or -not (Test-Path $fly0 -PathType Leaf)) {
        Write-Host "error: stage0 compiler not found (set `$env:FLY = 'C:\path\to\fly.exe')"; exit 1
    }
    $SEED = (Resolve-Path (Join-Path (Split-Path $fly0 -Parent) '..\lib')).Path
    New-Item -ItemType Directory -Force "$S1/bin", "$S1/lib" | Out-Null
    # Hardlink (same file, no copy) so the exe-path-based lib discovery resolves to
    # build\stage1\lib; Copy-Item fallback for cross-volume checkouts.
    Remove-Item "$S1/bin/fly.exe" -Force -ErrorAction SilentlyContinue
    try { New-Item -ItemType HardLink -Path "$S1/bin/fly.exe" -Target $fly0 -ErrorAction Stop | Out-Null }
    catch { Copy-Item $fly0 "$S1/bin/fly.exe" -Force }
    $FLY = "$S1/bin/fly.exe"
    $LIB = "$S1/lib"
} else {
    # Stage 2 ALWAYS runs the stage-1 output - $env:FLY is deliberately ignored: in
    # CI install_prerequisites.ps1 exports FLY=<stage0 bootstrap> job-wide
    # ($GITHUB_ENV), and honoring it here would silently rerun the bootstrap.
    $FLY = 'build/bin/fly.exe'
    if (-not (Test-Path $FLY -PathType Leaf)) {
        Write-Host "error: stage-1 fly 'build\bin\fly.exe' not found - run the stage-1 builds first."; exit 1
    }
    $SEED = "$S1/lib"
    $LIB = 'build/lib'
    New-Item -ItemType Directory -Force $LIB | Out-Null
}

function Assert-LastExit($what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed (exit $LASTEXITCODE)" }
}

# Space nested generic closers (`>>` -> `> >`) so a header re-read lexes them
# (the parser fuses `>>`); idempotent, handles `>>>` via the loop.
function Split-GenericClosers($file) {
    $c = Get-Content $file -Raw
    while ($c -match '>>') { $c = $c -replace '>>', '> >' }
    Set-Content $file $c -NoNewline
}
