# -----------------------------------------------------------------------------
# stage3.ps1 - OPTIONAL fixpoint check of the staged bootstrap (see the stage map
# in stage1.ps1; Linux mirror: ci/linux/stage3.sh). Run only at release time.
#
#   stage1  built by the SEED (C++ reference)          -> self-host #1
#   stage2  built by stage1 (self-host #1)             -> the SHIPPED compiler
#   stage3  built by stage2 (the shipped compiler)     -> THIS SCRIPT
#
# stage2 and stage3 are compiled from the SAME sources; they differ only in the
# compiler that compiled them. If the shipped compiler generates the same code
# as the compiler that built it, the two outputs are byte-identical - the
# canonical bootstrap fixpoint (GCC: `cmp stage2 stage3`). A difference means
# the self-host's codegen depends on WHICH compiler built it: stage1 (seed-built)
# and stage2 (self-built) miscompile or optimise differently, i.e. a latent
# codegen bug that the suites - which only ever run one compiler - cannot see.
#
# What is compared: the merged driver+compiler OBJECT (build\stage<N>\driver\
# Driver.o), the output of `fly -c` before the link. That is the compiler's own
# deterministic product; the linked fly.exe adds linker-chosen layout and (on
# Windows) a PE timestamp that say nothing about codegen.
#
# stage3 does NOT touch build\stage2 (that is what ships) - everything goes to
# build\stage3. It needs a finished stage 2 (stage2.ps1) and costs one more
# compile of the compiler (~minutes), which is why it is not part of the normal
# pipeline: CI runs it only from release.yml (build-*.yml `stage3: "1"`).
#
# On a mismatch it recompiles ONCE MORE with the same stage2 compiler to tell
# apart the two causes: stage3a != stage3b means the compiler is not
# deterministic (the check cannot give a verdict - fix that first), stage3a ==
# stage3b != stage2 means a genuine fixpoint failure.
#
# Exit codes: 0 fixpoint holds, 1 fixpoint broken or stage3 failed to build,
#             2 compiler non-deterministic (no verdict).
# FLY_DEBUG_SYMBOLS must be the same as when stage2 was built (CI passes the
# same env to both).
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
. "$PSScriptRoot\gnu_common.ps1"

$FLY2 = 'build/stage2/bin/fly.exe'
$OBJ2 = 'build/stage2/driver/Driver.o'
foreach ($need in $FLY2, $OBJ2) {
    if (-not (Test-Path $need -PathType Leaf)) {
        Write-Host "error: '$need' missing - run ci\windows\stage2.ps1 first."; exit 1
    }
}
$DBG = @(); if ($env:FLY_DEBUG_SYMBOLS -eq '1') { $DBG += '--debug-symbols' }

# Compile the compiler with the STAGE-2 fly into $dir; return the object path.
# Same invocation as build_compiler.ps1 STAGE=2 (self-host: -c, no link).
function Build-Driver([string]$dir) {
    Remove-Item $dir -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force $dir | Out-Null
    & $FLY2 --src-dir compiler @DBG @FLY_TARGET_ARGS -c -o Driver --out-dir $dir > "$dir/emit.log" 2>&1
    $obj = @("$dir/Driver", "$dir/Driver.fly.o", "$dir/Driver.fly.obj") | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $obj) {
        Write-Host "error: stage3 driver object not emitted; see $dir\emit.log:"
        Get-Content "$dir/emit.log" | Select-String 'error:' | Select-Object -First 5 | ForEach-Object { Write-Host "      $_" }
        exit 1
    }
    Move-Item $obj "$dir/Driver.o" -Force
    return "$dir/Driver.o"
}
function Sha([string]$p) { (Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash.ToLowerInvariant() }

Write-Host "stage3: compiling driver + compiler with the stage2 fly (fixpoint check) ...$(if ($DBG) { ' (+debug-symbols)' })"
$sw = [Diagnostics.Stopwatch]::StartNew()
$obj3 = Build-Driver 'build/stage3/driver'
Write-Host ("stage3: built in {0:n0}s" -f $sw.Elapsed.TotalSeconds)

$h2 = Sha $OBJ2; $h3 = Sha $obj3
Write-Host "  stage2 Driver.o  $h2  ($((Get-Item $OBJ2).Length) bytes)"
Write-Host "  stage3 Driver.o  $h3  ($((Get-Item $obj3).Length) bytes)"
if ($h2 -eq $h3) {
    Write-Host "stage3: OK - fixpoint holds (stage2 == stage3, byte-identical object)."
    exit 0
}

# Mismatch: is it the compiler's codegen, or non-determinism?
Write-Host "stage3: objects differ - recompiling once more to classify ..."
$obj3b = Build-Driver 'build/stage3/driver_repeat'
$h3b = Sha $obj3b
Write-Host "  stage3' Driver.o $h3b  ($((Get-Item $obj3b).Length) bytes)"
if ($h3 -ne $h3b) {
    Write-Host "stage3: INCONCLUSIVE - the stage2 compiler is not deterministic (two runs on the same input differ)."
    Write-Host "        make it deterministic first; compare build\stage3\driver and build\stage3\driver_repeat."
    exit 2
}
# Deterministic, so the difference is real: say where it starts.
$a = [IO.File]::ReadAllBytes((Resolve-Path $OBJ2)); $b = [IO.File]::ReadAllBytes((Resolve-Path $obj3))
$n = [Math]::Min($a.Length, $b.Length); $at = -1
for ($i = 0; $i -lt $n; $i++) { if ($a[$i] -ne $b[$i]) { $at = $i; break } }
Write-Host "stage3: FAILED - the fixpoint is broken: the stage2 compiler generates different code than the"
Write-Host "        stage1 compiler that built it, from the same sources."
Write-Host ("        first differing byte at offset {0} (sizes {1} vs {2})." -f $(if ($at -ge 0) { $at } else { $n }), $a.Length, $b.Length)
Write-Host "        inspect: llvm-objdump -d / -t on build\stage2\driver\Driver.o vs build\stage3\driver\Driver.o"
exit 1
