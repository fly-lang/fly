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
        SingleFileBuildTest.cpp
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
        CodeGen/CodeGenDebugInfoTest.cpp
        CodeGen/CodeGenSuiteTest.cpp
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
        Parser/ParserValueTest.cpp
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

# fly_test does not link FlyRuntime or fly_std_lib, but integration tests need
# both to exist on disk (including build/lib/*.fly.h headers) before the test binary runs.
add_dependencies(fly_test FlyRuntime fly_std_lib)

# Place fly_test next to the real fly binary in <build>/bin so the compiler's
# stdlib auto-discovery (<exe>/../lib) resolves to <build>/lib, where fly_std_lib,
# the runtime archive, and the *.fly.h headers live. Without this, multi-config
# generators (MSVC) put fly_test in <build>/test/Release/, making ../lib point at
# the nonexistent <build>/test/lib and breaking every integration test that
# imports the stdlib or links the runtime (ImportLib, AppTest, SingleFileBuild…).
set_target_properties(fly_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY                 "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG           "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE         "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO  "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL      "${CMAKE_BINARY_DIR}/bin"
)

target_compile_definitions(fly_test PRIVATE
        FLY_LIB_FLY_DIR="${CMAKE_SOURCE_DIR}/std/lib"
        FLY_TEST_LIB_DIR="${CMAKE_BINARY_DIR}/lib"
)

include(GoogleTest)

# Integration tests compile/link real Fly source files in the working directory.
# Give them a shared resource lock so they don't race each other when ctest runs
# with -j N.  Unit tests (CodeGen, Parser, …) do no disk I/O and can run freely
# in parallel.
set(_fly_integration_filter
    "AppTest.*:DriverTest.*:ImportLibTest.*:ImportTest.*:SingleFileBuildTest.*")

# GTest negative-filter syntax: positive*-neg1:neg2:neg3  (single '-' starts the
# negative section; subsequent patterns are ':'-separated without extra '-').
set(_fly_unit_filter
    "*:-AppTest.*:DriverTest.*:ImportLibTest.*:ImportTest.*:SingleFileBuildTest.*")

gtest_discover_tests(fly_test
    TEST_FILTER  "${_fly_integration_filter}"
    PROPERTIES   RESOURCE_LOCK "fly_workspace"
)

gtest_discover_tests(fly_test
    TEST_FILTER  "${_fly_unit_filter}"
)
