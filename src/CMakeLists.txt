add_executable(fly
        Fly.cpp)

add_subdirectory(Basic)
add_subdirectory(Lex)
add_subdirectory(Parser)
add_subdirectory(AST)
add_subdirectory(Sema)
add_subdirectory(CodeGen)
add_subdirectory(Driver)
add_subdirectory(Frontend)

target_link_libraries(fly
        FlyBasic
        FlyLex
        FlyParser
        FlyAST
        FlySema
        FlyCodeGen
        FlyDriver
        FlyFrontend
)

configure_file(
        ${CMAKE_SOURCE_DIR}/include/Config/Config.h.cmake
        ${CMAKE_SOURCE_DIR}/include/Config/Config.h)
