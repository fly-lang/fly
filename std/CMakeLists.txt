##===-- std/CMakeLists.txt - Fly standard library --------------------------===##
#
# The Fly standard library is 100% implemented in Fly source:
#
#   fly_lib.a/.lib — Fly-compiled namespace objects.
#                  x    Built by the fly binary after it is available.
#                      Extension: .a on Linux/macOS, .lib on Windows-MSVC.
#                      The compiler resolves stdlib imports by parsing .fly
#                      sources directly (no .fly.h headers needed).
#
# Users link: fly_lib.a  (or fly_lib.lib on Windows) plus the runtime library.
#
# Namespaces → Source files
#   fly.assert       src/assert.fly
#   fly.str          src/str.fly
#   fly.math         src/math.fly
#   fly.os.time      src/os/time.fly
#   fly.os.env       src/os/env.fly
#   fly.os.path      src/os/path.fly
#   fly.os.io        src/os/io.fly
#   fly.os.fs        src/os/fs.fly
#   fly.sync         src/sync.fly
#
# Tests live in test/:
#   test/str_test.fly        — Fly-native tests for fly.str
#   test/math_test.fly       — Fly-native tests for fly.math
#   test/os_env_test.fly     — Fly-native tests for fly.os.env
#   test/os_path_test.fly    — Fly-native tests for fly.os.path
#   test/os_fs_test.fly      — Fly-native tests for fly.os.fs
#   test/os_time_test.fly    — Fly-native tests for fly.os.time
#   test/os_io_test.fly      — Fly-native tests for fly.os.io
#

# ── FlyLib: empty INTERFACE target (kept for compiler/CMakeLists.txt compat) ──

add_library(FlyLib INTERFACE)

# ── Fly namespace archive (built by the fly binary) ──────────────────────────

set(FLY_LIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/assert.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/str.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/math.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/os/time.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/os/env.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/os/path.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/os/io.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/os/fs.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sync.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bridge/clang.fly
)

# fly --lib appends .a (Linux/macOS) or .lib (Windows-MSVC); pass the stem via -o.
set(FLY_LIB_STEM "${CMAKE_BINARY_DIR}/lib/fly_lib")
if(MSVC)
    set(FLY_LIB_OUTPUT "${FLY_LIB_STEM}.lib")
else()
    set(FLY_LIB_OUTPUT "${FLY_LIB_STEM}.a")
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_custom_command(
    OUTPUT  ${FLY_LIB_OUTPUT}
    COMMAND $<TARGET_FILE:fly>
            --lib -o ${FLY_LIB_STEM}
            ${FLY_LIB_SOURCES}
    DEPENDS fly ${FLY_LIB_SOURCES}
    COMMENT "Compiling Fly standard library namespaces"
    VERBATIM
)

add_custom_target(fly_lib ALL DEPENDS ${FLY_LIB_OUTPUT})

# ── Fly integration tests ─────────────────────────────────────────────────────
#
# Native Fly tests for fly.str, fly.math, and fly.os.
# Each binary exits 0 on success, non-zero with the failing assertion code.

add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/str_test_fly
    COMMAND $<TARGET_FILE:fly>
            ${CMAKE_CURRENT_SOURCE_DIR}/test/str_test.fly
            ${FLY_LIB_OUTPUT}
            -o ${CMAKE_BINARY_DIR}/str_test_fly
    DEPENDS fly fly_lib ${CMAKE_CURRENT_SOURCE_DIR}/test/str_test.fly
    COMMENT "Compiling str_test.fly"
    VERBATIM
)
add_custom_target(str_test_fly_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/str_test_fly)
add_test(NAME fly_str_fly_test COMMAND ${CMAKE_BINARY_DIR}/str_test_fly)

add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/math_test_fly
    COMMAND $<TARGET_FILE:fly>
            ${CMAKE_CURRENT_SOURCE_DIR}/test/math_test.fly
            ${FLY_LIB_OUTPUT}
            -o ${CMAKE_BINARY_DIR}/math_test_fly
    DEPENDS fly fly_lib ${CMAKE_CURRENT_SOURCE_DIR}/test/math_test.fly
    COMMENT "Compiling math_test.fly"
    VERBATIM
)
add_custom_target(math_test_fly_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/math_test_fly)
add_test(NAME fly_math_fly_test COMMAND ${CMAKE_BINARY_DIR}/math_test_fly)

add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/os_env_test_fly
    COMMAND $<TARGET_FILE:fly>
            ${CMAKE_CURRENT_SOURCE_DIR}/test/os_env_test.fly
            ${FLY_LIB_OUTPUT}
            -o ${CMAKE_BINARY_DIR}/os_env_test_fly
    DEPENDS fly fly_lib ${CMAKE_CURRENT_SOURCE_DIR}/test/os_env_test.fly
    COMMENT "Compiling os_env_test.fly"
    VERBATIM
)
add_custom_target(os_env_test_fly_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/os_env_test_fly)
add_test(NAME fly_os_env_fly_test COMMAND ${CMAKE_BINARY_DIR}/os_env_test_fly)

add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/os_path_test_fly
    COMMAND $<TARGET_FILE:fly>
            ${CMAKE_CURRENT_SOURCE_DIR}/test/os_path_test.fly
            ${FLY_LIB_OUTPUT}
            -o ${CMAKE_BINARY_DIR}/os_path_test_fly
    DEPENDS fly fly_lib ${CMAKE_CURRENT_SOURCE_DIR}/test/os_path_test.fly
    COMMENT "Compiling os_path_test.fly"
    VERBATIM
)
add_custom_target(os_path_test_fly_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/os_path_test_fly)
add_test(NAME fly_os_path_fly_test COMMAND ${CMAKE_BINARY_DIR}/os_path_test_fly)

add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/os_fs_test_fly
    COMMAND $<TARGET_FILE:fly>
            ${CMAKE_CURRENT_SOURCE_DIR}/test/os_fs_test.fly
            ${FLY_LIB_OUTPUT}
            -o ${CMAKE_BINARY_DIR}/os_fs_test_fly
    DEPENDS fly fly_lib ${CMAKE_CURRENT_SOURCE_DIR}/test/os_fs_test.fly
    COMMENT "Compiling os_fs_test.fly"
    VERBATIM
)
add_custom_target(os_fs_test_fly_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/os_fs_test_fly)
add_test(NAME fly_os_fs_fly_test COMMAND ${CMAKE_BINARY_DIR}/os_fs_test_fly)

add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/os_time_test_fly
    COMMAND $<TARGET_FILE:fly>
            ${CMAKE_CURRENT_SOURCE_DIR}/test/os_time_test.fly
            ${FLY_LIB_OUTPUT}
            -o ${CMAKE_BINARY_DIR}/os_time_test_fly
    DEPENDS fly fly_lib ${CMAKE_CURRENT_SOURCE_DIR}/test/os_time_test.fly
    COMMENT "Compiling os_time_test.fly"
    VERBATIM
)
add_custom_target(os_time_test_fly_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/os_time_test_fly)
add_test(NAME fly_os_time_fly_test COMMAND ${CMAKE_BINARY_DIR}/os_time_test_fly)

add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/os_io_test_fly
    COMMAND $<TARGET_FILE:fly>
            ${CMAKE_CURRENT_SOURCE_DIR}/test/os_io_test.fly
            ${FLY_LIB_OUTPUT}
            -o ${CMAKE_BINARY_DIR}/os_io_test_fly
    DEPENDS fly fly_lib ${CMAKE_CURRENT_SOURCE_DIR}/test/os_io_test.fly
    COMMENT "Compiling os_io_test.fly"
    VERBATIM
)
add_custom_target(os_io_test_fly_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/os_io_test_fly)
add_test(NAME fly_os_io_fly_test COMMAND ${CMAKE_BINARY_DIR}/os_io_test_fly)
