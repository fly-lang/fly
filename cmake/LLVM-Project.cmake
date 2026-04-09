include(FetchContent)

# Configure LLVM precompiled parameters
set(LLVM_RELEASES_PREFIX_URL https://github.com/fly-lang/llvm-project/releases/download)
if (MSVC)
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v20.1.8-win-x64/llvm-20.1.8-win-x64.zip")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.zip")
    set(LLVM_HASH 1fecdf21b5eb42728f9819f72169cef782cfd2c1f6a87cdc3650ba6c2c7e7d08) # TODO: update after build
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v20.1.8-macos-x86_64/llvm-20.1.8-x86_64-apple-darwin.tar.gz")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    set(LLVM_HASH 0000000000000000000000000000000000000000) # TODO: update after build
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v20.1.8-linux-x86_64/llvm-20.1.8-x86_64-linux-gnu.tar.gz")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    set(LLVM_HASH 9447dc6df09a1174f896889776e33711683356e92dbd877d53bb663ad02f1005) # TODO: update after build
else()
    message(FATAL_ERROR "Unknown system, cannot download pre-build llvm packages, run with -DLLVM_BUILD")
endif()

FetchContent_Declare(
        ${FLY_LLVM_PROJECT}
        URL ${LLVM_RELEASES_URL}
        URL_HASH SHA256=${LLVM_HASH}
        SOURCE_DIR llvm
        DOWNLOAD_EXTRACT_TIMESTAMP true)
FetchContent_MakeAvailable(${FLY_LLVM_PROJECT})

message(STATUS "Downloaded LLVM precompiled files")

# zstd is required by the precompiled LLVM (linked as zstd::libzstd_shared)
find_package(zstd QUIET)
if(NOT zstd_FOUND)
    find_library(ZSTD_LIBRARY NAMES zstd)
    if(ZSTD_LIBRARY)
        add_library(zstd::libzstd_shared SHARED IMPORTED GLOBAL)
        set_target_properties(zstd::libzstd_shared PROPERTIES IMPORTED_LOCATION ${ZSTD_LIBRARY})
    else()
        message(FATAL_ERROR "zstd library not found. Install libzstd-dev.")
    endif()
endif()

find_package(LLD REQUIRED CONFIG PATHS ${CMAKE_BINARY_DIR}/llvm NO_DEFAULT_PATH)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
add_definitions(${LLVM_DEFINITIONS})
