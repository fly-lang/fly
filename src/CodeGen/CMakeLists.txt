add_library(FlyCodeGen
        CodeGenBlock.cpp
        CodeGenExpr.cpp
        CodeGenFunction.cpp
        CodeGenGlobalVar.cpp
        CodeGenLocalVar.cpp
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