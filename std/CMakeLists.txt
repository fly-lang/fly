##===-- std/CMakeLists.txt - Fly standard library --------------------------===##
#
# The Fly standard library consists of two complementary artifacts:
#
#   libFlyLib.a   — C-compiled object files (functions that require
#                      operations not yet expressible in pure Fly, e.g.
#                      array subscript, unary minus, heap allocation).
#                      Built by the host C compiler via a normal CMake target.
#
#   fly_lib.a/.lib — Fly-compiled namespace objects + .fly.h header metadata.
#                      Built by the fly binary after it is available.
#                      Extension: .a on Linux/macOS, .lib on Windows-MSVC.
#
# Users link both archives: -lFlyLib fly_lib.a  (or fly_lib.lib on Windows)
#
# Namespace  → Source file      → Header
#   fly.str     src/str.fly     src/str.fly.h
#   fly.math    src/math.fly    src/math.fly.h
#   fly.os      src/os.fly      src/os.fly.h
#   fly.sync    src/sync.fly    src/sync.fly.h
#
# Module subdirectories (each owns its own static lib + freestanding tests)
#   str/    → fly_str   (fly_str.c / fly_str_test.c)
#   math/   → fly_math  (fly_math.c / fly_math_test.c)
#   os/     → fly_os    (fly_os_*.c / fly_os_*_test.c per submodule)
#

# ── Module subdirectories ─────────────────────────────────────────────────────

add_subdirectory(str)
add_subdirectory(math)
add_subdirectory(os)

# ── FlyLib: aggregate INTERFACE target linked by the compiler binary ─────────
#
# compiler/CMakeLists.txt links `fly` against FlyLib only.
# Every new C stdlib module (fly_str, fly_math, …) must be listed here so the
# compiler binary sees its symbols at link time.

add_library(FlyLib INTERFACE)
target_link_libraries(FlyLib INTERFACE
    fly_str
    fly_math
    fly_os
)

# ── Fly namespace archive (built by the fly binary) ──────────────────────────

set(FLY_LIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/str.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/math.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/os.fly
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sync.fly
)

# fly --lib appends .a (Linux/macOS) or .lib (Windows-MSVC); pass the stem via -o.
set(FLY_LIB_STEM "${CMAKE_BINARY_DIR}/lib/fly_lib")
if(MSVC)
    set(FLY_LIB_OUTPUT "${FLY_LIB_STEM}.lib")
else()
    set(FLY_LIB_OUTPUT "${FLY_LIB_STEM}.a")
endif()

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
