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
        FLY_LIB_FLY_DIR="${CMAKE_SOURCE_DIR}/std/src"
)

add_test(NAME fly_test COMMAND fly_test)

include(GoogleTest)
gtest_discover_tests(fly_test)
