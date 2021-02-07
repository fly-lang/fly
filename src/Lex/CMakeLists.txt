add_library(FlyLex
        Lexer.cpp
        LiteralSupport.cpp
        HeaderMap.cpp
        )

target_link_libraries(FlyLex
        LLVMSupport
        FlyBasic
        )