# Add Google Test dependency
find_package(GTest REQUIRED)
if (DEFINED GTEST_FOUND)
    message(STATUS "Found GTest")
    include_directories(${GTEST_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "GTest not found")
endif ()

add_executable(FlyTest
        HeaderMapTest.cpp
        LexerTest.cpp
        ParserTest.cpp
        CompilerTest.cpp
        )

target_link_libraries(FlyTest PRIVATE
        gtest gtest_main
        gmock gmock_main
        FlyBasic FlyLex FlyParser FlyAST FlyDriver FlyCompiler
        )

set_target_properties(FlyTest PROPERTIES LINK_FLAGS "-v")

gtest_discover_tests(FlyTest)

#add_test(NAME lex-tests COMMAND LexTest)
# Lex Test
# set(LexTestArgs --foo 1 --bar 2)
# gtest_add_tests(LexTest
# "${LexTestArgs}"
# AUTO)
