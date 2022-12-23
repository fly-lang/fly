//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenInstance.h - CodeGen Instance
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_INSTANCE_H
#define FLY_CODEGEN_INSTANCE_H

namespace llvm {
    class Value;
}

namespace fly {

    class CodeGenModule;
    class ASTClass;

    class CodeGenInstance {

        CodeGenModule *CGM;

        ASTClass *Class;

    public:
        CodeGenInstance(CodeGenModule *CGM, ASTClass *Class);

        void InvokeDefaultConstructor(llvm::Value *Instance);

    };
}

#endif //FLY_CODEGEN_INSTANCE_H
