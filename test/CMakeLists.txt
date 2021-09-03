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
# Resolve SEH exception with code 0xc0000005 thrown in the test body
if (MSVC)
    target_compile_options(fly_test PRIVATE /Zp1)
endif()

target_link_libraries(fly_test PRIVATE
        gmock_main
        FlyBasic
        FlyLex
        FlyParser
        FlyAST
        FlyCodeGen
        FlyFrontend
        FlyDriver
        )

configure_file(src/main.fly
        ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_test(NAME fly_test COMMAND fly_test)
set_target_properties(fly_test PROPERTIES LINK_FLAGS "-v")
