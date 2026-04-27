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

# Place the archive in a predictable location so ToolChain can locate it via
# the FLY_RUNTIME_LIB_DIR cmake variable baked into Config.h.
set_target_properties(FlyRuntime PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Runtime.h is the only public header.
target_include_directories(FlyRuntime PUBLIC
    "${CMAKE_SOURCE_DIR}/runtime"
)

# ── Platform dispatch ────────────────────────────────────────────────────────
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(Linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(Windows)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(macOS)
else()
    message(FATAL_ERROR "Runtime: unsupported platform '${CMAKE_SYSTEM_NAME}'")
endif()
