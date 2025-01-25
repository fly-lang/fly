//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenHandle.h - Code Generator of Handle Block
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_HANDLE_H
#define FLY_CODEGEN_HANDLE_H

namespace llvm {
    class BasicBlock;
}

namespace fly {

    class CodeGenModule;
    class CodeGenClass;
    class CodeGenFunctionBase;


    class CodeGenHandle {

        CodeGenModule *CGM = nullptr;

        llvm::BasicBlock *HandleBB;

        llvm::BasicBlock *SafeBB;

    public:

        CodeGenHandle(CodeGenModule *CGM, CodeGenFunctionBase *CGF);

        llvm::BasicBlock *getHandleBlock();

        llvm::BasicBlock *getSafeBlock();
    };
}

#endif //FLY_CODEGEN_HANDLE_H
