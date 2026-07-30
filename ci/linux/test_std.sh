#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_std.sh — run every std/test/**/*Suite.fly. These are `suite`/`case` programs
# that exercise the standard library, grouped in subdirectories:
#   data/  fly.data.* containers     core/  fly.str, fly.math, fly.mem, fly.bridge
#   os/    fly.os.*                  lang/  casts, enums, generics, interfaces,
#   meta/  fly.meta schema                  override/super/deep inheritance
# Each is compiled against the std archive via -L and driven with --suite: fly
# builds the suite executable AND runs it, exiting with the run's code. The
# per-case report (`    <case> ... FAIL(<code>): <msg>`, then the
# `suite <Name>: N cases, ...` summary) lands in the captured log, so a failure
# names the exact assertion.
#
# Scope: ONLY std/test. Compiler, driver and tool suites run in their own scripts.
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
# Scripts live in ci/linux/; operate from the project root (two levels up).
cd "$(dirname "$0")/../.."

# Scratch for per-test binaries/logs; under build/ but outside the stage dirs.
OUT=build/test
STD=std/lib
mkdir -p "$OUT"

# Compiler under test: $FLY (default: the compiler for $STAGE — see below; the
# artifact that ships). The compiler derives its stdlib dir from its own exe path
# (argv[0]), so the value is resolved to an absolute path here (a slash-less
# override is resolved through PATH).
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

# ── Selector (mirror of test_std.ps1) ─────────────────────────────────────────
# Optional $1 (or $FLY_TEST_SUITE): a suite name, a qualified ns.SuiteName, or a
# NAMESPACE — matched like the compiler's --suite=<sel>. Empty = every suite.
SEL="${1:-${FLY_TEST_SUITE:-}}"

suite_sel_match() {                      # $1=file $2=basename → 0 when selected
    [ -z "$SEL" ] && return 0
    [ "$2" = "$SEL" ] && return 0
    ns=$(grep -m1 -E '^namespace +[A-Za-z0-9_.]+' "$1" | sed -E 's/^namespace +//;s/[^A-Za-z0-9_.].*$//')
    [ -z "$ns" ] && return 1             # namespace-less: bare name only
    [ "$ns" = "$SEL" ] && return 0
    [ "$ns.$2" = "$SEL" ] && return 0
    case "$ns" in "$SEL".*) return 0 ;; esac
    return 1
}

selected=""
for t in $(find std/test -name '*Suite.fly' | sort); do
    name=$(basename "$t" .fly)
    if suite_sel_match "$t" "$name"; then selected="$selected $t"; fi
done
if [ -z "$(echo "$selected" | tr -d ' ')" ]; then
    echo "error: no suite or namespace matches '$SEL' under std/test" >&2
    exit 1
fi

# ── Run mode (mirror of test_std.ps1) ─────────────────────────────────────────
# ONE-SHOT (default for the self-host stages): one `--suite[=SEL]` build — the
# std tree compiles ONCE into a single test binary (unblocked by the B036 fix:
# identity-strong specialization keys). Reports are DEDUPED by suite name with
# the worst failure kept: OsProcSuite's spawnSuccessTest re-runs the binary as
# a child BY DESIGN, duplicating every report line. Per-suite loop kept for
# STAGE=0 (the pinned seed) and FLY_TEST_PER_SUITE=1 (debugging).
if [ "$STAGE" != "0" ] && [ "${FLY_TEST_PER_SUITE:-}" != "1" ]; then
    log="$OUT/_std_oneshot.log"
    if [ -n "$SEL" ]; then
        "$FLY" --suite="$SEL" --src-dir std -o std_all --out-dir "$OUT" -L "$STD" >"$log" 2>&1
    else
        "$FLY" --suite --src-dir std -o std_all --out-dir "$OUT" -L "$STD" >"$log" 2>&1
    fi
    code=$?

    pass=0
    fail=0
    reported=""
    # dedup: sort by name then TAKE THE WORST (max failed) per suite
    while IFS=' ' read -r name nfail; do
        [ -z "$name" ] && continue
        reported="$reported $name"
        if [ "$nfail" = "0" ]; then
            echo "  PASS          $name"
            pass=$((pass + 1))
        else
            echo "  RUN  FAIL     $name ($nfail failed)"
            fail=$((fail + 1))
        fi
    done < <(grep -E '^suite [A-Za-z0-9_]+: [0-9]+ cases, [0-9]+ passed, [0-9]+ failed' "$log" \
             | sed -E 's/^suite ([A-Za-z0-9_]+):.*, ([0-9]+) failed$/\1 \2/' \
             | sort -k1,1 -k2,2nr | sort -u -k1,1 || true)

    if [ -z "$(echo "$reported" | tr -d ' ')" ]; then
        echo "  COMPILE FAIL  (exit $code) - $log"
        grep -m6 -E 'error:|broken|abort' "$log" | sed 's/^/      /' || tail -6 "$log" | sed 's/^/      /'
        exit 1
    fi

    if [ "$fail" -gt 0 ]; then
        grep -m12 -E 'FAIL\(' "$log" | sed 's/^/      /' || true
    fi

    missing=0
    for t in $selected; do
        name=$(basename "$t" .fly)
        case " $reported " in
            *" $name "*) ;;
            *) echo "  NO REPORT     $name (run aborted before it? exit $code)"; missing=$((missing + 1)) ;;
        esac
    done

    echo "─────────────────────────────────────────────"
    echo "  $pass passed, $fail failed, $missing unreported (one-shot, exit $code)"
    [ "$fail" -eq 0 ] && [ "$missing" -eq 0 ] && [ "$code" -eq 0 ]
    exit $?
fi

pass=0
fail=0
for t in $selected; do
    name=$(basename "$t" .fly)
    log="$OUT/_std_$name.log"
    # One-shot: --suite compiles AND runs; fly's exit code is the run's code (or
    # the compile failure). DIRECTORY CLI (every stage): the suite is discovered
    # by name from the source root — `std`, not std/test, so the fly.meta SOURCE
    # under std/lib/meta stays pullable (suite names also repeat across trees:
    # ManifestSuite exists in compiler/test too).
    if "$FLY" --suite="$name" --src-dir std -o "std_$name" --out-dir "$OUT" -L "$STD" >"$log" 2>&1; then
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
            # warnings like 'errorHandler'; -m3 avoids the "Broken pipe" noise.
            grep -m3 -E 'error:|broken|abort' "$log" | sed 's/^/      /'
        fi
        fail=$((fail + 1))
    fi
done

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
