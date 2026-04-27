######################################
# Add Fly Tests
######################################

message(STATUS "Adding Fly Tests")

add_executable(fly_test
        AppTest.cpp
        DriverTest.cpp
        FrontendTest.cpp
        ToolChainTest.cpp
        ImportLibTest.cpp
        ImportTest.cpp
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

# Link backward-cpp stack trace support using its stable cmake variables
# (avoids the deprecated add_backward() and the version-dependent Backward:: targets)
target_link_libraries(fly_test PRIVATE ${BACKWARD_LIBRARIES})
target_include_directories(fly_test PRIVATE ${BACKWARD_INCLUDE_DIRS})

target_link_libraries(fly_test PRIVATE
        gtest_main
        FlyCompiler
        )

target_compile_definitions(fly_test PRIVATE
        FLY_LIB_FLY_DIR="${CMAKE_SOURCE_DIR}/lib/fly"
)

add_test(NAME fly_test COMMAND fly_test)

######################################
# Runtime smoke test (freestanding C)
######################################

add_executable(fly_runtime_test RuntimeTest.c)

target_include_directories(fly_runtime_test PRIVATE ${CMAKE_SOURCE_DIR}/runtime)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(fly_runtime_test PRIVATE
        -ffreestanding -nostdinc -fno-stack-protector -fno-builtin)
    target_link_options(fly_runtime_test PRIVATE -nostdlib -static -Wl,-e,_start)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_options(fly_runtime_test PRIVATE
        -ffreestanding -fno-stack-protector -fno-builtin)
    target_link_options(fly_runtime_test PRIVATE -lSystem)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(MSVC)
        target_compile_options(fly_runtime_test PRIVATE /GS-)
    else()
        target_compile_options(fly_runtime_test PRIVATE -fno-stack-protector -fno-builtin)
    endif()
    target_link_libraries(fly_runtime_test PRIVATE synchronization kernel32)
endif()

target_link_libraries(fly_runtime_test PRIVATE FlyRuntime)

add_test(NAME fly_runtime_test COMMAND fly_runtime_test)
set_tests_properties(fly_runtime_test PROPERTIES PASS_REGULAR_EXPRESSION "fly runtime ok")

######################################
# fly.string library unit tests
######################################

add_executable(fly_lib_test
        LibTest.cpp
)

target_link_libraries(fly_lib_test PRIVATE
        gtest_main
        FlyLib
)

add_test(NAME fly_lib_test COMMAND lib_test)
