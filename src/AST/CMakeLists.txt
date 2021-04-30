add_library(FlyAST
        ASTContext.cpp
        ASTNameSpace.cpp
        ASTNode.cpp
        ASTNodeBase.cpp
        FuncDecl.cpp
        )

target_link_libraries(FlyAST
        LLVMBinaryFormat
        LLVMCore
        LLVMSupport
        FlyBasic
        FlyLex
        )