add_library(FlyCodeGen
        CodeGen.cpp
        )

target_link_libraries(FlyCodeGen
        LLVMSupport
        FlyBasic
        FlyAST
        )