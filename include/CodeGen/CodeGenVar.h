//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenVar.h - Code Generator of Statements
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_VAR_H
#define FLY_CODEGEN_VAR_H

#include "CodeGenExpr.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Instructions.h>

namespace llvm {
    class Type;
    class ConstantInt;
}

namespace fly {

    class CodeGenModule;
    class ASTVar;
    class SemaVar;
	class CodeGenArrayValue;

    class CodeGenVar : public CodeGenExpr {

    protected:

        SemaVar *Sema = nullptr;

        llvm::Type *T = nullptr;

        size_t Index;

        llvm::Value *Pointer = nullptr;

        llvm::LoadInst *LoadI = nullptr;

        llvm::StringRef BlockID;

    public:

        CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T);

        CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T, size_t Index);

        llvm::Type *getType();

    	llvm::AllocaInst *Alloca();

        llvm::StoreInst *Store(llvm::Value *Val);

    	llvm::Value *StoreArrayValue(CodeGenArrayValue *ArrayValue);

		llvm::Value *getDefaultValue(llvm::Type *T);

		llvm::StoreInst *StoreDefaultValue();

        llvm::LoadInst *Load();

        llvm::Value *getValue() override;

        size_t getIndex();

        llvm::Value *getPointer();

        void setPointer(llvm::Value *Pointer);

    };
}

#endif //FLY_CODEGEN_VAR_H
