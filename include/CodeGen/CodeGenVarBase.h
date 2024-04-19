//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenVarBase.h - Code Generator of Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_VARBASE_H
#define FLY_CODEGEN_VARBASE_H

namespace llvm {
    class StoreInst;
    class LoadInst;
    class Value;
}

namespace fly {

    class ASTVar;

    class CodeGenVarBase {

    public:

        virtual void Init() = 0;

        virtual llvm::StoreInst *Store(llvm::Value *Val) = 0;

        virtual llvm::LoadInst *Load() = 0;

        virtual llvm::Value *getValue() = 0;

        virtual llvm::Value *getPointer() = 0;

        virtual ASTVar *getVar() = 0;
    };
}

#endif //FLY_CODEGEN_VARBASE_H
