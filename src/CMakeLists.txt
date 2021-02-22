add_executable(Fly
        Fly.cpp
        )

add_subdirectory(Basic)
add_subdirectory(Lex)
add_subdirectory(Parser)
add_subdirectory(AST)
add_subdirectory(CodeGen)
add_subdirectory(Driver)
add_subdirectory(Compiler)

target_link_libraries(
        Fly
        FlyBasic
        FlyLex
        FlyParser
        FlyAST
        FlyCodeGen
        FlyDriver
        FlyCompiler
)

configure_file(
        ${CMAKE_SOURCE_DIR}/include/Config/config.h.cmake
        ${CMAKE_SOURCE_DIR}/include/Config/config.h)
