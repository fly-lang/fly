//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_INSTANCE_H
#define FLY_CODEGEN_INSTANCE_H

#include "CodeGenVar.h"
#include "AST/ASTCall.h"

namespace llvm {
    class StringRef;
    class StructType;
    class AllocaInst;
}

namespace fly {

    class CodeGenModule;
    class CodeGenClass;
    class ASTClass;
    class ASTClassVar;
    class ASTClassFunction;
    class CodeGenClassVar;
    class CodeGenClassFunction;

    class CodeGenInstance : public CodeGenVarBase {

        ASTClass *Class;

        llvm::StringMap<CodeGenClassVar *> Vars;

    public:

        CodeGenInstance(CodeGenModule *CGM, ASTVar *Var);

        void Init(llvm::Value *Pointer);

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

        CodeGenClassVar *getVar(llvm::StringRef Name);
    };
}

#endif //FLY_CODEGEN_INSTANCE_H
