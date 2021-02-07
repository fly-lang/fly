add_executable(LexTest
  HeaderMapTest.cpp
  LexerTest.cpp
)

target_link_libraries(LexTest PRIVATE
        gtest gtest_main
        gmock gmock_main
        FlyBasic FlyLex
)

set_target_properties(LexTest PROPERTIES LINK_FLAGS "-v")

gtest_discover_tests(LexTest)

#add_test(NAME lex-tests COMMAND LexTest)
# Lex Test
# set(LexTestArgs --foo 1 --bar 2)
# gtest_add_tests(LexTest
# "${LexTestArgs}" 
# AUTO)
