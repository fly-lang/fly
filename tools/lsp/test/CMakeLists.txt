######################################
# fly-lsp unit tests
######################################
include(${CMAKE_SOURCE_DIR}/cmake/GTest.cmake)

add_executable(fly_lsp_test
    LspProtocolTest.cpp
    LspTransportTest.cpp
    LspServerTest.cpp
    LspAnalyzerTest.cpp
    LspDocHighlightTest.cpp
)

target_include_directories(fly_lsp_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(fly_lsp_test PRIVATE
    FlyLspLib
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(fly_lsp_test)
