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
set(FLY_GTEST_MIN_VERSION 1.7.0)

message(STATUS "Fly Project directory: ${CMAKE_SOURCE_DIR}")

######################################
# Add LLVM dependency (version 10.0.1)
######################################

message(STATUS "Searching LLVM version ${FLY_LLVM_VERSION}")
find_package(LLVM ${FLY_LLVM_VERSION} EXACT QUIET)
if (LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
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

######################################
# Add Google Test dependency
######################################

message(STATUS "Searching GoogleTest min version ${FLY_GTEST_MIN_VERSION}")
find_package(GTest ${FLY_GTEST_MIN_VERSION} QUIET)
if (DEFINED GTEST_FOUND)
    message(STATUS "Found GoogleTest ${GTest_PACKAGE_VERSION}")
else ()
    message(STATUS "GoogleTest ${GOOGLETEST_VERSION} not found, downloading...")
    # Download and unpack googletest at configure time
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
            EXCLUDE_FROM_ALL)
    set(GTEST_INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-src/googletest/include
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-src/googlemock/include)
endif ()
if (DEFINED GTEST_INCLUDE_DIRS)
    message(STATUS "Include GoogleTest directories: ${GTEST_INCLUDE_DIRS}")
    include_directories(${GTEST_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "GTEST_INCLUDE_DIRS not found")
endif()

#Enable Verbose
set(CMAKE_VERBOSE_MAKEFILE ON)
add_link_options("-v")

# Include Fly headers
include_directories(${CMAKE_SOURCE_DIR}/include/)

# Add Sources
add_subdirectory(src)

# Add Tests
enable_testing()
add_subdirectory(test)
