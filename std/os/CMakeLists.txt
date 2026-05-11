##===-- std/os/CMakeLists.txt - fly.os build targets -------------------===##
#
# Targets
#   fly_os              — static library (5 submodules: io, fs, path, env, time)
#   fly_os_path_test    — freestanding test binary
#   fly_os_env_test     — freestanding test binary
#   fly_os_time_test    — freestanding test binary
#   fly_os_fs_test      — freestanding test binary
#   fly_os_io_test      — freestanding test binary
#   fly_os_inspect      — emits fly_os_*.ll and greps for libc symbols
#
# All library code: -nostdinc -O2.
# Test binaries: -nostdlib -nostartfiles, linked with -lc (malloc/free/setenv).
##===-------------------------------------------------------------------===##

# ── Library ──────────────────────────────────────────────────────────────────

add_library(fly_os STATIC
    fly_os_io.c
    fly_os_path.c
    fly_os_env.c
    fly_os_time.c
    fly_os_fs.c
)

target_include_directories(fly_os
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}          # fly_os_*.h
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}          # fly_os_linux.h
)

# fly_os public API exposes fly_string/fly_string_array — propagate fly_str headers
target_link_libraries(fly_os PUBLIC fly_str)

target_compile_options(fly_os PRIVATE
    -nostdinc
    -O2
    -Wall
    -Wextra
    -Werror
)

set_target_properties(fly_os PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ── Common compile / link options for test binaries ───────────────────────────

set(OS_TEST_COMPILE_OPTS -nostdinc -O2 -Wall -Wextra)
set(OS_TEST_LINK_OPTS    -nostdlib -nostartfiles)

# ── Helper macro to define one test target ────────────────────────────────────

macro(add_fly_os_test name src)
    add_executable(${name} ${src})
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${name} PRIVATE ${OS_TEST_COMPILE_OPTS})
    target_link_options(${name} PRIVATE ${OS_TEST_LINK_OPTS})
    target_link_libraries(${name} PRIVATE fly_os c)
    add_test(NAME ${name} COMMAND ${name})
endmacro()

add_fly_os_test(fly_os_path_test fly_os_path_test.c)
add_fly_os_test(fly_os_env_test  fly_os_env_test.c)
add_fly_os_test(fly_os_time_test fly_os_time_test.c)
add_fly_os_test(fly_os_fs_test   fly_os_fs_test.c)
add_fly_os_test(fly_os_io_test   fly_os_io_test.c)

# ── IR inspection: grep output for unexpected libc symbols ────────────────────
#
# Usage: cmake --build . --target fly_os_inspect
# A hit on malloc/free/setenv is expected (declared as extern).
# A hit on open/read/write/stat would indicate libc bleeding — move to .ll.

add_custom_target(fly_os_inspect
    COMMAND ${CMAKE_COMMAND} -E echo "=== fly_os_io ==="
    COMMAND ${CMAKE_C_COMPILER} -S -emit-llvm -O2 -nostdinc
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_SOURCE_DIR}/std/str
            ${CMAKE_CURRENT_SOURCE_DIR}/fly_os_io.c
            -o ${CMAKE_CURRENT_BINARY_DIR}/fly_os_io_inspect.ll
    COMMAND grep -o "call.*@[a-zA-Z_][a-zA-Z_0-9]*" ${CMAKE_CURRENT_BINARY_DIR}/fly_os_io_inspect.ll
            | grep -v "llvm\\." | sort -u || true
    COMMAND ${CMAKE_COMMAND} -E echo "=== fly_os_fs ==="
    COMMAND ${CMAKE_C_COMPILER} -S -emit-llvm -O2 -nostdinc
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_SOURCE_DIR}/std/str
            ${CMAKE_CURRENT_SOURCE_DIR}/fly_os_fs.c
            -o ${CMAKE_CURRENT_BINARY_DIR}/fly_os_fs_inspect.ll
    COMMAND grep -o "call.*@[a-zA-Z_][a-zA-Z_0-9]*" ${CMAKE_CURRENT_BINARY_DIR}/fly_os_fs_inspect.ll
            | grep -v "llvm\\." | sort -u || true
    COMMENT "Inspecting fly_os modules for external symbols"
    VERBATIM
)
