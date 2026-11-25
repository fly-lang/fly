# -----------------------------------------------------------------------------
# stage2.ps1 - stage 2 of the staged bootstrap (see the stage map in stage1.ps1):
# the fly built by stage1 (build\stage1\bin\fly.exe) produces the SHIPPED
# artifacts into build\stage2 - what CI uploads and the release packages.
#
# From 0.16 the whole stage is one `fly build` on the root Manifest.fly:
#
#   build\stage1\bin\fly.exe build --profile dist[-debug] --out-dir build\stage2 --keep-objects
#
# runtime, TLS archives, std, compiler, fly-lsp and fly-registry, all compiled
# by the stage-1 fly, plus the LLVM pieces stage 0 fetched (LLVM-C, ld.lld,
# lldb, the mingw sysroot, llvm.fly.h), laid out as the release: bin\ and lib\.
# The profile is `dist-debug` when FLY_DEBUG_SYMBOLS=1 (CI on push: DWARF, so a
# crash symbolizes), `dist` otherwise (the release). Up to 0.15 the stage ran
# one hand-written script per artifact; the Manifest now says what they said.
#
# --keep-objects leaves each executable's object next to it; they are moved to
# build\stage2\obj (outside what the release packages): stage 3 compiles the
# compiler again with the stage-2 fly and compares its object with obj\fly.o.
#
# The build itself is build_stage2.ps1, shared with repro_check.ps1.
#
# A failed stage-2 build is FATAL, exactly as in ci/linux/stage2.sh: there is
# no fallback to the stage-1 artifacts. build\stage2 is what the release
# packages verbatim: it must be the stage-2 build or not exist.
# -----------------------------------------------------------------------------
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
Set-Location (Resolve-Path (Join-Path $PSScriptRoot '..\..'))
$env:STAGE = '2'

& "$PSScriptRoot\build_stage2.ps1"
if ($LASTEXITCODE -ne 0) { Write-Host "error: build_stage2.ps1 failed (exit $LASTEXITCODE)"; exit 1 }

# -- the guards: properties of the artifact no suite can see ---------------------
#
# The tools are PRODUCTS of the toolchain and ship from build\stage2\bin; a tool
# failure here is a real failure of the artifact.
# test_dbg: the debugger is PROVISIONED (copied by the Manifest's copy targets
# from the fork LLVM), not compiled - but it ships from build\stage2\bin like
# the tools, so it is verified here with them.
# test_abi: the calling conventions of the compiler that ships must match the
# seed's (a library and a program compiled by different releases link together).
# test_sema: the compiler must still REJECT what the language forbids. No suite
# can see a check that went missing - suites only compile valid programs.
# test_traps: a run-time error (a null reference) must stop the program with its
# diagnostic. No suite can see a trap that went missing - suites run valid programs.
# test_heap: the object heap reports, at the end of main, the objects a program
# never deleted, and releases them (fly-lang/fly#235).
# test_fly_build: build\stage2 is a toolchain of its own (bare PATH, no option),
# and a project built with `fly build` reads its own manifest.
foreach ($step in 'test_lsp', 'test_registry', 'test_tools', 'test_abi', 'test_sema', 'test_traps', 'test_heap',
                  'test_leaks', 'test_dbg', 'test_fly_build') {
    & ".\ci\windows\$step.ps1"
    if ($LASTEXITCODE -ne 0) { Write-Host "error: $step.ps1 failed (exit $LASTEXITCODE)"; exit 1 }
}

Write-Host "stage2: done - build\stage2\bin\fly.exe"
exit 0
