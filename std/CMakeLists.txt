##===-- std/CMakeLists.txt - Fly standard library --------------------------===##
#
# The Fly standard library is 100% implemented in Fly source:
#
#   fly_std_lib.a/.lib — Fly-compiled namespace objects.
#                        Built by the fly binary after it is available.
#                        Extension: .a on Linux/macOS, .lib on Windows-MSVC.
#                        The compiler resolves stdlib imports by parsing .fly.h
#                        headers from build/lib/ (copied at build time).
#
# Users link: fly_std_lib.a (or fly_std_lib.lib on Windows) plus the runtime library.
#
# Build outputs in ${CMAKE_BINARY_DIR}/lib/:
#   fly_std_lib.a         — compiled stdlib archive
#   *.fly.h               — declaration headers (copied from std/lib/ at build time)
#
# Namespaces → Source files
#   fly.assert       lib/assert.fly
#   fly.str          lib/str.fly
#   fly.math         lib/math.fly
#   fly.os.time      lib/os/time.fly
#   fly.os.env       lib/os/env.fly
#   fly.os.path      lib/os/path.fly
#   fly.os.io        lib/os/io.fly
#   fly.os.fs        lib/os/fs.fly
#   fly.os.proc      lib/os/proc.fly
#   fly.sync         lib/sync.fly
#   fly.mem          lib/mem.fly
#   fly.data         lib/data/list.fly
#   fly.data         lib/data/stack.fly
#   fly.data         lib/data/queue.fly
#   fly.data         lib/data/deque.fly
#   fly.data         lib/data/map.fly
#   fly.data         lib/data/set.fly
#   fly.data         lib/data/tree.fly
#
# Tests live in test/:
#   test/str_test.fly        — Fly-native tests for fly.str
#   test/math_test.fly       — Fly-native tests for fly.math
#   test/os_env_test.fly     — Fly-native tests for fly.os.env
#   test/os_path_test.fly    — Fly-native tests for fly.os.path
#   test/os_fs_test.fly      — Fly-native tests for fly.os.fs
#   test/os_time_test.fly    — Fly-native tests for fly.os.time
#   test/os_io_test.fly      — Fly-native tests for fly.os.io
#   test/mem_test.fly        — Fly-native tests for fly.mem
#   test/data_test.fly       — Fly-native tests for fly.data.*
#

# ── FlyLib: empty INTERFACE target (kept for compiler/CMakeLists.txt compat) ──

add_library(FlyLib INTERFACE)

# ── Derive the .fly.h output paths produced by fly --lib ─────────────────────
#
# fly --lib writes one flat <stem>.fly.h per input source into the output
# directory (build/lib/).  We declare them here so CMake can track them as
# outputs of the fly_std_lib command and rebuild when they are missing.

set(FLY_H_HEADERS_DST)
foreach(_SRC ${FLY_LIB_SOURCES})
    get_filename_component(_STEM "${_SRC}" NAME_WE)
    list(APPEND FLY_H_HEADERS_DST "${CMAKE_BINARY_DIR}/lib/${_STEM}.fly.h")
endforeach()

# ── Fly namespace archive (built by the fly binary) ──────────────────────────

set(FLY_LIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/assert.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/str.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/math.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/os/time.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/os/env.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/os/path.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/os/io.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/os/fs.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/sync.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mem.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/bridge/clang.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/data/list.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/data/stack.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/data/queue.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/data/deque.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/data/map.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/data/set.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/data/tree.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/data/wrapper.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/os/proc.fly
)

# fly --lib appends .a (Linux/macOS) or .lib (Windows-MSVC); pass the stem via -o.
set(FLY_LIB_STEM "${CMAKE_BINARY_DIR}/lib/fly_std_lib")
if(MSVC)
    set(FLY_LIB_OUTPUT "${FLY_LIB_STEM}.lib")
else()
    set(FLY_LIB_OUTPUT "${FLY_LIB_STEM}.a")
endif()

# fly_std_lib depends on FLY_H_HEADERS_DST so headers are in build/lib/ before
# fly tries to load them during stdlib compilation.
add_custom_command(
    OUTPUT  ${FLY_LIB_OUTPUT} ${FLY_H_HEADERS_DST}
    COMMAND $<TARGET_FILE:fly>
            --lib -o ${FLY_LIB_STEM}
            ${FLY_LIB_SOURCES}
    DEPENDS fly ${FLY_LIB_SOURCES} fly_runtime_lib_headers
    COMMENT "Compiling Fly standard library namespaces"
    VERBATIM
)

add_custom_target(fly_std_lib ALL DEPENDS ${FLY_LIB_OUTPUT})

install(FILES "${FLY_LIB_OUTPUT}" DESTINATION lib)

# ── Fly integration tests ─────────────────────────────────────────────────────
#
# Native Fly tests for each stdlib namespace.
# Each binary exits 0 on success, non-zero with the failing assertion code.

macro(add_fly_std_test name)
    add_custom_command(
        OUTPUT  ${CMAKE_BINARY_DIR}/${name}_test_fly
        COMMAND $<TARGET_FILE:fly>
                ${CMAKE_CURRENT_SOURCE_DIR}/test/${name}_test.fly
                ${FLY_LIB_OUTPUT}
                -o ${CMAKE_BINARY_DIR}/${name}_test_fly
        DEPENDS fly fly_std_lib FlyRuntime
                ${CMAKE_CURRENT_SOURCE_DIR}/test/${name}_test.fly
        COMMENT "Compiling ${name}_test.fly"
        VERBATIM
    )
    add_custom_target(${name}_test_fly_bin ALL
        DEPENDS ${CMAKE_BINARY_DIR}/${name}_test_fly)
    add_test(NAME fly_${name}_fly_test
        COMMAND ${CMAKE_BINARY_DIR}/${name}_test_fly)
endmacro()

foreach(_test str math os_env os_path os_fs os_time os_io mem data generics enum cast override super interface deepinherit supermember)
    add_fly_std_test(${_test})
endforeach()
