include(FetchContent)

# Configure LLVM precompiled parameters
set(LLVM_RELEASES_PREFIX_URL https://github.com/fly-lang/llvm-project/releases/download)
if (MSVC)
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v11.1.0-win-x64/llvm-11.1.0-win-x64.zip")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.zip")
    set(LLVM_HASH 8ad19b396b808bbb30000cb765ff9fcfd0da980b)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v11.1.0-macos-x86_64/llvm-11.1.0-x86_64-apple-darwin.tar.gz")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    set(LLVM_HASH 37434bded4be01650c2fce61b565bd37902cf079)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v11.1.0-linux-x86_64/llvm-11.1.0-x86_64-linux-gnu.tar.gz")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    set(LLVM_HASH 87277380a3636cdc53ca0f5d8ec664322f54902d)
else()
    message(FATAL_ERROR "Unknown system, cannot download pre-build llvm packages, run with -DLLVM_BUILD")
endif()

FetchContent_Declare(
        ${FLY_LLVM_PROJECT}
        URL ${LLVM_RELEASES_URL}
        URL_HASH SHA1=${LLVM_HASH}
        SOURCE_DIR llvm)
FetchContent_MakeAvailable(${FLY_LLVM_PROJECT})

message(STATUS "Downloaded LLVM precompiled files")
find_package(LLD REQUIRED CONFIG PATHS ${CMAKE_BINARY_DIR}/llvm NO_DEFAULT_PATH)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
add_definitions(${LLVM_DEFINITIONS})
