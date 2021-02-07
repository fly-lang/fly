add_executable(Fly Fly.cpp)

add_subdirectory(Basic)
add_subdirectory(Lex)

target_link_libraries(
        Fly
        FlyBasic
        FlyLex
)

configure_file(
        ${CMAKE_SOURCE_DIR}/include/Config/config.h.cmake
        ${CMAKE_SOURCE_DIR}/include/Config/config.h)
