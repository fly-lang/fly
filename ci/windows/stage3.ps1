# -----------------------------------------------------------------------------
# stage3.ps1 - fixpoint check of the staged bootstrap (see the stage map in
# stage1.ps1; Linux mirror: ci/linux/stage3.sh). CI runs it on every build.
#
#   stage1  built by the SEED (the previous release)   -> self-host #1
#   stage2  built by stage1 (self-host #1)             -> the SHIPPED compiler
#   stage3  built by stage2 (the shipped compiler)     -> THIS SCRIPT
#
# stage2 and stage3 are compiled from the SAME sources; they differ only in the
# compiler that compiled them. If the shipped compiler generates the same code
# as the compiler that built it, the two outputs are byte-identical - the
# canonical bootstrap fixpoint (GCC: `cmp stage2 stage3`). A difference means
# the self-host's codegen depends on WHICH compiler built it: a latent codegen
# bug that the suites - which only ever run one compiler - cannot see.
#
# From 0.16 stage 3 is the same `fly build` as stage 2 (build_stage2.ps1), for
# the compiler target only, run by the stage-2 fly into build\stage3:
#
#   build\stage2\bin\fly.exe build --profile dist[-debug] --out-dir build\stage3 --target fly --keep-objects
#
# build\stage3\lib starts as a copy of build\stage2\lib, so the compiler links
# and reads headers exactly as at stage 2. What is compared: the compiler's
# OBJECT, bin\fly.o (stage 2 keeps its own in build\stage2\obj), the output of
# the compile before the link - the compiler's own product; the linked exe adds
# what the linker records (on Mach-O the path of that object). The profile's
# prefix map writes the package root, the compiling fly's lib\ and the output
# directory the same way at both stages, so the two can be compared at all.
#
# stage3 does NOT touch build\stage2 (that is what ships). On a mismatch it
# builds ONCE MORE with the same stage2 compiler to tell apart the two causes:
# stage3a != stage3b means the compiler is not deterministic (no verdict - fix
# that first), stage3a == stage3b != stage2 means a genuine fixpoint failure.
#
# Exit codes: 0 fixpoint holds, 1 fixpoint broken or stage3 failed to build,
#             2 compiler non-deterministic (no verdict).
# FLY_DEBUG_SYMBOLS must be the same as when stage2 was built (CI passes the
# same env to both).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))

$FLY2 = 'build/stage2/bin/fly.exe'
$OBJ2 = 'build/stage2/obj/fly.o'
foreach ($need in $FLY2, $OBJ2) {
    if (-not (Test-Path $need -PathType Leaf)) {
        Write-Host "error: '$need' missing - run ci\windows\stage2.ps1 first."; exit 1
    }
}
$FLY_PROFILE = if ($env:FLY_DEBUG_SYMBOLS -eq '1') { 'dist-debug' } else { 'dist' }

# Build-Compiler DIR - the compiler target, built by the stage-2 fly into DIR;
# returns the path of its object.
function Build-Compiler([string]$dir) {
    Remove-Item $dir -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force $dir | Out-Null
    Copy-Item 'build/stage2/lib' "$dir/lib" -Recurse -Force
    & $FLY2 build --profile $FLY_PROFILE --out-dir $dir --target fly --keep-objects > "$dir/build.log" 2>&1
    if (($LASTEXITCODE -ne 0) -or -not (Test-Path "$dir/bin/fly.o")) {
        Write-Host "error: stage3 compiler not built; see $dir\build.log:"
        Get-Content "$dir/build.log" | Select-String 'error' | Select-Object -First 5 | ForEach-Object { Write-Host "      $_" }
        exit 1
    }
    return "$dir/bin/fly.o"
}

function Sha([string]$p) { (Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash.ToLowerInvariant() }

Write-Host "stage3: compiling the compiler with the stage2 fly (fixpoint check, profile $FLY_PROFILE) ..."
$sw = [Diagnostics.Stopwatch]::StartNew()
$obj3 = Build-Compiler 'build/stage3'
Write-Host ("stage3: built in {0:n0}s" -f $sw.Elapsed.TotalSeconds)
$h2 = Sha $OBJ2; $h3 = Sha $obj3
Write-Host "  stage2 fly.o  $h2  ($((Get-Item $OBJ2).Length) bytes)"
Write-Host "  stage3 fly.o  $h3  ($((Get-Item $obj3).Length) bytes)"
if ($h2 -eq $h3) {
    Write-Host "stage3: OK - fixpoint holds (stage2 == stage3, byte-identical object)."
    exit 0
}

Write-Host "stage3: objects differ - building once more to classify ..."
$obj3b = Build-Compiler 'build/stage3_repeat'
$h3b = Sha $obj3b
Write-Host "  stage3' fly.o $h3b  ($((Get-Item $obj3b).Length) bytes)"
if ($h3 -ne $h3b) {
    Write-Host "stage3: INCONCLUSIVE - the stage2 compiler is not deterministic (two runs on the same input differ)."
    Write-Host "        make it deterministic first; compare build\stage3\bin\fly.o and build\stage3_repeat\bin\fly.o."
    exit 2
}
$a = [IO.File]::ReadAllBytes((Resolve-Path $OBJ2)); $b = [IO.File]::ReadAllBytes((Resolve-Path $obj3))
$n = [Math]::Min($a.Length, $b.Length); $at = -1
for ($i = 0; $i -lt $n; $i++) { if ($a[$i] -ne $b[$i]) { $at = $i; break } }
Write-Host "stage3: FAILED - the fixpoint is broken: the stage2 compiler generates different code than the"
Write-Host "        stage1 compiler that built it, from the same sources."
Write-Host ("        first differing byte at offset {0} (sizes {1} vs {2})." -f $(if ($at -ge 0) { $at } else { $n }), $a.Length, $b.Length)
Write-Host "        inspect: llvm-objdump -d / -t on build\stage2\obj\fly.o vs build\stage3\bin\fly.o"
exit 1
