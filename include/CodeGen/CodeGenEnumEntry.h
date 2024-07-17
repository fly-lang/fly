//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_ENUMENTRY_H
#define FLY_CODEGEN_ENUMENTRY_H

#include "CodeGenVarBase.h"
#include "llvm/ADT/StringRef.h"

namespace llvm {
    class Value;
}

namespace fly {

    class CodeGenModule;
    class ASTEnumEntry;

    class CodeGenEnumEntry : public CodeGenVarBase {

        llvm::Value *Value;

        llvm::Value *Instance = nullptr;

    public:
        CodeGenEnumEntry(CodeGenModule *CGM, ASTEnumEntry *EnumEntry);

//        llvm::AllocaInst *Alloca() override;

        llvm::StoreInst *Store(llvm::Value *Val) override;

        llvm::LoadInst *Load() override;

        llvm::Value *getValue() override;

        llvm::Value *getPointer() override;

        CodeGenVarBase *getVar(llvm::StringRef Name) override;
    };
}

#endif //FLY_CODEGEN_ENUMENTRY_H
