add_library(FlyCodeGen
        BackendUtil.cpp
        CodeGen.cpp
        CodeGenModule.cpp
        CodeGenOptions.cpp)

target_link_libraries(FlyCodeGen
        LLVMSupport
        LLVMPasses
        FlyBasic
        FlyAST
        )