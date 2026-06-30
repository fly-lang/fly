# -----------------------------------------------------------------------------
# build_compiler.ps1 - build the self-host compiler EXECUTABLE `fly` (Windows).
#
# PowerShell port of build_compiler.sh. The entry is compiler/Fly.fly (which has
# `void main`); without --lib the reference toolchain links an executable.
# --src-dir indexes the whole fly.compiler graph (driver -> frontend ->
# parser/sema/codegen) and pulls in every file - that's fine: each `main` is in
# its own namespace, so the entry file's main becomes the C entry without clashing
# with test mains. -L resolves std.
#
# This is the single build script: compiling the executable compile-checks every
# compiler source (and links), so it subsumes the old --lib-only build.
# -----------------------------------------------------------------------------

$ErrorActionPreference = 'Stop'
# Under `shell: pwsh` CI runners, pwsh 7.4 enables this preference, making a
# native non-zero exit throw before our own $LASTEXITCODE check. Disable it so the
# explicit check below is the sole arbiter (harmless no-op variable on PS 5.1).
$PSNativeCommandUseErrorActionPreference = $false
Set-Location $PSScriptRoot

# build/bin is the release-artifact path the workflows (build-windows.yml ->
# release.yml) upload and package; keep emitting the `fly` binary there.
$OUT = "build/bin"
$STD = "std/lib"
New-Item -ItemType Directory -Force $OUT | Out-Null

# Invoke the bootstrap compiler via $FLY (default: `fly` on PATH). CI sets FLY to
# an absolute path: released binaries derive their stdlib dir from the executable
# path, and a bare `fly` resolved from a cwd containing a `fly/` directory fails
# that lookup. A path-qualified value sidesteps it. See ../fly Driver.cpp.
$FLY = if ($env:FLY) { $env:FLY } else { "fly" }

& $FLY compiler/Fly.fly `
    --src-dir compiler `
    -o fly --out-dir $OUT -L $STD

# PowerShell does not abort on a native non-zero exit (no `set -e` equivalent for
# external programs), so surface the failure explicitly.
if ($LASTEXITCODE -ne 0) {
    throw "build failed: $FLY exited with code $LASTEXITCODE"
}

Write-Host "fly -> $OUT/fly"
