add_library(FlyCodeGen
        BackendUtil.cpp
        CodeGen.cpp
        CodeGenModule.cpp
        )

target_link_libraries(FlyCodeGen
        LLVMSupport
        LLVMPasses
        LLVMLTO
        FlyBasic
        FlyAST
        )