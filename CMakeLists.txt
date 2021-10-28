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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

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

macro(addLLVM)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake from: ${LLVM_DIR}")
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
endmacro()

macro(addLLD)
    include_directories(${CMAKE_SOURCE_DIR}/llvm-project/lld/include)
endmacro()

message(STATUS "Searching LLVM (version ${FLY_LLVM_VERSION})")
option(ENABLE_LLVM_GTEST "" OFF)

# Use custom LLVM binary path
# Example -DLLVM_INSTALL_PATH=/usr/lib/llvm/lib/cmake
if (DEFINED LLVM_INSTALL_PATH AND DEFINED LLD_INSTALL_PATH)
    message(STATUS "Use LLVM installation from ${LLVM_INSTALL_PATH}")
    set(LLVM_ENABLE_PROJECTS lld)
    find_package(LLVM ${FLY_LLVM_VERSION} REQUIRED CONFIG PATHS ${LLVM_INSTALL_PATH} NO_DEFAULT_PATH)
    addLLVM()

    # Use custom LLD binary path
    # Example -DLLD_INSTALL_PATH=/usr/lib/llvm-11/lib/cmake/lld
    message(STATUS "Use LLD installation from ${LLD_INSTALL_PATH}")
    find_package(LLD REQUIRED CONFIG PATHS ${LLD_INSTALL_PATH} NO_DEFAULT_PATH)
    set(LLD_INCLUDE_DIR ${LLD_INSTALL_PATH}/include)

# Compile LLVM with this project
# Example -DENABLE_LLVM_PROJECT=true
elseif (DEFINED ENABLE_LLVM_PROJECT AND ENABLE_LLVM_PROJECT)
    message(STATUS "Use llvm-project sources")

    set(LLVM_DIR ${CMAKE_BINARY_DIR}/llvm-project/llvm/lib/cmake/llvm)
    set(LLD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/llvm-project/lld/include)
    set(LLVM_ENABLE_PROJECTS lld)

    add_subdirectory(${CMAKE_SOURCE_DIR}/llvm-project/llvm)
    find_package(LLVM ${FLY_LLVM_VERSION} REQUIRED CONFIG)
    addLLVM()

    #GTest
    set(ENABLE_LLVM_GTEST true)

else()
    # Find LLVM installation
    message(STATUS "Search for LLVM installations" )
    find_package(LLVM ${FLY_LLVM_VERSION} REQUIRED CONFIG)
    addLLVM()

    ## Find LLD installation
    message(STATUS "Search for LLD installations")
    find_package(LLD REQUIRED CONFIG PATHS ${LLVM_DIR}/../lib/cmake/lld NO_DEFAULT_PATH)
endif()

message(STATUS "Include LLD directory: ${LLD_INCLUDE_DIR}")
include_directories(${LLD_INCLUDE_DIR})

#Enable Verbose for best debugging
set(CMAKE_VERBOSE_MAKEFILE ON)
add_link_options("-v")

# Include Fly headers
include_directories(include)

# Add Fly Sources
add_subdirectory(src)

# Add Fly Tests
if (ENABLE_LLVM_GTEST)
    message(STATUS "Use LLVM GTEST")
    include_directories(
            ${CMAKE_SOURCE_DIR}/llvm-project/llvm/utils/unittest/googletest/include
            ${CMAKE_SOURCE_DIR}/llvm-project/llvm/utils/unittest/googlemock/include
            )
else()
    message(STATUS "Use Fly GTEST")
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

enable_testing()
add_subdirectory(test)
