cmake_minimum_required(VERSION 3.4.3)
project(Fly VERSION 0.0.1)

# specify the C++ standard and flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(FLY_LLVM_VERSION 10.0.1)

# Options
set(BUG_REPORT_URL "https://github.com/fly-lang/fly/issues")

message(STATUS "Fly Project directory: ${CMAKE_SOURCE_DIR}")

# Add LLVM dependency (version 10.0.1)
find_package(LLVM ${FLY_LLVM_VERSION} EXACT QUIET)
if (LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
else()
    add_subdirectory(${CMAKE_SOURCE_DIR}/llvm)
    get_target_property(LLVM_INCLUDE_DIRS LLVMSupport INCLUDE_DIRECTORIES)
endif()

message(STATUS "Using LLVMConfig.cmake from: ${LLVM_DIR}")

if (DEFINED LLVM_INCLUDE_DIRS)
    message(STATUS "Include LLVM directories: ${LLVM_INCLUDE_DIRS}")
    include_directories(${LLVM_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "LLVM_INCLUDE_DIRS not found")
endif()

#Enable Verbose
set(CMAKE_VERBOSE_MAKEFILE ON)
add_link_options("-v")

# Include Fly headers
include_directories(${CMAKE_SOURCE_DIR}/include/)

# Add Sources
add_subdirectory(src)

#Add Tests
enable_testing()
add_subdirectory(test)
