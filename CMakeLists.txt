cmake_minimum_required(VERSION 3.4.3)
project(Fly VERSION 0.0.1)

# specify the C++ standard and flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

# Options
set(BUG_REPORT_URL "https://github.com/fly-lang/fly/issues")

# Add LLVM dependency (minimum version 10.0.1)
find_package(LLVM "10.0.1" REQUIRED)
if (DEFINED LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake from: ${LLVM_DIR}")
else()
    message(FATAL_ERROR "LLVM not found")
endif()

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
message(STATUS "Include directories: ${LLVM_INCLUDE_DIRS}")

#Enable Verbose
set(CMAKE_VERBOSE_MAKEFILE ON)
add_link_options("-v")

message(STATUS "Project directory: ${CMAKE_SOURCE_DIR}")

#Include Project headers
include_directories(${CMAKE_SOURCE_DIR}/include/)

# Add Sources
add_subdirectory(src)

#Add Tests
enable_testing()
add_subdirectory(test)
