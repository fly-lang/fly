add_library(FlyParser
        Parser.cpp
        )

target_link_libraries(FlyParser
        LLVMMC
        LLVMMCParser
        LLVMSupport
        FlyBasic
        )