//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenEnumVar.cpp - Code Generator Enum Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenEnumEntry.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTEnumEntry.h"

using namespace fly;

CodeGenEnumEntry::CodeGenEnumEntry(CodeGenModule *CGM, ASTEnumEntry *EnumEntry) :
        Value(llvm::ConstantInt::get(CGM->getCodeGen()->Int32Ty, EnumEntry->getIndex())) {
}

llvm::Value *CodeGenEnumEntry::getValue() {
    return Value;
}

void CodeGenEnumEntry::Init() {

}

StoreInst *CodeGenEnumEntry::Store(llvm::Value *Val) {
    return nullptr;
}

LoadInst *CodeGenEnumEntry::Load() {
    return nullptr;
}

Value *CodeGenEnumEntry::getPointer() {
    return Value;
}

ASTVar *CodeGenEnumEntry::getVar() {
    return nullptr;
}
