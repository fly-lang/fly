# -----------------------------------------------------------------------------
# build_compiler.ps1 - build the COMPILED std library, then the self-host `fly`
# (Windows). PowerShell port of build_compiler.sh.
#
# Mirrors fly/'s CMake model: the standard library is first compiled into ONE
# archive `fly_std_lib.lib` plus flat `*.fly.h` headers via the bootstrap
# compiler `--lib`; the compiler executable is then built LINKING that archive.
# The release ships `bin/fly` + a sibling `lib/` holding the compiled std
# (fly_std_lib.lib + *.fly.h + the runtime archive + bridge stubs), NOT source.
#
# A released fly resolves std at <exe_dir>/../lib. The reference compiler has no
# flag to point its stdlib/runtime dir elsewhere, so to link OUR archive we run a
# copy of the bootstrap FROM build/bin: <exe>/../lib then resolves to build/lib
# and auto-discovery loads our headers + links fly_std_lib.lib (+ the runtime lib).
# -----------------------------------------------------------------------------

$ErrorActionPreference = 'Stop'
# Under `shell: pwsh` CI runners, pwsh 7.4 enables this preference, making a
# native non-zero exit throw before our own $LASTEXITCODE check. Disable it so the
# explicit check below is the sole arbiter (harmless no-op variable on PS 5.1).
$PSNativeCommandUseErrorActionPreference = $false
# Scripts live in ci\windows\; operate from the project root (two levels up).
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

# build/bin + build/lib are the release-artifact paths the workflows
# (build-windows.yml -> release.yml) upload and package.
$OUT = "build/bin"
$LIB = "build/lib"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null
New-Item -ItemType Directory -Force $LIB | Out-Null

# Invoke the bootstrap compiler via $FLY (default: `fly` on PATH). CI sets FLY to
# an absolute path: released binaries derive their stdlib dir from the executable
# path, and a bare `fly` resolved from a cwd containing a `fly/` directory fails
# that lookup. A path-qualified value sidesteps it. See ../fly Driver.cpp.
$FLY = if ($env:FLY) { $env:FLY } else { "fly" }

function Assert-LastExit($what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed: $FLY exited with code $LASTEXITCODE" }
}

# -- 1) Seed the runtime bridge stubs + runtime archive from the bootstrap's own
#       lib. The `--lib` std build needs llvm.fly.h/runtime.fly.h to resolve
#       fly.runtime/fly.llvm; the release+link need the runtime archive. --------
$BLIB = Resolve-Path (Join-Path (Split-Path $FLY -Parent) '..\lib')
Copy-Item "$BLIB/llvm.fly.h" $LIB/; Copy-Item "$BLIB/runtime.fly.h" $LIB/
Copy-Item "$BLIB/fly_runtime_lib.lib" $LIB/

# -- 2) Compile the std into one archive + flat *.fly.h headers. Source order
#       mirrors fly/std/CMakeLists.txt. The compiler appends the .lib extension. -
& $FLY --lib -o "$LIB/fly_std_lib" `
    "$STD/assert.fly" "$STD/str.fly" "$STD/math.fly" `
    "$STD/os/time.fly" "$STD/os/env.fly" "$STD/os/path.fly" "$STD/os/io.fly" "$STD/os/fs.fly" `
    "$STD/sync.fly" "$STD/mem.fly" "$STD/bridge/clang.fly" `
    "$STD/data/list.fly" "$STD/data/stack.fly" "$STD/data/queue.fly" "$STD/data/deque.fly" `
    "$STD/data/map.fly" "$STD/data/set.fly" "$STD/data/tree.fly" "$STD/data/wrapper.fly" `
    "$STD/os/proc.fly"
Assert-LastExit "std --lib build"

# -- 3) Build the compiler executable LINKING our fly_std_lib.lib. Run a copy of
#       the bootstrap from build/bin so <exe>/../lib == build/lib: auto-discovery
#       then loads our headers and links fly_std_lib.lib + the runtime lib. -------
Copy-Item $FLY "$OUT/flyc.exe"
& "$OUT/flyc.exe" compiler/Fly.fly `
    --src-dir compiler `
    -o fly --out-dir $OUT
Assert-LastExit "compiler build"

# -- 4) Cleanup: bin/ ships only the executable (drop the bootstrap copy + objects).
Remove-Item "$OUT/flyc.exe" -Force -ErrorAction SilentlyContinue
Remove-Item "$OUT/*.o" -Force -ErrorAction SilentlyContinue

Write-Host "fly -> $OUT/fly"
Write-Host "std -> $LIB (fly_std_lib.lib + *.fly.h + runtime)"
