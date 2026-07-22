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
# Tests live in test/, ONE DIRECTORY PER PROGRAM (fly compiles a source
# directory: the entry — main() or suite — is discovered from --src-dir):
#   test/str/str_test.fly    — Fly-native tests for fly.str
#   test/math/math_test.fly  — Fly-native tests for fly.math
#   ... (one subdir per namespace test)
#   test/demo/DemoSuite.fly  — suite-runner e2e (all pass)
#   test/fail/FailSuite.fly  — suite-runner e2e (one failure)
#   test/error/error_test.fly — unhandled-error contract
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
# fly tries to load them during stdlib compilation. The whole std/lib directory
# is the library: fly discovers every source under --src-dir.
add_custom_command(
    OUTPUT  ${FLY_LIB_OUTPUT} ${FLY_H_HEADERS_DST}
    COMMAND $<TARGET_FILE:fly>
            --lib -o ${FLY_LIB_STEM}
            --src-dir ${CMAKE_CURRENT_SOURCE_DIR}/lib
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

# The std archive is not passed explicitly: fly auto-links fly_std_lib and the
# runtime from <bin>/../lib (their .fly.h headers resolve the imports too).
macro(add_fly_std_test name)
    add_custom_command(
        OUTPUT  ${CMAKE_BINARY_DIR}/${name}_test_fly
        COMMAND $<TARGET_FILE:fly>
                --src-dir ${CMAKE_CURRENT_SOURCE_DIR}/test/${name}
                -o ${CMAKE_BINARY_DIR}/${name}_test_fly
        DEPENDS fly fly_std_lib FlyRuntime
                ${CMAKE_CURRENT_SOURCE_DIR}/test/${name}/${name}_test.fly
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

# ── Unhandled-error contract test ─────────────────────────────────────────────
# error_test.fly ends with an unhandled `fail 42, <computed message>`: main
# must print "error 42: boom got 5" to stderr and exit 42, so it cannot use
# add_fly_std_test's exit-0 add_test. Two ctest entries on the same binary:
# one matches the printed error line, the other requires the non-zero exit.
add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/error_test_fly
    COMMAND $<TARGET_FILE:fly>
            --src-dir ${CMAKE_CURRENT_SOURCE_DIR}/test/error
            -o ${CMAKE_BINARY_DIR}/error_test_fly
    DEPENDS fly fly_std_lib FlyRuntime
            ${CMAKE_CURRENT_SOURCE_DIR}/test/error/error_test.fly
    COMMENT "Compiling error_test.fly"
    VERBATIM
)
add_custom_target(error_test_fly_bin ALL
    DEPENDS ${CMAKE_BINARY_DIR}/error_test_fly)
add_test(NAME fly_error_fly_test
    COMMAND ${CMAKE_BINARY_DIR}/error_test_fly)
set_tests_properties(fly_error_fly_test PROPERTIES
    PASS_REGULAR_EXPRESSION "error 42: boom got 5")
add_test(NAME fly_error_exit_fly_test
    COMMAND ${CMAKE_BINARY_DIR}/error_test_fly)
set_tests_properties(fly_error_exit_fly_test PROPERTIES WILL_FAIL TRUE)

# ── Suite runner end-to-end tests ─────────────────────────────────────────────
# DemoSuite: all cases pass → exit 0, summary "3 cases, 3 passed, 0 failed".
# FailSuite: one case fails → the FAIL line carries code+message, the later
# case still runs, the summary counts the failure, and the binary exits 1
# (regex-based ctest entries ignore the exit code by design).
foreach(_pair "Demo;demo" "Fail;fail")
    list(GET _pair 0 _suite)
    list(GET _pair 1 _dir)
    add_custom_command(
        OUTPUT  ${CMAKE_BINARY_DIR}/${_suite}Suite_fly
        COMMAND $<TARGET_FILE:fly>
                --test
                --src-dir ${CMAKE_CURRENT_SOURCE_DIR}/test/${_dir}
                -o ${CMAKE_BINARY_DIR}/${_suite}Suite_fly
        DEPENDS fly fly_std_lib FlyRuntime
                ${CMAKE_CURRENT_SOURCE_DIR}/test/${_dir}/${_suite}Suite.fly
        COMMENT "Compiling ${_suite}Suite.fly"
        VERBATIM
    )
    add_custom_target(${_suite}Suite_fly_bin ALL
        DEPENDS ${CMAKE_BINARY_DIR}/${_suite}Suite_fly)
endforeach()

add_test(NAME fly_demo_suite_test COMMAND ${CMAKE_BINARY_DIR}/DemoSuite_fly)
set_tests_properties(fly_demo_suite_test PROPERTIES
    PASS_REGULAR_EXPRESSION "suite DemoSuite: 3 cases, 3 passed, 0 failed")

add_test(NAME fly_fail_suite_test COMMAND ${CMAKE_BINARY_DIR}/FailSuite_fly)
set_tests_properties(fly_fail_suite_test PROPERTIES
    PASS_REGULAR_EXPRESSION "boom \\.\\.\\. FAIL\\(42\\): assertEqI: got 1, expected 2")

add_test(NAME fly_fail_suite_summary_test COMMAND ${CMAKE_BINARY_DIR}/FailSuite_fly)
set_tests_properties(fly_fail_suite_summary_test PROPERTIES
    PASS_REGULAR_EXPRESSION "suite FailSuite: 2 cases, 1 passed, 1 failed")
