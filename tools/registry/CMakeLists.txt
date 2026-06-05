cmake_minimum_required(VERSION 3.24.0)

include(${CMAKE_SOURCE_DIR}/cmake/CLI11.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/GTest.cmake)

######################################
# registry_lib — handler logic (no main, no sockets)
######################################
add_library(registry_lib STATIC lib/RegistryHandler.cpp)

target_include_directories(registry_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

######################################
# flyp-registry — server executable
######################################
add_executable(flyp-registry FlypRegistry.cpp)

target_link_libraries(flyp-registry PRIVATE
    registry_lib
    CLI11::CLI11
)

set_target_properties(flyp-registry PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS flyp-registry RUNTIME DESTINATION bin)

######################################
# Tests
######################################
add_subdirectory(test)
