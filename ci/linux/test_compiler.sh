#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_compiler.sh — run every compiler/test/**/*Suite.fly against the compiler
# sources, without flyp. Single-file build: each suite is the entry; source
# discovery is implicit (a fly project compiles from the CURRENT directory — the
# repo root here), so the import graph pulls fly.compiler.*, fly.test.util, … into
# one module while std namespaces stay archive-linked (the -L pass registers them
# first). `--suite` builds the suite executable in test mode AND runs it in one
# shot — fly exits with the run's code and the per-case FAIL(<code>): <msg>
# report lands in the captured log; `--out-dir` sends the executable and its
# intermediate objects into $OUT.
#
# Scope: ONLY compiler/test (the compiler's own unit suites). The std, driver, and
# flyp suites run in their own scripts — see the note at the bottom of this file.
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

# Scratch for per-suite test binaries/logs; under build/ but separate from
# the stage dirs so it doesn't sit next to the release artifact.
OUT=build/test
STD=std/lib
mkdir -p "$OUT"

# CodeGen/target suites emit IR/objects to /tmp/cg and read them back (e.g.
# cgm.emitIR("/tmp/cg/suite.ll")). The directory must exist before they run —
# otherwise the LLVM file open fails and LLVMPrintModuleToFile/EmitToFile crash
# on the error path. Create it up front so a fresh checkout/CI runner passes.
mkdir -p /tmp/cg

# Compiler under test: $FLY (default: the compiler for $STAGE — see below; the
# that ships; its --test system was ported from the reference and all suites
# are green under it). The compiler derives its stdlib dir from its own
# executable path (argv[0]), so the value is resolved to an absolute path here
# (a slash-less override is resolved through PATH).
# -- Stage plumbing: WHICH compiler runs the tests. ----------------------------
# STAGE=N runs the suites with build/stageN's own compiler — each stage tests the
# compiler it just produced, so every step of the bootstrap is covered:
#   STAGE=0  the pinned REFERENCE seed that stage0 downloaded, with its bundled
#            runtime/std. A failure here is a SOURCE-level problem.
#   STAGE=1  the self-host stage1 just built WITH the reference. A failure here
#            that passed at 0 is the self-host's own codegen.
#   STAGE=2  the self-host stage2 just built WITH the self-host — the shipped
#            fixpoint artifact. A failure here that passed at 1 is stage2's codegen.
# So a suite that passes at N and fails at N+1 indicts the compiler stage N+1 built.
# Default 2 = the artifact that ships; use STAGE=1 for meaningful compiler-suite
# results while the stage2 binary is still miscompiled. $FLY overrides all.
STAGE="${STAGE:-2}"
STAGE_PREV="stage$STAGE"
STAGE_FLY="build/$STAGE_PREV/bin/fly"
FLY="${FLY:-$STAGE_FLY}"

# Both stages link the SAME fork LLVM, so a difference between the runs is never
# the LLVM underneath. stage0.sh exports LIBRARY_PATH through $GITHUB_ENV in CI; a
# local shell that did not source it still needs the dir, or `-lLLVM-20` fails.
if [ -d build/llvm/lib ]; then
    case ":${LIBRARY_PATH:-}:" in
        *":$PWD/build/llvm/lib:"*) ;;
        *) export LIBRARY_PATH="$PWD/build/llvm/lib:${LIBRARY_PATH:-}" ;;
    esac
    case ":${LD_LIBRARY_PATH:-}:" in
        *":$PWD/build/llvm/lib:"*) ;;
        *) export LD_LIBRARY_PATH="$PWD/build/llvm/lib:${LD_LIBRARY_PATH:-}" ;;
    esac
fi
case "$FLY" in
    /*) ;;                    # absolute — keep as given
    */*) FLY="$PWD/$FLY" ;;   # relative — anchor to the repo root
    *)
        FLY_RESOLVED="$(command -v "$FLY" || true)"
        if [ -z "$FLY_RESOLVED" ]; then
            echo "error: compiler '$FLY' not found on PATH." >&2
            exit 1
        fi
        FLY="$FLY_RESOLVED"
        ;;
esac
if [ ! -x "$FLY" ]; then
    echo "error: FLY='$FLY' is not an executable file — run ci/linux/$STAGE_PREV.sh first (or set FLY)." >&2
    exit 1
fi
if [ ! -d "$(dirname "$FLY")/../lib" ]; then
    echo "error: no lib/ directory next to '$FLY' (expected <exe_dir>/../lib" >&2
    echo "       with llvm.fly.h, runtime.fly.h, fly_runtime_lib.a)." >&2
    exit 1
fi

pass=0
fail=0
for suite in $(find compiler/test -name '*Suite.fly' | sort); do
    name=$(basename "$suite" .fly)
    log="$OUT/_$name.log"
    # One-shot: --suite compiles AND runs; fly's exit code is the run's code (or
    # the compile failure). --suite stays LAST: its value is optional, so in the
    # reference CLI a following positional would be swallowed as the suite name.
    # DIRECTORY CLI (every stage): the suite is discovered by name from the
    # compiler/ tree (suite names repeat across trees — ManifestSuite also
    # exists in std/test — and compiler/ as the root keeps the driver/compiler
    # imports resolving from source).
    if "$FLY" --suite="$name" --src-dir compiler -o "test_$name" --out-dir "$OUT" -L "$STD" >"$log" 2>&1; then
        echo "  PASS          $name"
        pass=$((pass + 1))
    else
        rc=$?
        # A `suite <Name>` line in the log means the compile succeeded and the
        # run got to the report: show the FAIL(<code>): <msg> cases and the
        # summary (log tail on a report-less crash). No report = compile broke.
        if grep -q '^suite ' "$log"; then
            echo "  RUN  FAIL     $name (exit $rc)"
            hits=$(grep -m6 -E 'FAIL\(|^suite .*:' "$log" || true)
            if [ -n "$hits" ]; then printf '%s\n' "$hits" | sed 's/^/      /'
            else tail -5 "$log" | sed 's/^/      /'; fi
        else
            echo "  COMPILE FAIL  $name (exit $rc)"
            # match real diagnostics ('error:'), not the substring "error" inside
            # warnings like 'errorHandler'; -m3 instead of |head avoids the
            # "grep: write error: Broken pipe" noise on every failure
            grep -m3 -E 'error:|broken|abort' "$log" | sed 's/^/      /'
        fi
        fail=$((fail + 1))
    fi
done

# The driver + package-manager suites now live under compiler/test/driver (the
# driver is compiled INTO the compiler monolithically), so they run here too -
# there is no separate test_driver step anymore.
# Sibling test scripts run the other targets as separate workflow steps:
#   test_std.sh        — std/test/**/*Suite.fly (std library)
#   test_runtime.sh    — runtime/test/*Suite.fly (per-platform runtime)

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
