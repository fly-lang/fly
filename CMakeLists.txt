cmake_minimum_required(VERSION 3.4.3)
project(Fly VERSION 0.0.1)

# specify the C++ standard and flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

#Fly config options
set(FLY_LLVM_VERSION 10.0.1)
set(FLY_LLVM_PATH ${CMAKE_SOURCE_DIR}/llvm)
set(FLY_BUG_REPORT_URL "https://github.com/fly-lang/fly/issues")

message(STATUS "Fly Project directory: ${CMAKE_SOURCE_DIR}")

# Add LLVM dependency (version 10.0.1)
find_package(LLVM ${FLY_LLVM_VERSION} EXACT QUIET)
if (LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
else()
    message(STATUS "Building LLVM from ${FLY_LLVM_PATH}")
    add_subdirectory(${FLY_LLVM_PATH})
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

# Add Google Test dependency
find_package(GTest REQUIRED QUIET)
if (DEFINED GTEST_FOUND)
    message(STATUS "Found GTest")
    include_directories(${GTEST_INCLUDE_DIRS})
else ()
    message(STATUS "GTest not found, downloading...")
    # Download and unpack googletest at configure time
    configure_file(${CMAKE_SOURCE_DIR}/GTest.CMakeLists.txt googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    message(STATUS "##############${CMAKE_BINARY_DIR}/googletest-src")
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
            ${CMAKE_BINARY_DIR}/googletest-build)
endif ()

#Add Tests
enable_testing()
add_subdirectory(test)
