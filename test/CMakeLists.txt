add_executable(FlyTest
        CodeGenTest.cpp
        DriverTest.cpp
        FrontendTest.cpp
#        HeaderMapTest.cpp
        LexerTest.cpp
        ParserTest.cpp
        )

target_link_libraries(FlyTest PRIVATE
        gtest gtest_main
        FlyBasic
        FlyLex
        FlyParser
        FlyAST
        FlyCodeGen
        FlyFrontend
        FlyDriver
        )

add_test(NAME FlyTest COMMAND FlyTest)
set_target_properties(FlyTest PROPERTIES LINK_FLAGS "-v")
