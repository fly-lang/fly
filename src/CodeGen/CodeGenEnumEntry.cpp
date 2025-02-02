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
#include "Sym/SymEnumEntry.h"

using namespace fly;

CodeGenEnumEntry::CodeGenEnumEntry(CodeGenModule *CGM, SymEnumEntry *Sym) : T(CGM->Int32Ty),
        Value(llvm::ConstantInt::get(CGM->Int32Ty, Sym->getIndex())) {
}

llvm::Type *CodeGenEnumEntry::getType() {
    return T;
}

StoreInst *CodeGenEnumEntry::Store(llvm::Value *Val) {
    // FIXME give error
    return nullptr;
}

LoadInst *CodeGenEnumEntry::Load() {
    // FIXME give error
    return nullptr;
}

llvm::Value *CodeGenEnumEntry::getValue() {
    return Value;
}

Value *CodeGenEnumEntry::getPointer() {
    // FIXME give error
    return nullptr;
}

CodeGenVarBase *CodeGenEnumEntry::getVar(llvm::StringRef Name) {
    // FIXME give error
    return nullptr;
}
