add_library(FlyAST
        ASTContext.cpp
        PackageDecl.cpp
        )

target_link_libraries(FlyAST
        LLVMBinaryFormat
        LLVMCore
        LLVMSupport
        FlyBasic
        FlyLex
        )