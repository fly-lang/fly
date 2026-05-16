cmake_minimum_required(VERSION 3.24.0)

# Set the project name and version
set(FLY_VERSION "0.12.2")
project(Fly VERSION ${FLY_VERSION} LANGUAGES C CXX ASM)

#Fly config options
set(FLY_LLVM_VERSION 20.1.8)
set(FLY_BUG_REPORT_URL "https://github.com/fly-lang/fly/issues")
set(FLY_SOURCE_DIR ${CMAKE_SOURCE_DIR})
message(STATUS "Fly Project directory: ${CMAKE_SOURCE_DIR}")

# specify the C++ standard and flags
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS NO)
if (NOT MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "Build Type: ${BUILD_TYPE}")

if(MSVC)
    set(CMAKE_C_FLAGS_DEBUG "/W4 /WX")
    set(CMAKE_CXX_FLAGS_DEBUG "/W4 /WX")
    set(CMAKE_C_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

message(STATUS "C Flags: ${CMAKE_C_FLAGS_${BUILD_TYPE}}")
message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")

# Enable LLVM Debug messages
if (BUILD_TYPE STREQUAL DEBUG)
    set(LLVM_ENABLE_ASSERTIONS "On")
    message(STATUS "LLVM_ENABLE_ASSERTIONS: On")
endif()

######################################
# Add LLVM dependency
######################################
message(STATUS "Adding LLVM (version ${FLY_LLVM_VERSION})")
option(ENABLE_LLVM_GTEST "" OFF)
set(FLY_LLVM_PROJECT llvm-project)
set(FLY_LLVM_BINARY_DIR ${CMAKE_BINARY_DIR}/${FLY_LLVM_PROJECT})

if (DEFINED LLVM_BUILD AND LLVM_BUILD)
    set(LLVM_GIT_REPO https://github.com/fly-lang/llvm-project.git)
    message(STATUS "Checkout llvm-project from ${LLVM_GIT_REPO}")

    include(ExternalProject)
    ExternalProject_Add(${FLY_LLVM_PROJECT}
            GIT_REPOSITORY ${LLVM_GIT_REPO}
            GIT_TAG fly-llvm-${FLY_LLVM_VERSION}
            GIT_PROGRESS true
            GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
            DOWNLOAD_EXTRACT_TIMESTAMP true)

    message(STATUS "Building LLVM from sources")
    set(FLY_LLVM_DIR ${FLY_LLVM_BINARY_DIR})
    set(LLVM_DIR ${FLY_LLVM_BINARY_DIR}/llvm/lib/cmake/llvm)
    set(LLD_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/${FLY_LLVM_PROJECT}/lld/include)
    set(LLVM_ENABLE_PROJECTS lld)

    add_subdirectory(${CMAKE_SOURCE_DIR}/${FLY_LLVM_PROJECT}/llvm)
    find_package(LLVM ${FLY_LLVM_VERSION} REQUIRED CONFIG)
    add_definitions(${LLVM_DEFINITIONS})

    message(STATUS "Include LLD directory: ${LLD_INCLUDE_DIRS}")
    include_directories(${LLD_INCLUDE_DIRS})

    include_directories(
            ${CMAKE_SOURCE_DIR}/${FLY_LLVM_PROJECT}/llvm/utils/unittest/googletest/include
            ${CMAKE_SOURCE_DIR}/${FLY_LLVM_PROJECT}/llvm/utils/unittest/googlemock/include)

    #GTest
    set(ENABLE_LLVM_GTEST ON)

else()
    # Precompiled LLVM is extracted into ${CMAKE_BINARY_DIR}/llvm
    set(FLY_LLVM_DIR ${CMAKE_BINARY_DIR}/llvm)

    # Add LLVM Precompiled version
    include(cmake/LLVM-Project.cmake)

    # Configure google-test as a downloadable library.
    include(cmake/GTest.cmake)

    # Backward is a beautiful stack trace pretty printer
    include(cmake/Backward.cmake)

endif()


message(STATUS "Include LLVM directory: ${LLVM_INCLUDE_DIRS}")
include_directories(${LLVM_INCLUDE_DIRS})

# Include Fly headers
include_directories(include)

# Deterministic output path for the runtime static library.
# Must be set before add_subdirectory(src) because src/CMakeLists.txt calls
# configure_file() which bakes FLY_RUNTIME_LIB_DIR into include/Config/Config.h.
# Extract major version number from FLY_LLVM_VERSION (e.g. "20.1.8" → "20")
string(REGEX MATCH "^([0-9]+)" FLY_LLVM_VERSION_MAJOR "${FLY_LLVM_VERSION}")

set(FLY_RUNTIME_LIB_DIR "${CMAKE_BINARY_DIR}/lib")

# Runtime must be configured before compiler/ so configure_file() in compiler/CMakeLists.txt
# bakes the correct FLY_RUNTIME_LIB_DIR into include/Config/Config.h.
add_subdirectory(runtime)

# Add Fly Sources
add_subdirectory(compiler)

# Standard library — compiled by the fly binary, so must come after src/
enable_testing()
add_subdirectory(std)
add_subdirectory(test)

# flyp package manager
add_subdirectory(tools/flyp)
