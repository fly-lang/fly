cmake_minimum_required(VERSION 3.24.0)

######################################
# Third-party dependencies
######################################
include(${CMAKE_SOURCE_DIR}/cmake/Tomlplusplus.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CLI11.cmake)

######################################
# flyp_lib — shared logic (no main)
######################################
add_library(flyp_lib STATIC
    util/Checksum.cpp
    util/Error.cpp
    manifest/Manifest.cpp
    manifest/Lockfile.cpp
    resolver/MVSResolver.cpp
    fetcher/Cache.cpp
    fetcher/GitFetcher.cpp
    builder/Builder.cpp
    cli/Commands.cpp
)

target_include_directories(flyp_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(flyp_lib PUBLIC
    tomlplusplus::tomlplusplus
)

set_target_properties(flyp_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

######################################
# flyp — executable
######################################
add_executable(flyp main.cpp)

target_link_libraries(flyp PRIVATE
    flyp_lib
    CLI11::CLI11
)

set_target_properties(flyp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS flyp RUNTIME DESTINATION bin)

######################################
# Tests
######################################
add_subdirectory(test)
