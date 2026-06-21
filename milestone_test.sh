#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# milestone_test.sh — end-to-end runnable milestones. Each codegen test program
# emits a .ll under /tmp/cg/; this harness clang-links that IR into a native
# executable and asserts its exit code. This is the real "compile + link + run"
# proof the self-host CodeGen produces working programs (object emission via the
# LLVM target-machine API is blocked by the CLang bridge's 2-output limitation,
# so we emit textual IR and let clang lower + link it).
# ─────────────────────────────────────────────────────────────────────────────
set -uo pipefail
cd "$(dirname "$0")"

FLY="${FLY:-../fly/cmake-build-relwithdebinfo/bin/fly}"
CLANG="${CLANG:-clang}"
OUT=/tmp/cgtest
mkdir -p "$OUT" /tmp/cg

# runnable milestones: "<test-basename> <expected-exit-code>"
TESTS=(
    "stage4 42"
    "stage5 42"
    "stage5b 11"
    "stage6 33"
    "stage6b 5"
    "stage8 42"
    "stage10 42"
)

pass=0
fail=0
for entry in "${TESTS[@]}"; do
    set -- $entry
    name=$1
    expect=$2
    src="test/codegen/${name}_main.fly"
    log="$OUT/_m_$name.log"

    # 1) run the codegen test program → emits /tmp/cg/<name>.ll
    if ! "$FLY" "$src" --src-dir compiler -o "$name" --out-dir "$OUT" -L std/lib >"$log" 2>&1; then
        echo "  COMPILE FAIL  $name"; grep -iE 'error|broken' "$log" | head -3 | sed 's/^/      /'; fail=$((fail + 1)); continue
    fi
    if ! "$OUT/$name" >>"$log" 2>&1; then
        echo "  EMIT FAIL     $name (codegen program exit $?)"; fail=$((fail + 1)); continue
    fi
    # 2) clang-link the emitted IR + run
    if ! "$CLANG" "/tmp/cg/${name}.ll" -o "/tmp/cg/${name}prog" 2>>"$log"; then
        echo "  LINK FAIL     $name"; grep -iE 'error' "$log" | head -3 | sed 's/^/      /'; fail=$((fail + 1)); continue
    fi
    "/tmp/cg/${name}prog"
    code=$?
    if [ "$code" -eq "$expect" ]; then
        echo "  PASS          $name (exit $code)"
        pass=$((pass + 1))
    else
        echo "  RUN  FAIL     $name (exit $code, expected $expect)"
        fail=$((fail + 1))
    fi
done

echo "─────────────────────────────────────────────"
echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
