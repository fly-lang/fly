######################################
# Add Fly Tests
######################################

message(STATUS "Adding Fly Tests")

add_executable(fly_test
        AppTest.cpp
        DriverTest.cpp
        FrontendTest.cpp
        ToolChainTest.cpp
        CodeGen/CodeGenTest.cpp
        CodeGen/CodeGenBaseTest.cpp
        CodeGen/CodeGenErrorTest.cpp
        CodeGen/CodeGenArrayTest.cpp
        CodeGen/CodeGenClassTest.cpp
        CodeGen/CodeGenEnumTest.cpp
        CodeGen/CodeGenInterfaceTest.cpp
        CodeGen/CodeGenStringTest.cpp
        CodeGen/CodeGenStructTest.cpp
        CodeGen/CodeGenSmartAllocTest.cpp
        Parser/LexerTest.cpp
#        Parser/ParserBadCode.cpp
        Parser/ParserBlockTest.cpp
        Parser/ParserClassTest.cpp
        Parser/ParserCommentTest.cpp
        Parser/ParserExprTest.cpp
        Parser/ParserEnumTest.cpp
        Parser/ParserErrorHandlerTest.cpp
        Parser/ParserFunctionTest.cpp
        Parser/ParserNameSpaceTest.cpp
        Parser/ParserLocalVarTest.cpp
        TestMain.cpp
        ${BACKWARD_ENABLE}
)

add_backward(fly_test)

target_link_libraries(fly_test PRIVATE
        gtest_main
        FlyCompiler
        )

add_test(NAME fly_test COMMAND fly_test)

######################################
# Runtime smoke test (freestanding C)
######################################

add_executable(RuntimeTest RuntimeTest.c)

target_include_directories(RuntimeTest PRIVATE ${CMAKE_SOURCE_DIR}/runtime)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(RuntimeTest PRIVATE
        -ffreestanding -nostdinc -fno-stack-protector -fno-builtin)
    target_link_options(RuntimeTest PRIVATE -nostdlib -static -Wl,-e,_start)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_options(RuntimeTest PRIVATE
        -ffreestanding -fno-stack-protector -fno-builtin)
    target_link_options(RuntimeTest PRIVATE -lSystem)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(MSVC)
        target_compile_options(RuntimeTest PRIVATE /GS-)
    else()
        target_compile_options(RuntimeTest PRIVATE -fno-stack-protector -fno-builtin)
    endif()
    target_link_libraries(RuntimeTest PRIVATE synchronization kernel32)
endif()

target_link_libraries(RuntimeTest PRIVATE Runtime)

add_test(NAME RuntimeSmoke COMMAND RuntimeTest)
set_tests_properties(RuntimeSmoke PROPERTIES PASS_REGULAR_EXPRESSION "fly runtime ok")
