add_executable(FlyTest
        HeaderMapTest.cpp
        LexerTest.cpp
        ParserTest.cpp
        FrontendTest.cpp
        DriverTest.cpp
        )

target_link_libraries(FlyTest PRIVATE
        gtest gtest_main
        gmock gmock_main
        FlyBasic FlyLex FlyParser FlyAST FlyCodeGen FlyFrontend FlyDriver
        )

gtest_discover_tests(FlyTest)
set_target_properties(FlyTest PROPERTIES LINK_FLAGS "-v")
