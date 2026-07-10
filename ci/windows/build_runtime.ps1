# -----------------------------------------------------------------------------
# build_runtime.ps1 - stage the Fly runtime for Windows into build\stage$STAGE\lib.
# PowerShell counterpart of ci/linux/build_runtime.sh; see stage1.ps1 for the
# stage map. Run with STAGE=1 (seeds from the stage0 lib) or STAGE=2 (seeds
# from the stage1 lib).
#
# On Windows the runtime is SEED-ONLY for now: the bootstrap's fly_runtime_lib.lib
# (Windows C primitives + the reference-built Fly member) plus llvm.fly.h and
# runtime.fly.h are staged as-is.
#
# WHY NOT recompile like build_runtime.sh (yet): the Linux flow rebuilds the Fly
# member from runtime/lib/runtime.fly. The Windows source (runtime-windows.fly)
# was "written and compile-checked on Linux; NOT linked or run on Windows" - and
# it currently HANGS at startup on Windows (stack/heap overflow in the exe-path /
# GetCommandLineA args handling), regardless of which fly compiles it. Linking
# fly.exe against that fresh runtime makes the compiler hang on EVERY input, so a
# recompile here is a hard regression (stage1 links it, then stage2 hangs the
# moment it runs stage1's fly.exe). Keep seeding the working reference runtime
# until runtime-windows.fly runs cleanly on Windows; the recompile port (with a
# __atomic C shim + lib.exe merge) is staged and validated but gated on that fix.
# Output: $LIB/fly_runtime_lib.lib + runtime.fly.h + llvm.fly.h.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# -- Stage plumbing: pick the in/out dirs from $STAGE. -------------------------
$STAGE = if ($env:STAGE) { $env:STAGE } else { '1' }
$LIB = "build/stage$STAGE/lib"
New-Item -ItemType Directory -Force $LIB | Out-Null
$SEED = if ($STAGE -eq '1') { 'build/stage0/lib' } else { 'build/stage1/lib' }

foreach ($f in 'llvm.fly.h', 'runtime.fly.h', 'fly_runtime_lib.lib') {
    if (-not (Test-Path "$SEED/$f")) {
        Write-Host "error: seed '$SEED\$f' missing - run the previous stage first (stage0.ps1 / stage1.ps1)."
        exit 1
    }
    Copy-Item "$SEED/$f" $LIB/ -Force
}

Write-Host "stage${STAGE}: runtime -> $LIB/fly_runtime_lib.lib (seeded; Fly-member rebuild is a follow-up)"
exit 0
