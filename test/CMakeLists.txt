######################################
# Add Fly Tests
######################################

message(STATUS "Adding Fly Tests")

add_executable(fly_test
#        CmdTest.cpp
        CodeGen/CodeGenTest.cpp
#        DriverTest.cpp
#        ImportTest.cpp
        Parser/LexerTest.cpp
#        Parser/ParserCommentTest.cpp
#        Parser/ParserExprTest.cpp
#        Parser/ParserFunctionTest.cpp
#        Parser/ParserGlobalVarTest.cpp
#        Parser/ParserImportTest.cpp
#        Parser/ParserLocalVarTest.cpp
#        Parser/ParserBadCode.cpp
#        Parser/ParserBlockTest.cpp
#        Parser/ParserClassTest.cpp
        TestMain.cpp
        ${BACKWARD_ENABLE}
        CodeGen/CodeGenTest.h
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
