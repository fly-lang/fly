##===-- std/math/CMakeLists.txt - fly.math build targets ---------------===##
#
# Targets
#   fly_math         — static library (freestanding, no system headers)
#   fly_math_test    — freestanding test binary (links fly_math + libm)
#   fly_math_inspect — emits fly_math.ll and greps for LLVM intrinsics
#
# All library code compiled with -nostdinc -nostdlib; only Clang builtins
# and fly_math.h types are used.  libm is linked by the TEST only, to
# supply implementations for the transcendental intrinsics at run time.
##===-------------------------------------------------------------------===##

# ── Library ──────────────────────────────────────────────────────────────────

add_library(fly_math STATIC fly_math.c)

target_include_directories(fly_math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(fly_math PRIVATE
    -nostdinc
    -O2
    -Wall
    -Wextra
    -Werror
    -fno-math-errno      # allow __builtin_sin etc. to emit llvm.sin.f64
    -fno-trapping-math   # remove trapping semantics; enables more intrinsics
    -mpopcnt             # emit hardware popcnt instruction, avoids __popcountdi2
)

set_target_properties(fly_math PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ── Freestanding test binary ─────────────────────────────────────────────────

add_executable(fly_math_test fly_math_test.c)

target_include_directories(fly_math_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(fly_math_test PRIVATE
    -nostdinc
    -O2
    -Wall
    -Wextra
    -fno-math-errno
    -fno-trapping-math
    -fno-stack-protector
    -mpopcnt
)

# -nostdlib/-nostartfiles: no CRT; _start in fly_math_test.c is the entry point.
# Stack is force-aligned to 16 bytes inside the naked _start before any calls.
# -lm: supplies implementations for the LLVM transcendental intrinsics at link time.
target_link_options(fly_math_test PRIVATE -nostdlib -nostartfiles)
target_link_libraries(fly_math_test PRIVATE fly_math m c)

add_test(NAME fly_math_test COMMAND fly_math_test)

# ── IR inspection target ──────────────────────────────────────────────────────
#
# Usage:  cmake --build . --target fly_math_inspect
# Emits fly_math.ll in the build directory and greps for LLVM intrinsics.
# A non-zero exit here means the expected intrinsics are missing and the
# affected function should be moved to std/math/fly_math.ll (Level-2).

add_custom_target(fly_math_inspect
    COMMAND ${CMAKE_C_COMPILER}
            -S -emit-llvm -O2 -nostdinc -nostdlib
            -fno-math-errno -fno-trapping-math
            -I${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/fly_math.c
            -o ${CMAKE_CURRENT_BINARY_DIR}/fly_math_inspect.ll
    COMMAND ${CMAKE_COMMAND} -E echo "=== LLVM intrinsics found in fly_math ==="
    COMMAND grep -o "llvm\\.[a-z0-9._]*" ${CMAKE_CURRENT_BINARY_DIR}/fly_math_inspect.ll
            | sort -u
    COMMENT "Inspecting fly_math.c for LLVM intrinsics"
    VERBATIM
)
