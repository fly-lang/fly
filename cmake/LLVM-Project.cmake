include(FetchContent)

FetchContent_Declare(
        ${FLY_LLVM_PROJECT}
        GIT_REPOSITORY https://github.com/fly-lang/llvm-project.git
        GIT_TAG fly-llvm-${FLY_LLVM_VERSION})
FetchContent_MakeAvailable(${FLY_LLVM_PROJECT})

message(STATUS "Building LLVM from sources")
set(LLVM_DIR ${FLY_LLVM_BINARY_DIR}/llvm/lib/cmake/llvm)
set(LLD_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/${FLY_LLVM_PROJECT}/lld/include)
set(LLVM_ENABLE_PROJECTS lld)

add_subdirectory(${CMAKE_SOURCE_DIR}/${FLY_LLVM_PROJECT}/llvm)
find_package(LLVM ${FLY_LLVM_VERSION} REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})

message(STATUS "Include LLD directory: ${LLD_INCLUDE_DIRS}")
include_directories(${LLD_INCLUDE_DIRS})
