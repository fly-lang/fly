include(FetchContent)

# Configure LLVM precompiled parameters
set(LLVM_RELEASES_PREFIX_URL https://github.com/fly-lang/llvm-project/releases/download)

if (MSVC)
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v${FLY_LLVM_VERSION}-win-x64/llvm-${FLY_LLVM_VERSION}-win-x64.zip")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.zip")
    set(LLVM_HASH 5ddc777fcb86d6c4b5d52a6bd36f98f865d1f3c89d75f00fc3a503ac4c8dd680)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v${FLY_LLVM_VERSION}-macos-x86_64/llvm-${FLY_LLVM_VERSION}-x86_64-apple-darwin.tar.gz")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    set(LLVM_HASH 0000000000000000000000000000000000000000)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v${FLY_LLVM_VERSION}-linux-x86_64/llvm-${FLY_LLVM_VERSION}-x86_64-linux-gnu.tar.gz")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    set(LLVM_HASH 0484881e1ee455aac85a0fdf2a29e589597de6f630ea702f18a4a9f23430810c)
else()
    message(FATAL_ERROR "Unknown system, cannot download pre-build llvm packages, run with -DLLVM_BUILD")
endif()

# Download and extract the precompiled LLVM package
message(STATUS "Downloading LLVM from ${LLVM_RELEASES_URL}")
FetchContent_Declare(
        ${FLY_LLVM_PROJECT}
        URL ${LLVM_RELEASES_URL}
        URL_HASH SHA256=${LLVM_HASH}
        SOURCE_DIR llvm
        DOWNLOAD_EXTRACT_TIMESTAMP true)
FetchContent_MakeAvailable(${FLY_LLVM_PROJECT})

message(STATUS "Downloaded LLVM precompiled files")

# The precompiled LLVM package ships Findzstd.cmake in its cmake dir.
# Add it to CMAKE_MODULE_PATH so find_package(zstd) uses it.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/llvm/lib/cmake/llvm")

# zstd is required by the precompiled LLVM (linked as zstd::libzstd_shared).
# Search order:
#   1. System cmake config  (finds libzstd-dev on Linux, vcpkg on Windows)
#   2. find_library fallback with common library names (static variants included)
#   3. FetchContent: download and build zstd statically from source
find_package(zstd QUIET)
if(NOT zstd_FOUND)
    find_library(ZSTD_LIBRARY
        NAMES zstd libzstd zstd_static libzstd_static
        PATHS "${CMAKE_BINARY_DIR}/llvm/lib"
    )
    if(ZSTD_LIBRARY)
        add_library(zstd::libzstd_shared UNKNOWN IMPORTED GLOBAL)
        set_target_properties(zstd::libzstd_shared PROPERTIES
            IMPORTED_LOCATION "${ZSTD_LIBRARY}")
    else()
        message(STATUS "zstd not found — downloading and building from source")
        FetchContent_Declare(
            zstd_fc
            URL      https://github.com/facebook/zstd/releases/download/v1.5.5/zstd-1.5.5.tar.gz
            URL_HASH SHA256=9c4396cc829cfae319a6e2615202e82aad41372073482fce286fac78646d3ee4
            SOURCE_SUBDIR  build/cmake
            DOWNLOAD_EXTRACT_TIMESTAMP true
        )
        set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_STATIC   ON  CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(zstd_fc)
        # Expose the static target under the name the LLVM cmake config expects
        add_library(zstd::libzstd_shared ALIAS libzstd_static)
    endif()
endif()

find_package(LLD REQUIRED CONFIG PATHS ${CMAKE_BINARY_DIR}/llvm NO_DEFAULT_PATH)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
add_definitions(${LLVM_DEFINITIONS})
