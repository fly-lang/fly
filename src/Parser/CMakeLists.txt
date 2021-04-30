add_library(FlyParser
        ClassParser.cpp
        FunctionParser.cpp
        GlobalVarParser.cpp
        Parser.cpp
        )

target_link_libraries(FlyParser
        LLVMMC
        LLVMMCParser
        LLVMSupport
        FlyBasic
        )