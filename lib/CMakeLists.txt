##===-- src/Lib/CMakeLists.txt - Fly standard library ---------------------===##
#
# The Fly standard library consists of two complementary artifacts:
#
#   libFlyLib.a   — C-compiled object files (functions that require
#                      operations not yet expressible in pure Fly, e.g.
#                      array subscript, unary minus, heap allocation).
#                      Built by the host C compiler via a normal CMake target.
#
#   fly_lib.lib   — Fly-compiled namespace objects + .fly.h header metadata.
#                      Built by the fly binary after it is available.
#
# Users link both archives: -lflyLib fly_lib.lib
#
# Namespace  → Source file      → Header
#   fly.string  Fly/String.fly  include/Lib/Fly/String.fly.h
#   fly.math    Fly/Math.fly    include/Lib/Fly/Math.fly.h
#   fly.os      Fly/Os.fly      include/Lib/Fly/Os.fly.h
#   fly.thread  Fly/Thread.fly  include/Lib/Fly/Thread.fly.h
#

# ── C implementations (linked into a regular static library) ─────────────────

add_library(FlyLib STATIC
        String.c
)

target_include_directories(FlyLib PUBLIC
    ${CMAKE_SOURCE_DIR}/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}        # exposes Fly/String.h to consumers
)

target_link_libraries(FlyLib PUBLIC FlyRuntime)

set_target_properties(FlyLib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ── Fly namespace archive (built by the fly binary) ──────────────────────────

set(FLY_LIB_SOURCES
        Fly/string.fly
        Fly/math.fly
        Fly/os.fly
        Fly/thread.fly
)

# The fly compiler appends .lib automatically; pass the stem without extension.
set(FLY_LIB_STEM   "${CMAKE_BINARY_DIR}/lib/fly_lib")
set(FLY_LIB_OUTPUT "${FLY_LIB_STEM}.lib")

add_custom_command(
    OUTPUT  ${FLY_LIB_OUTPUT}
    COMMAND $<TARGET_FILE:fly>
            -lib ${FLY_LIB_STEM}
            ${FLY_LIB_SOURCES}
    DEPENDS fly ${FLY_LIB_SOURCES}
    COMMENT "Compiling Fly standard library namespaces"
    VERBATIM
)

add_custom_target(fly_lib ALL DEPENDS ${FLY_LIB_OUTPUT})
