add_library(FlyCodeGen
        CGBlock.cpp
        CGExpr.cpp
        CGFunction.cpp
        CGGlobalVar.cpp
        CGVar.cpp
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