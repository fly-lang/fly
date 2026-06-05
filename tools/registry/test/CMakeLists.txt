######################################
# flyp-registry unit tests
######################################
add_executable(registry_test RegistryHandlerTest.cpp)

target_link_libraries(registry_test PRIVATE
    registry_lib
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(registry_test)
