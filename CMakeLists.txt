cmake_minimum_required(VERSION 3.4.3)
project(Fly VERSION 0.0.4 LANGUAGES C CXX ASM)

# specify the C++ standard and flags
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS NO)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

#Fly config options
set(FLY_LLVM_VERSION 11.1.0)
set(FLY_BUG_REPORT_URL "https://github.com/fly-lang/fly/issues")

message(STATUS "Fly Project directory: ${CMAKE_SOURCE_DIR}")

######################################
# Add LLVM dependency (version 11)
######################################

macro(addLLVM)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake from: ${LLVM_DIR}")
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
endmacro()

message(STATUS "Searching LLVM (version ${FLY_LLVM_VERSION})")

# Default your installed LLVM is used if the version is equal to ${FLY_LLVM_VERSION}
# Force to use LLVM from this project by -DUSE_INSTALLED_LLVM=false
if (USE_INSTALLED_LLVM OR NOT DEFINED USE_INSTALLED_LLVM)
    find_package(LLVM ${FLY_LLVM_VERSION} REQUIRED CONFIG)
    if (DEFINED LLVM_FOUND)
        addLLVM()
    endif()
endif()

# Use LLVM from this project
if (NOT DEFINED LLVM_FOUND)
    set(FLY_LLVM_DIR ${CMAKE_BINARY_DIR}/llvm/lib/cmake/llvm)
    set(LLVM_DIR ${FLY_LLVM_DIR})
    add_subdirectory(${CMAKE_SOURCE_DIR}/llvm)
    find_package(LLVM REQUIRED CONFIG)
    addLLVM()
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
