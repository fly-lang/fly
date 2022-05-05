add_library(FlyParser
        ClassParser.cpp
        ExprParser.cpp
        FunctionParser.cpp
        GlobalVarParser.cpp
        NumberParser.cpp
        Parser.cpp
        )

target_link_libraries(FlyParser
        LLVMMC
        LLVMMCParser
        LLVMSupport
        FlyBasic
        FlySema
        )