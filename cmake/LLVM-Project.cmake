include(FetchContent)

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
