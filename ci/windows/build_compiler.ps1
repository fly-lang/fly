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

# Invoke the bootstrap compiler via $FLY (default: `fly` on PATH). The compiler
# derives its stdlib dir from its own executable path (argv[0]), and a bare name
# breaks that lookup - so a path-less $FLY is resolved through PATH into an
# absolute path here. See ../fly Driver.cpp.
$FLY = if ($env:FLY) { $env:FLY } else { "fly" }
if ($FLY -notmatch '[\\/]') {
    $resolved = Get-Command $FLY -CommandType Application -ErrorAction SilentlyContinue |
                Select-Object -First 1
    if (-not $resolved) {
        Write-Host "error: bootstrap compiler '$FLY' not found on PATH."
        Write-Host "       Set FLY to the bootstrap compiler, e.g.:"
        Write-Host "       `$env:FLY = 'C:\path\to\fly\build\bin\fly.exe'"
        exit 1
    }
    $FLY = $resolved.Source
}
if (-not (Test-Path $FLY -PathType Leaf)) {
    Write-Host "error: FLY='$FLY' is not an executable file."
    exit 1
}
if (-not (Test-Path (Join-Path (Split-Path $FLY -Parent) '..\lib') -PathType Container)) {
    Write-Host "error: no lib\ directory next to '$FLY' (expected <exe_dir>\..\lib"
    Write-Host "       with llvm.fly.h, runtime.fly.h, fly_runtime_lib.lib)."
    Write-Host "       Point FLY at a built bootstrap compiler, e.g. fly\build\bin\fly.exe."
    exit 1
}

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
# The bootstrap copy runs as build/bin/fly.exe but writes its output to a STAGING
# dir (not build/bin), then we move it into place: a process can't overwrite its
# own running executable — on Windows the running .exe is locked, so writing
# build/bin/fly.exe from a process running as build/bin/fly.exe fails with
# "permission denied" (lld-link). build/bin/fly.exe still resolves <exe>/../lib to
# build/lib as required.
$STAGE = "build/stage"
Remove-Item $STAGE -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $STAGE | Out-Null
Copy-Item $FLY "$OUT/fly.exe"
& "$OUT/fly.exe" compiler/Fly.fly `
    --src-dir compiler `
    -o fly --out-dir $STAGE
Assert-LastExit "compiler build"
Remove-Item "$OUT/fly.exe" -Force -ErrorAction SilentlyContinue  # bootstrap done — drop the copy
Move-Item "$STAGE/fly.exe" "$OUT/fly.exe" -Force                 # install the built compiler

# -- 3b) Optional SELF-CONTAINED bundle (Rust-style), gated on FLY_BUNDLE_LLVM=1.
#        The release then needs neither system LLVM nor a system linker.
#        Windows differs from Linux: no rpath is needed — the loader searches the
#        EXE's own directory for DLLs first, so copying LLVM-C.dll next to fly.exe
#        makes it self-contained. The linker `fly-lld.exe` is our own single-flavor
#        LLD driver (ci/linux/lld_driver.cpp → COFF on _WIN32), built against the
#        fork LLVM's static archives (lld's C++ deps aren't in the C-API DLL).
#        NOTE: validated in CI only (no local Windows here); the clang link line
#        below is a first cut and may need CI iteration on the exact lib set.
if ($env:FLY_BUNDLE_LLVM -eq '1') {
    $llvmRoot = Resolve-Path (Join-Path (Split-Path $FLY -Parent) '..\..\llvm') -ErrorAction SilentlyContinue
    if (-not $llvmRoot) { $llvmRoot = (Resolve-Path 'build\llvm' -ErrorAction SilentlyContinue) }
    if (-not $llvmRoot) { throw "FLY_BUNDLE_LLVM=1: build\llvm (fork LLVM tree) not found; run install_prerequisites.ps1" }
    $llvmRoot = $llvmRoot.Path

    # 1) fly.exe self-contained for LLVM: copy the shared LLVM-C.dll next to it.
    Write-Host "bundling LLVM-C.dll into $OUT ..."
    Copy-Item (Join-Path $llvmRoot 'bin\LLVM-C.dll') "$OUT/LLVM-C.dll" -Force

    # 2) Build fly-lld.exe (COFF) from the shared driver. Uses llvm-config from the
    #    fork tree to enumerate the static archives (like the Linux build); links
    #    lld's COFF+Common static libs + LLVM static libs. clang++ drives the link.
    #    llvm-config on Windows emits libraries MSVC-style ("psapi.lib", "LLVMCore.lib")
    #    — but clang++ (a GNU-style driver) treats a bare "foo.lib" as an INPUT FILE
    #    and fails to open it. Convert each bare "foo.lib" token to "-lfoo" so clang++
    #    SEARCHES for it (LLVM libs under -L, Windows SDK libs via clang's MSVC
    #    detection); leave absolute paths and existing -l/-L flags untouched. This is
    #    the Windows/Linux link-name difference the CLang bridge also handles.
    function ConvertTo-LinkFlags([string]$s) {
        $s.Split(' ') | Where-Object { $_ } | ForEach-Object {
            if ($_ -match '^[A-Za-z0-9_.+-]+\.lib$') { '-l' + ($_ -replace '\.lib$', '') } else { $_ }
        }
    }
    $llvmConfig = Join-Path $llvmRoot 'bin\llvm-config.exe'
    $clang = (Get-Command clang++ -ErrorAction SilentlyContinue).Source
    if (-not $clang) { $clang = Join-Path $llvmRoot 'bin\clang++.exe' }
    if ((Test-Path $llvmConfig) -and (Test-Path $clang) -and (Test-Path (Join-Path $llvmRoot 'lib\lldCOFF.lib'))) {
        Write-Host "building $OUT/fly-lld.exe (lld static, LLVM dynamic) ..."
        $llvmLibs = ConvertTo-LinkFlags ((& $llvmConfig --link-static --libs all) -join ' ') | Where-Object { $_ -notmatch 'olly' }
        $sysLibs  = ConvertTo-LinkFlags ((& $llvmConfig --link-static --system-libs) -join ' ')
        & $clang ci/linux/lld_driver.cpp -std=c++17 `
            "-I$(Join-Path $llvmRoot 'include')" `
            "-L$(Join-Path $llvmRoot 'lib')" -llldCOFF -llldCommon `
            @llvmLibs @sysLibs `
            -o "$OUT/fly-lld.exe"
        # Non-fatal: bundling the linker is best-effort. If the clang link fails
        # (SDK/toolchain quirks that only surface on the runner), ship fly.exe +
        # LLVM-C.dll anyway — ToolChain.fly then falls back to a system lld-link/link.
        if ($LASTEXITCODE -ne 0) {
            Write-Host "warning: fly-lld.exe link failed (exit $LASTEXITCODE); shipping without a"
            Write-Host "         bundled linker — fly.exe falls back to a system lld-link/link."
            Remove-Item "$OUT/fly-lld.exe" -Force -ErrorAction SilentlyContinue
        }
    } else {
        Write-Host "warning: fly-lld.exe not built (missing llvm-config/clang/lld static libs in $llvmRoot);"
        Write-Host "         the released fly.exe will fall back to a system linker (lld-link/link)."
    }
}

# -- 4) Cleanup: bin/ ships the executable (+ bundled DLL/linker); drop the staging
#        dir and any intermediate objects.
Remove-Item $STAGE -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item "$OUT/*.o" -Force -ErrorAction SilentlyContinue

Write-Host "fly -> $OUT/fly"
Write-Host "std -> $LIB (fly_std_lib.lib + *.fly.h + runtime)"
