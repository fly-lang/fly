######################################
# Add Google Test dependency
######################################

message(STATUS "Adding GoogleTest")
include_directories(
        ./utils/googletest
        ./utils/googletest/include
        ./utils/googlemock
        ./utils/googlemock/include
)
add_library(fly_gtest
        ./utils/googletest/src/gtest-all.cc
        ./utils/googlemock/src/gmock-all.cc
        )

add_library(fly_gtest_main
        TestMain.cpp
        )

target_link_libraries(fly_gtest_main
        fly_gtest
        LLVMSupport)

add_executable(fly_test
        ASTTest.cpp
        CmdTest.cpp
        CodeGenTest.cpp
        DriverTest.cpp
        FrontendTest.cpp
        LexerTest.cpp
        ParserTest.cpp
        )

target_link_libraries(fly_test PRIVATE
        fly_gtest fly_gtest_main
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
