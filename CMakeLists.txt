cmake_minimum_required(VERSION 3.4.3)
project(Fly VERSION 0.0.3)

# specify the C++ standard and flags
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS NO)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

#Fly config options
set(FLY_LLVM_VERSION 11.1.0)
set(FLY_LLVM_PATH ${CMAKE_SOURCE_DIR}/llvm)
set(FLY_BUG_REPORT_URL "https://github.com/fly-lang/fly/issues")

message(STATUS "Fly Project directory: ${CMAKE_SOURCE_DIR}")

######################################
# Add LLVM dependency (version 11)
######################################

message(STATUS "Searching LLVM (version ${FLY_LLVM_VERSION})")
find_package(LLVM 11.1.0 QUIET)
if (LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake from: ${LLVM_DIR}")
else()
    message(STATUS "Building LLVM from ${FLY_LLVM_PATH}")
    add_subdirectory(${FLY_LLVM_PATH})
    get_target_property(LLVM_INCLUDE_DIRS LLVMSupport INCLUDE_DIRECTORIES)
endif()
if (DEFINED LLVM_INCLUDE_DIRS)
    message(STATUS "Include LLVM directories: ${LLVM_INCLUDE_DIRS}")
    include_directories(${LLVM_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "LLVM_INCLUDE_DIRS not found")
endif()

#Enable Verbose for best debugging
set(CMAKE_VERBOSE_MAKEFILE ON)
add_link_options("-v")

# Include Fly headers
include_directories(include)

# Add Fly Sources
add_subdirectory(src)

# Add Fly Tests
enable_testing()
add_subdirectory(test)
