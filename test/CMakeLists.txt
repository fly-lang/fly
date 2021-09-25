######################################
# Add Google Test dependency
######################################

message(STATUS "Adding GoogleTest")

add_executable(fly_test
        ASTTest.cpp
        CmdTest.cpp
        CodeGenTest.cpp
        DriverTest.cpp
        FrontendTest.cpp
        LexerTest.cpp
        ParserTest.cpp
        TestMain.cpp
        )

target_link_libraries(fly_test PRIVATE
        gtest_main
        FlyBasic
        FlyLex
        FlyParser
        FlyAST
        FlyCodeGen
        FlyFrontend
        FlyDriver
        )

#configure_file(src/main.fly
#        ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/test/src)
#file(COPY src DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(
        ${CMAKE_SOURCE_DIR}/test/ConfigTest.h.cmake
        ${CMAKE_SOURCE_DIR}/test/ConfigTest.h)
add_test(NAME fly_test COMMAND fly_test)
set_target_properties(fly_test PROPERTIES LINK_FLAGS "-v")
