# -----------------------------------------------------------------------------
# build_lsp.ps1 - build the fly-lsp language server into build\stage$STAGE\bin.
#
# This is the SECOND executable in the tree, and it exists because of --entry:
# `--src-dir compiler/lib` makes the compiler's namespaces importable, but that
# directory also declares driver/Driver.fly's main(). Discovery would then see
# two entry points and stop with "multiple main() functions found". --entry names
# the entry outright, so discovery never runs; the import closure of the named
# file still pulls what it needs from every --src-dir root.
#
# MONOLITHIC, like build_compiler.ps1: the compiler is compiled FROM SOURCE into
# the LSP object rather than linked as an archive. A compiler static lib let the
# linker COMDAT-dedup its generic instantiations against the consumer's own
# copies, which produced a use-after-free (root-caused 2026-07-19).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

# Written back into the ENVIRONMENT, not just a local: link_bin.ps1 derives its
# own lib dir from $env:STAGE and defaults to 1, so a standalone run of this
# script would otherwise compile the object at stage 2 and link it against the
# stage-1 libraries.
$STAGE = if ($env:STAGE) { $env:STAGE } else { '2' }
$env:STAGE = $STAGE
$OUT = "build/stage$STAGE/bin"
$LIB = "build/stage$STAGE/lib"
$D = "build/stage$STAGE/lsp"
New-Item -ItemType Directory -Force $OUT, $D | Out-Null

# The compiler that BUILDS the LSP is THIS STAGE'S OWN fly.exe, not the seed the
# rest of the stage uses. fly-lsp is a PRODUCT of the toolchain, not part of the
# bootstrap: nothing downstream compiles against it, so there is no fixpoint to
# preserve. It also has to be this way - --entry is a self-host option and the
# pinned 0.13.x seed rejects it outright ("unknown option: --entry").
$FLY = "build/stage$STAGE/bin/fly.exe"
if (-not (Test-Path $FLY)) { Write-Host "error: compiler '$FLY' not found - run link_fly.ps1 for this stage first."; exit 1 }
if (-not (Test-Path 'tools/lsp/lib/FlyLsp.fly')) { Write-Host 'error: tools/lsp/lib/FlyLsp.fly not found.'; exit 1 }

$DBG = @(); if ($env:FLY_DEBUG_SYMBOLS -eq '1') { $DBG = @('--debug-symbols') }
$TARGET = @(); if ($env:FLY_TARGET_ARGS) { $TARGET = $env:FLY_TARGET_ARGS -split ' ' }

Write-Host "stage${STAGE}: compiling tools/lsp (monolithic, compiler from source) ..."
& $FLY --entry tools/lsp/lib/FlyLsp.fly --src-dir tools/lsp --src-dir compiler/lib `
       @DBG @TARGET -c -o FlyLsp --out-dir $D -L $LIB
if ($LASTEXITCODE -ne 0) { throw "fly-lsp compile failed (exit $LASTEXITCODE)" }

# The emitted object's name varies by producer, as in build_compiler.ps1.
$OBJ = @("$D/FlyLsp", "$D/FlyLsp.o", "$D/FlyLsp.fly.o", "$D/FlyLsp.fly.obj") |
       Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $OBJ) { Write-Host "error: fly-lsp object not emitted in $D."; exit 1 }
if ($OBJ -ne "$D/FlyLsp.o") { Move-Item $OBJ "$D/FlyLsp.o" -Force }

# -WithLLVM: the import closure reaches the compiler's CodeGen through Sema, so
# the LLVM-C symbols must resolve even though the LSP never emits code.
& "$PSScriptRoot\link_bin.ps1" -Obj "$D/FlyLsp.o" -Out "$OUT/fly-lsp.exe" -WithLLVM
if ($LASTEXITCODE -ne 0) { throw "fly-lsp link failed (exit $LASTEXITCODE)" }

Write-Host "stage${STAGE}: fly-lsp -> $OUT/fly-lsp.exe"
exit 0
