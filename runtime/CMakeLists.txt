##===-- runtime/CMakeLists.txt - Fly minimal runtime library ----------===##
#
# Builds 'Runtime' as a static library with no libc dependency.
# Platform subdirectories add sources and compile flags.
#
#   src/Runtime/Linux/  — Linux x86-64          (syscall ABI)
#   src/Runtime/Windows/— Windows x64           (Win32/NT API)
#   src/Runtime/macOS/  — macOS AArch64/x86-64  (libSystem / __ulock)
#

add_library(FlyRuntime STATIC)

# Place the archive alongside fly_std_lib.a and the .fly.h headers so the
# entire lib/ directory is the single install unit.
set_target_properties(FlyRuntime PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    OUTPUT_NAME              "fly_runtime_lib"
    PREFIX                   ""
)

install(TARGETS FlyRuntime ARCHIVE DESTINATION lib)

# ── Fly bridge header stubs (llvm, runtime) — copied to build/lib/ ────────────
# llvm.fly.h   — LLVM intrinsics declarations (fly.llvm namespace)
# runtime.fly.h— C runtime declarations (fly.runtime namespace)
# Both are declaration-only stubs used by every stdlib module.
set(_fly_bridge_headers
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/llvm.fly.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/runtime.fly.h"
)
set(_fly_bridge_dst)
foreach(_h ${_fly_bridge_headers})
    get_filename_component(_name "${_h}" NAME)
    set(_dst "${CMAKE_BINARY_DIR}/lib/${_name}")
    add_custom_command(
        OUTPUT  "${_dst}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_h}" "${_dst}"
        DEPENDS "${_h}"
        COMMENT "Copying ${_name} to build/lib/"
        VERBATIM
    )
    list(APPEND _fly_bridge_dst "${_dst}")
endforeach()
add_custom_target(fly_runtime_lib_headers ALL DEPENDS ${_fly_bridge_dst})
# Export DST paths to parent scope so std/CMakeLists.txt can add them to DEPENDS.
set(FLY_RUNTIME_LIB_HEADERS_DST ${_fly_bridge_dst} PARENT_SCOPE)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/"
        DESTINATION lib FILES_MATCHING PATTERN "*.fly.h")

# Runtime.h is the only public header.
target_include_directories(FlyRuntime PUBLIC
    "${CMAKE_SOURCE_DIR}/runtime"
)

# ── Platform dispatch ────────────────────────────────────────────────────────
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(platform/Linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(platform/Windows)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(platform/macOS)
else()
    message(FATAL_ERROR "Runtime: unsupported platform '${CMAKE_SYSTEM_NAME}'")
endif()

######################################
# Runtime test (freestanding C)
######################################

add_executable(fly_runtime_test test/RuntimeTest.c)

target_include_directories(fly_runtime_test PRIVATE ${CMAKE_SOURCE_DIR}/runtime)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(fly_runtime_test PRIVATE
            -ffreestanding -nostdinc -fno-stack-protector -fno-builtin)
    # mem_alloc is backed by malloc; link libc but supply our own _start entry point.
    target_link_options(fly_runtime_test PRIVATE -nostartfiles -Wl,-e,_start)
    target_link_libraries(fly_runtime_test PRIVATE c m)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_options(fly_runtime_test PRIVATE
            -ffreestanding -fno-stack-protector -fno-builtin)
    target_link_options(fly_runtime_test PRIVATE -lSystem)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(MSVC)
        target_compile_options(fly_runtime_test PRIVATE /GS-)
    else()
        target_compile_options(fly_runtime_test PRIVATE -fno-stack-protector -fno-builtin)
    endif()
    target_link_libraries(fly_runtime_test PRIVATE synchronization kernel32)
endif()

target_link_libraries(fly_runtime_test PRIVATE FlyRuntime)

add_test(NAME fly_runtime_test COMMAND fly_runtime_test)
set_tests_properties(fly_runtime_test PROPERTIES PASS_REGULAR_EXPRESSION "fly runtime ok")

######################################
# LLDB debug integration test
######################################

find_program(LLVM_DWARFDUMP        NAMES llvm-dwarfdump llvm-dwarfdump-20 llvm-dwarfdump-19 llvm-dwarfdump-18)

if(MSVC)
    set(_fly_std_lib "${CMAKE_BINARY_DIR}/std/fly_std_lib.lib")
else()
    set(_fly_std_lib "${CMAKE_BINARY_DIR}/std/fly_std_lib.a")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_test(NAME debug_dwarf_test
        COMMAND powershell -ExecutionPolicy Bypass -File
            "${CMAKE_CURRENT_SOURCE_DIR}/test/debug_dwarf_test.ps1"
            "$<TARGET_FILE:fly>"
            "${_fly_std_lib}"
            "${LLVM_DWARFDUMP}"
    )
else()
    add_test(NAME debug_dwarf_test
        COMMAND bash
            "${CMAKE_CURRENT_SOURCE_DIR}/test/debug_dwarf_test.sh"
            "$<TARGET_FILE:fly>"
            "${_fly_std_lib}"
            "${LLVM_DWARFDUMP}"
    )
endif()

set_tests_properties(debug_dwarf_test PROPERTIES
    SKIP_REGULAR_EXPRESSION "^SKIP:"
)

