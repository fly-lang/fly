include(FetchContent)

# Configure LLVM precompiled parameters
set(LLVM_RELEASES_PREFIX_URL https://github.com/fly-lang/llvm-project/releases/download)

if (MSVC)
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v${FLY_LLVM_VERSION}-win-x64/llvm-${FLY_LLVM_VERSION}-win-x64.zip")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.zip")
    set(LLVM_HASH 5368d07f4cee65d53b2b4c4ef3726942911b6229fea61f0a883cb1b4bb76db57)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v${FLY_LLVM_VERSION}-macos-x86_64/llvm-${FLY_LLVM_VERSION}-x86_64-apple-darwin.tar.gz")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    set(LLVM_HASH 0000000000000000000000000000000000000000)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LLVM_RELEASES_URL "${LLVM_RELEASES_PREFIX_URL}/v${FLY_LLVM_VERSION}-linux-x86_64/llvm-${FLY_LLVM_VERSION}-x86_64-linux-gnu.tar.gz")
    set(LLVM_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/llvm.tar.gz")
    set(LLVM_HASH 8664061d7282113e1e9a1c7d7fcc4def96a7762d84f275cd49e19d6e70b6eb0a)
else()
    message(FATAL_ERROR "Unknown system, cannot download pre-build llvm packages, run with -DLLVM_BUILD")
endif()

# Download and extract the precompiled LLVM package.
# We download manually first so we can show a clear error with the actual
# hash when the package has been updated (instead of CMake's cryptic message).
message(STATUS "Downloading LLVM from ${LLVM_RELEASES_URL}")
get_filename_component(LLVM_ARCHIVE_NAME "${LLVM_RELEASES_URL}" NAME)
set(LLVM_ARCHIVE "${CMAKE_BINARY_DIR}/${LLVM_ARCHIVE_NAME}")
if(NOT EXISTS "${LLVM_ARCHIVE}")
    file(DOWNLOAD "${LLVM_RELEASES_URL}" "${LLVM_ARCHIVE}" SHOW_PROGRESS STATUS dl_status)
    list(GET dl_status 0 dl_code)
    if(NOT dl_code EQUAL 0)
        file(REMOVE "${LLVM_ARCHIVE}")
        message(FATAL_ERROR "Failed to download LLVM package: ${dl_status}")
    endif()
endif()
file(SHA256 "${LLVM_ARCHIVE}" actual_hash)
if(NOT actual_hash STREQUAL LLVM_HASH)
    file(REMOVE "${LLVM_ARCHIVE}")
    message(FATAL_ERROR
        "LLVM package hash mismatch — the precompiled package has changed.\n"
        "  File:     ${LLVM_RELEASES_URL}\n"
        "  Expected: ${LLVM_HASH}\n"
        "  Actual:   ${actual_hash}\n"
        "Update LLVM_HASH in cmake/LLVM-Project.cmake with the actual value above.")
endif()

FetchContent_Declare(
        ${FLY_LLVM_PROJECT}
        URL "${LLVM_ARCHIVE}"
        SOURCE_DIR llvm
        DOWNLOAD_EXTRACT_TIMESTAMP true)
FetchContent_MakeAvailable(${FLY_LLVM_PROJECT})

message(STATUS "Downloaded LLVM precompiled files")

# The precompiled LLVM package may have been built on a machine with a different
# Visual Studio edition (e.g. Enterprise) than the one available locally.
# Patch LLVMExports.cmake to replace any hardcoded VS edition path with the
# actual installation path of VS on the current machine so diaguids.lib is found correctly.
if(MSVC)
    set(_llvm_exports_file "${CMAKE_BINARY_DIR}/llvm/lib/cmake/llvm/LLVMExports.cmake")
    if(EXISTS "${_llvm_exports_file}")
        # Use vswhere.exe to reliably find the VS installation path regardless of
        # whether VSINSTALLDIR env var is set (it is only set in Developer Prompt).
        # Note: $ENV{ProgramFiles(x86)} is invalid in CMake (parentheses not allowed),
        # so we use find_program with known fallback paths instead.
        find_program(_vswhere vswhere.exe
            PATHS
                "C:/Program Files (x86)/Microsoft Visual Studio/Installer"
                "C:/Program Files/Microsoft Visual Studio/Installer"
            NO_DEFAULT_PATH)
        if(_vswhere)
            execute_process(
                COMMAND "${_vswhere}" -latest -property installationPath
                OUTPUT_VARIABLE _vs_install_path
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        endif()
        # Fallback to VSINSTALLDIR env var if vswhere is not available
        if(NOT _vs_install_path AND DEFINED ENV{VSINSTALLDIR})
            file(TO_CMAKE_PATH "$ENV{VSINSTALLDIR}" _vs_install_path)
            string(REGEX REPLACE "/$" "" _vs_install_path "${_vs_install_path}")
        endif()
        if(_vs_install_path)
            file(TO_CMAKE_PATH "${_vs_install_path}" _vs_install_path)
            file(READ "${_llvm_exports_file}" _llvm_exports_content)
            string(REGEX REPLACE
                "C:/Program Files/Microsoft Visual Studio/[0-9]+/[^/\"]+/DIA SDK"
                "${_vs_install_path}/DIA SDK"
                _llvm_exports_patched
                "${_llvm_exports_content}")
            if(NOT _llvm_exports_patched STREQUAL _llvm_exports_content)
                file(WRITE "${_llvm_exports_file}" "${_llvm_exports_patched}")
                message(STATUS "Patched LLVMExports.cmake: DIA SDK path -> ${_vs_install_path}/DIA SDK")
            endif()
        else()
            message(WARNING "Could not determine VS installation path — LLVMExports.cmake not patched. diaguids.lib may not be found.")
        endif()
    endif()
endif()

# The precompiled LLVM package ships Findzstd.cmake in its cmake dir.
# Add it to CMAKE_MODULE_PATH so find_package(zstd) uses it.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/llvm/lib/cmake/llvm")

# zstd is required by the precompiled LLVM (linked as zstd::libzstd_shared).
# Search order:
#   1. System cmake config  (finds libzstd-dev on Linux, vcpkg on Windows)
#   2. find_library fallback with common library names (static variants included)
#   3. FetchContent: download and build zstd statically from source
find_package(zstd QUIET)
if(NOT zstd_FOUND)
    find_library(ZSTD_LIBRARY
        NAMES zstd libzstd zstd_static libzstd_static
        PATHS "${CMAKE_BINARY_DIR}/llvm/lib"
    )
    if(ZSTD_LIBRARY)
        add_library(zstd::libzstd_shared UNKNOWN IMPORTED GLOBAL)
        set_target_properties(zstd::libzstd_shared PROPERTIES
            IMPORTED_LOCATION "${ZSTD_LIBRARY}")
    else()
        message(STATUS "zstd not found — downloading and building from source")
        FetchContent_Declare(
            zstd_fc
            URL      https://github.com/facebook/zstd/releases/download/v1.5.5/zstd-1.5.5.tar.gz
            URL_HASH SHA256=9c4396cc829cfae319a6e2615202e82aad41372073482fce286fac78646d3ee4
            SOURCE_SUBDIR  build/cmake
            DOWNLOAD_EXTRACT_TIMESTAMP true
        )
        set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_STATIC   ON  CACHE BOOL "" FORCE)
        set(ZSTD_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
        # zstd 1.5.5 ships a CMakeLists with `cmake_minimum_required(VERSION 2.8.x)`, which
        # CMake >= 4.0 (e.g. the Windows CI runner) rejects ("Compatibility with CMake < 3.5
        # has been removed"). Raise the policy version floor for the FetchContent subproject;
        # the variable is ignored by older CMake, so Linux/macOS builds are unaffected.
        set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
        FetchContent_MakeAvailable(zstd_fc)
        unset(CMAKE_POLICY_VERSION_MINIMUM)
        # Expose the static target under the name the LLVM cmake config expects
        add_library(zstd::libzstd_shared ALIAS libzstd_static)
    endif()
endif()

find_package(LLD REQUIRED CONFIG PATHS ${CMAKE_BINARY_DIR}/llvm NO_DEFAULT_PATH)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
add_definitions(${LLVM_DEFINITIONS})
