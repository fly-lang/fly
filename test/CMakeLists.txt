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

# Compile with /MT instead of /MD using CMake
# https://stackoverflow.com/questions/14172856/compile-with-mt-instead-of-md-using-cmake
set_property(TARGET fly_test PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_test(NAME fly_test COMMAND fly_test)
set_target_properties(fly_test PROPERTIES LINK_FLAGS "-v")
