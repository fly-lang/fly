######################################
# Add Fly Tests
######################################

message(STATUS "Adding Fly Tests")

add_executable(fly_test
        AppTest.cpp
        DriverTest.cpp
        FrontendTest.cpp
        ToolChainTest.cpp
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
        TestMain.cpp
        ${BACKWARD_ENABLE}
)

add_backward(fly_test)

target_link_libraries(fly_test PRIVATE
        gtest_main
        flyLib
        )

add_test(NAME fly_test COMMAND fly_test)
