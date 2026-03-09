######################################
# Add Fly Tests
######################################

message(STATUS "Adding Fly Tests")

add_executable(fly_test
#        CmdTest.cpp
#        DriverTest.cpp
#        ImportTest.cpp
        CodeGen/CodeGenTest.cpp
        CodeGen/CodeGenBaseTest.cpp
        CodeGen/CodeGenErrorTest.cpp
        CodeGen/CodeGenArrayTest.cpp
#        CodeGen/CodeGenClassTest.cpp
        CodeGen/CodeGenEnumTest.cpp
        CodeGen/CodeGenStringTest.cpp
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

# Copy all .fly from test/src/ dir
file(COPY src DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(
        ${CMAKE_SOURCE_DIR}/test/TestConfig.h.cmake
        ${CMAKE_SOURCE_DIR}/test/TestConfig.h)
add_test(NAME fly_test COMMAND fly_test)
