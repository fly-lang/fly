######################################
# Add Google Test dependency
######################################

message(STATUS "Adding GoogleTest")
include_directories(
        ../llvm/utils/unittest/googletest
        ../llvm/utils/unittest/googletest/include
        ../llvm/utils/unittest/googlemock
        ../llvm/utils/unittest/googlemock/include
)
add_library(gtest
        ../llvm/utils/unittest/googletest/src/gtest-all.cc
        ../llvm/utils/unittest/googlemock/src/gmock-all.cc
        )

add_library(gtest_main
        TestMain.cpp
        )

target_link_libraries(gtest_main
        gtest
        LLVMSupport)

add_executable(fly_test
        ASTTest.cpp
        CodeGenTest.cpp
        DriverTest.cpp
        FrontendTest.cpp
        LexerTest.cpp
        ParserTest.cpp
        )

target_link_libraries(fly_test PRIVATE
        gtest gtest_main
        FlyBasic
        FlyLex
        FlyParser
        FlyAST
        FlyCodeGen
        FlyFrontend
        FlyDriver
        )

add_test(NAME fly_test COMMAND fly_test)
set_target_properties(fly_test PROPERTIES LINK_FLAGS "-v")
