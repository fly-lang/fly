//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenEnumVar.cpp - Code Generator Enum Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenEnumEntry.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaEnumEntry.h"

#include <llvm/IR/Constants.h>

using namespace fly;

CodeGenEnumEntry::CodeGenEnumEntry(CodeGenModule *CGM, SemaEnumEntry *Sema) : T(CGM->Int32Ty),
        Value(llvm::ConstantInt::get(CGM->Int32Ty, Sema->getIndex())) {
}

llvm::Type *CodeGenEnumEntry::getType() {
    return T;
}

llvm::StoreInst *CodeGenEnumEntry::Store(llvm::Value *Val) {
    // FIXME give error
    return nullptr;
}

llvm::LoadInst *CodeGenEnumEntry::Load() {
    // FIXME give error
    return nullptr;
}

llvm::Value *CodeGenEnumEntry::getValue() {
    return Value;
}

llvm::Value *CodeGenEnumEntry::getPointer() {
    // FIXME give error
    return nullptr;
}

CodeGenVarBase *CodeGenEnumEntry::getVar(llvm::StringRef Name) {
    // FIXME give error
    return nullptr;
}
