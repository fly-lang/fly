#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# test_runtime.sh — run every runtime/test/*Suite.fly. These are `suite`/`case`
# programs exercising the Fly runtime (fly.runtime: the libc/libm FFI backend +
# the fly.os wrappers over it). Each is compiled against the std + runtime
# archives via -L and driven with --suite: fly builds the suite executable AND
# runs it, exiting with the run's code; the per-case FAIL(<code>): <msg> report
# lands in the captured log.
#
# The runtime is platform-specific (fly.runtime.* links the HOST runtime), so this
# runs ONLY the host suite: it SKIPS foreign-platform suites (RuntimeWindows* /
# RuntimeMacos*) so RuntimeLinuxSuite is what runs on Linux.
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
#   STAGE=0  the pinned REFERENCE seed that stage0 downloaded. The suites
#            compile the in-tree std sources (-L), so a failure here is a
#            SOURCE-level problem (from 0.13.14 the seed ships no std of its own).
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

# Seed link extras. The self-host driver links the fly_tls_stub archive (tls_*
# primitives) by itself; the reference seed only auto-links
# fly_std_lib/fly_runtime_lib from <exe>/../lib — so under the seed (stage1's
# interleaved pass) a tls-touching suite would not link. Both drivers DO link
# every archive at the top level of a -L dir, so stage the stub ALONE in a
# scratch dir (alone: fly_tls_lib next to it is the real backend with
# Schannel/OpenSSL system deps, and must not race the stub for extraction).
EXTRA_L=""
FLY_LIB_DIR="$(dirname "$FLY")/../lib"
for ext in a lib; do
    if [ -f "$FLY_LIB_DIR/fly_tls_stub.$ext" ]; then
        mkdir -p "$OUT/_seed_link"
        cp -f "$FLY_LIB_DIR/fly_tls_stub.$ext" "$OUT/_seed_link/"
        EXTRA_L="-L $OUT/_seed_link"
        break
    fi
done

pass=0
fail=0
found=0
for t in $(find runtime/test -name '*Suite.fly' 2>/dev/null | sort); do
    name=$(basename "$t" .fly)
    # Skip foreign-platform suites (their osname/arch assertions target another OS).
    case "$name" in *Windows*|*Macos*) continue ;; esac
    found=$((found + 1))
    log="$OUT/_rt_$name.log"
    # One-shot: --suite compiles AND runs; fly's exit code is the run's code (or
    # the compile failure). --suite stays LAST: its value is optional, so in the
    # reference CLI a following positional would be swallowed as the suite name.
    # DIRECTORY CLI (every stage): the suite is discovered by name from the
    # runtime/test root.
    if "$FLY" --suite="$name" --src-dir runtime/test -o "rt_$name" --out-dir "$OUT" -L "$STD" $EXTRA_L >"$log" 2>&1; then
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
            grep -m3 -E 'error:|broken|abort' "$log" | sed 's/^/      /'
        fi
        fail=$((fail + 1))
    fi
done

echo "─────────────────────────────────────────────"
if [ "$found" -eq 0 ]; then
    echo "  no runtime/test/*Suite.fly found for this platform"
fi
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
