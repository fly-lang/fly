##===-- std/str/CMakeLists.txt - fly.str build targets -----------------===##
#
# Targets
#   fly_str      — static library (freestanding, no system headers)
#   fly_str_test — freestanding test binary (links fly_str + libc for malloc)
#
# fly_str.c uses `extern void *malloc / free` declared directly; libc
# is NOT linked into the library itself — only the test binary needs it.
##===-------------------------------------------------------------------===##

# ── Library ──────────────────────────────────────────────────────────────────

add_library(fly_str STATIC fly_str.c)

target_include_directories(fly_str
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}          # exposes fly_str.h
    PRIVATE ${CMAKE_SOURCE_DIR}/runtime          # platform/Runtime.h
)

target_compile_options(fly_str PRIVATE
    -nostdinc
    -O2
    -Wall
    -Wextra
    -Werror
)

set_target_properties(fly_str PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ── Freestanding test binary ─────────────────────────────────────────────────

add_executable(fly_str_test fly_str_test.c)

target_include_directories(fly_str_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(fly_str_test PRIVATE
    -nostdinc
    -O2
    -Wall
    -Wextra
    -fno-stack-protector
)

# -nostdlib/-nostartfiles: no CRT; _start in fly_str_test.c is the entry point.
# Stack is force-aligned to 16 bytes inside the naked _start before any calls.
# -lc: supplies malloc/free used by fly_str.c transform functions.
target_link_options(fly_str_test PRIVATE -nostdlib -nostartfiles)
target_link_libraries(fly_str_test PRIVATE fly_str c)

add_test(NAME fly_str_test COMMAND fly_str_test)
