######################################
# Add Fly Tests
######################################

message(STATUS "Adding Fly Tests")

add_executable(fly_test
        CmdTest.cpp
        CodeGenTest.cpp
        DriverTest.cpp
        LexerTest.cpp
        ImportTest.cpp
        ParserTest.cpp
        TestMain.cpp
        ${BACKWARD_ENABLE}
        )

add_backward(fly_test)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIB "bfd" "dl")
    MESSAGE(STATUS "Adding system library: ${SYSTEM_LIB}")
endif()

target_link_libraries(fly_test PRIVATE
        gtest_main
        ${SYSTEM_LIB}
        flyLib
        )

# Copy all .fly from test/src/ dir
file(COPY src DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(
        ${CMAKE_SOURCE_DIR}/test/TestConfig.h.cmake
        ${CMAKE_SOURCE_DIR}/test/TestConfig.h)
add_test(NAME fly_test COMMAND fly_test)
