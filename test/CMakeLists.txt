######################################
# Add Fly Tests
######################################

message(STATUS "Adding Fly Tests")

add_executable(fly_test
        AppTest.cpp
        DriverTest.cpp
        FrontendTest.cpp
        ToolChainTest.cpp
        ImportLibTest.cpp
        ImportTest.cpp
        CodeGen/CodeGenTest.cpp
        CodeGen/CodeGenBaseTest.cpp
        CodeGen/CodeGenErrorTest.cpp
        CodeGen/CodeGenArrayTest.cpp
        CodeGen/CodeGenClassTest.cpp
        CodeGen/CodeGenEnumTest.cpp
        CodeGen/CodeGenInterfaceTest.cpp
        CodeGen/CodeGenStringTest.cpp
        CodeGen/CodeGenStructTest.cpp
        CodeGen/CodeGenSmartAllocTest.cpp
        CodeGen/CodeGenDebugInfoTest.cpp
        CodeGen/CodeGenSuiteTest.cpp
        Parser/LexerTest.cpp
#        Parser/ParserBadCode.cpp
        Parser/ParserBlockTest.cpp
        Parser/ParserClassTest.cpp
        Parser/ParserCommentTest.cpp
        Parser/ParserExprTest.cpp
        Parser/ParserEnumTest.cpp
        Parser/ParserErrorHandlerTest.cpp
        Parser/ParserFunctionTest.cpp
        Parser/ParserNameSpaceTest.cpp
        Parser/ParserLocalVarTest.cpp
        Parser/ParserValueTest.cpp
        TestMain.cpp
        ${BACKWARD_ENABLE}
)

# Link backward-cpp stack trace support using its stable cmake variables
# (avoids the deprecated add_backward() and the version-dependent Backward:: targets)
target_link_libraries(fly_test PRIVATE ${BACKWARD_LIBRARIES})
target_include_directories(fly_test PRIVATE ${BACKWARD_INCLUDE_DIRS})

target_link_libraries(fly_test PRIVATE
        gtest_main
        FlyCompiler
        )

# fly_test does not link FlyRuntime, but integration tests (AppTest, ToolChainTest)
# need libFlyRuntime.a to exist on disk before the test binary runs.
add_dependencies(fly_test FlyRuntime)

target_compile_definitions(fly_test PRIVATE
        FLY_LIB_FLY_DIR="${CMAKE_SOURCE_DIR}/std/lib"
)

include(GoogleTest)

# Integration tests compile/link real Fly source files in the working directory.
# Give them a shared resource lock so they don't race each other when ctest runs
# with -j N.  Unit tests (CodeGen, Parser, …) do no disk I/O and can run freely
# in parallel.
set(_fly_integration_filter
    "AppTest.*:DriverTest.*:ImportLibTest.*:ImportTest.*")

# GTest negative-filter syntax: positive*-neg1:neg2:neg3  (single '-' starts the
# negative section; subsequent patterns are ':'-separated without extra '-').
set(_fly_unit_filter
    "*:-AppTest.*:DriverTest.*:ImportLibTest.*:ImportTest.*")

gtest_discover_tests(fly_test
    TEST_FILTER  "${_fly_integration_filter}"
    PROPERTIES   RESOURCE_LOCK "fly_workspace"
)

gtest_discover_tests(fly_test
    TEST_FILTER  "${_fly_unit_filter}"
)
