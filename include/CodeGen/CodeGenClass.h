//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASS_H
#define FLY_CODEGEN_CLASS_H

#include "CodeGenClassVar.h"
#include "CodeGenClassFunction.h"

namespace llvm {
    class StructType;
    class AllocaInst;
}

namespace fly {

    class CodeGenModule;
    class ASTClass;

    class CodeGenClass {

        CodeGenModule * CGM = nullptr;

        ASTClass *AST = nullptr;

        llvm::StructType *Type = nullptr;

        llvm::PointerType *TypePtr = nullptr;

        llvm::StructType *VTableType = nullptr;

        llvm::SmallVector<llvm::Type *, 4> TypeVector;

        llvm::SmallVector<CodeGenClassVar *, 4> Vars;

        llvm::SmallVector<CodeGenClassFunction *, 4> Constructors;

        llvm::SmallVector<CodeGenClassFunction *, 4> Functions;

    public:
        CodeGenClass(CodeGenModule *CGM, ASTClass *Class, bool isExternal = false);

        void Generate();

        llvm::StructType *getType();

        llvm::PointerType *getTypePtr();

        llvm::StructType *getVTableType();

        const llvm::SmallVector<CodeGenClassVar *, 4> &getVars() const;

        const llvm::SmallVector<CodeGenClassFunction *, 4> &getConstructors() const;

        const llvm::SmallVector<CodeGenClassFunction *, 4> &getFunctions() const;
    };
}

#endif //FLY_CODEGEN_CLASS_H
