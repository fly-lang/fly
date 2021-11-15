cmake_minimum_required(VERSION 3.4.3)
project(Fly VERSION 0.1.0 LANGUAGES C CXX ASM)
#set(FLY_VERSION_PHASE alpha) #remove this line for stable release
if(DEFINED FLY_VERSION_PHASE)
    set(FLY_VERSION "${PROJECT_VERSION}-${FLY_VERSION_PHASE}")
else()
    set(FLY_VERSION "${PROJECT_VERSION}")
endif()

message(STATUS "Fly Project directory: ${CMAKE_SOURCE_DIR}")

# specify the C++ standard and flags
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS NO)
if (NOT MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
if(MSVC)
    set(CMAKE_C_FLAGS_DEBUG "/W4")
    set(CMAKE_CXX_FLAGS_DEBUG "/W4")
    set(CMAKE_C_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "C Flags: ${CMAKE_C_FLAGS_${BUILD_TYPE}}")
message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")
if (BUILD_TYPE STREQUAL DEBUG)
    set(LLVM_ENABLE_ASSERTIONS "On")
    message(STATUS "LLVM_ENABLE_ASSERTIONS: On")
endif()

#Fly config options
set(FLY_LLVM_VERSION 11.1.0)
set(FLY_BUG_REPORT_URL "https://github.com/fly-lang/fly/issues")
set(FLY_LLD_INCLUDE_DIRS )

######################################
# Add LLVM dependency
######################################
message(STATUS "Adding LLVM (version ${FLY_LLVM_VERSION})")
option(ENABLE_LLVM_GTEST "" OFF)

if (DEFINED LLVM_BUILD AND LLVM_BUILD)
    set(LLVM_GIT_REPO https://github.com/fly-lang/llvm-project.git)
    message(STATUS "Checkout llvm-project from ${LLVM_GIT_REPO}")

    include(ExternalProject)
    ExternalProject_Add(llvm-project
            GIT_REPOSITORY https://github.com/fly-lang/llvm-project.git
            GIT_TAG fly-llvm-${FLY_LLVM_VERSION}
            GIT_PROGRESS true
            GIT_REMOTE_UPDATE_STRATEGY CHECKOUT)

    message(STATUS "Building LLVM from sources")
    set(LLVM_DIR ${CMAKE_BINARY_DIR}/llvm-project/llvm/lib/cmake/llvm)
    set(LLD_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/llvm-project/lld/include)
    set(LLVM_ENABLE_PROJECTS lld)

    add_subdirectory(${CMAKE_SOURCE_DIR}/llvm-project/llvm)
    find_package(LLVM ${FLY_LLVM_VERSION} REQUIRED CONFIG)
    add_definitions(${LLVM_DEFINITIONS})

    message(STATUS "Include LLD directory: ${LLD_INCLUDE_DIRS}")
    include_directories(${LLD_INCLUDE_DIRS})

    include_directories(
            ${CMAKE_SOURCE_DIR}/llvm-project/llvm/utils/unittest/googletest/include
            ${CMAKE_SOURCE_DIR}/llvm-project/llvm/utils/unittest/googlemock/include)

    #GTest
    set(ENABLE_LLVM_GTEST true)

else()
    # Download precompiled LLVM
    message(STATUS "Downloading LLVM precompiled files")

    set(LLVM_BUILD_PATH ${CMAKE_BINARY_DIR}/llvm-project)
    set(LLVM_RELEASES_PREFIX_URL https://github.com/fly-lang/llvm-project/releases/download)
    if (MSVC)
        set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v11.1.0-win-x64/llvm-11.1.0-win-x64.zip")
        set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.zip")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v11.1.0-macos-x86_64/llvm-11.1.0-x86_64-apple-darwin.tar.gz")
        set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v11.1.0-linux-x86_64/llvm-11.1.0-x86_64-linux-gnu.tar.gz")
        set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    else()
        message(FATAL_ERROR "Unknown system, cannot download pre-build llvm packages, run with -DLLVM_BUILD")
    endif()

    file(DOWNLOAD ${LLVM_RELEASES_URL} ${LLVM_DOWNLOAD_FILE})
    file(ARCHIVE_EXTRACT
            INPUT ${LLVM_DOWNLOAD_FILE}
            DESTINATION ${LLVM_BUILD_PATH})
    file(REMOVE ${LLVM_DOWNLOAD_FILE})

    message(STATUS "Downloaded precompiled LLVM from ${LLVM_RELEASES_URL}")
    find_package(LLD REQUIRED CONFIG PATHS ${LLVM_BUILD_PATH} NO_DEFAULT_PATH)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    add_definitions(${LLVM_DEFINITIONS})

    add_subdirectory(test/googletest)
    # Add Fly Tests
    # Prevent GoogleTest from overriding our compiler/linker options
    # when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    if (MSVC)
        message(STATUS "MSVC Build -> GTEST_LINKED_AS_SHARED_LIBRARY 1")
        set(GTEST_LINKED_AS_SHARED_LIBRARY 1)
    endif()

endif()

message(STATUS "Include LLVM directory: ${LLVM_INCLUDE_DIRS}")
include_directories(${LLVM_INCLUDE_DIRS})

# Include Fly headers
include_directories(include)

# Add Fly Sources
add_subdirectory(src)

enable_testing()
add_subdirectory(test)
