//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassVar.cpp - Code Generator Class Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenInstance.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenClassVar.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClass.h"
#include "AST/ASTType.h"
#include "AST/ASTClassVar.h"

#include "llvm/ADT/StringMap.h"

using namespace fly;

CodeGenInstance::CodeGenInstance(CodeGenModule *CGM, ASTVar *Var) : CodeGenVarBase(CGM, Var) {
    this->Class = ((ASTClassType *)Var->getType())->getDef();
    this->T = Class->getCodeGen()->getType();
}

void CodeGenInstance::Init(llvm::Value *Pointer) {
    this->Pointer = Pointer;
    uint32_t n = 0;
    for (auto &Var : Class->getCodeGen()->getVars()) {
        CodeGenClassVar *CGV = new CodeGenClassVar(CGM, (ASTClassVar *) Var->getVar(), Class->getCodeGen()->getType(), n++);
        CGV->setInstance(Pointer);

        this->Vars.insert(std::make_pair(Var->getVar()->getName(), CGV));
    }
    CodeGenVarBase::Init();
}

llvm::StoreInst *CodeGenInstance::Store(llvm::Value *Val) {
    return CodeGenVarBase::Store(Val);
}

llvm::LoadInst *CodeGenInstance::Load() {
    return CodeGenVarBase::Load();
}

llvm::Value *CodeGenInstance::getValue() {
    return CodeGenVarBase::getValue();
}

llvm::Value *CodeGenInstance::getPointer() {
    return CodeGenVarBase::getPointer();
}

CodeGenClassVar *CodeGenInstance::getVar(llvm::StringRef Name) {
    return this->Vars.lookup(Name);
}
