set(FLY_LSP_SOURCES
    lib/LspTransport.cpp
    lib/LspProtocol.cpp
    lib/LspAnalyzer.cpp
    lib/LspServer.cpp
)

add_library(FlyLspLib STATIC ${FLY_LSP_SOURCES})

target_include_directories(FlyLspLib PUBLIC
    include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/compiler
    ${CMAKE_BINARY_DIR}/include
)

# Inherit all LLVM + compiler dependencies through FlyCompiler
target_link_libraries(FlyLspLib PUBLIC FlyCompiler)

# ── fly-lsp executable ───────────────────────────────────────────────────────
add_executable(fly-lsp FlyLsp.cpp)
target_link_libraries(fly-lsp PRIVATE FlyLspLib)
set_target_properties(fly-lsp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY             "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG       "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE     "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL  "${CMAKE_BINARY_DIR}/bin"
)
install(TARGETS fly-lsp RUNTIME DESTINATION bin)

# LSP unit tests
add_subdirectory(test)
