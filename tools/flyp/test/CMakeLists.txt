######################################
# flyp unit tests
######################################
include(${CMAKE_SOURCE_DIR}/cmake/GTest.cmake)

add_executable(flyp_test
    ManifestTest.cpp
    MVSResolverTest.cpp
    SemVerRangeTest.cpp
)

target_link_libraries(flyp_test PRIVATE
    flyp_lib
    GTest::gtest
    GTest::gtest_main
)


include(GoogleTest)
gtest_discover_tests(flyp_test)
