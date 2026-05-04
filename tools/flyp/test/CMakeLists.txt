######################################
# flyp unit tests
######################################
include(${CMAKE_SOURCE_DIR}/cmake/GTest.cmake)

add_executable(flyp_test
    ManifestTest.cpp
    MVSResolverTest.cpp
)

target_link_libraries(flyp_test PRIVATE
    flyp_lib
    GTest::gtest
    GTest::gtest_main
)

set_target_properties(flyp_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

include(GoogleTest)
gtest_discover_tests(flyp_test)
