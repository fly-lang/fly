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
#include "AST/ASTClassAttribute.h"

using namespace fly;

CodeGenInstance::CodeGenInstance(CodeGenModule *CGM, ASTVar *Var) : CodeGenVar(CGM, Var) {
    this->Class = (ASTClass *) ((ASTClassType *)Var->getType())->getDef();
    this->T = Class->getCodeGen()->getType();
}

void CodeGenInstance::Init(llvm::Value *Pointer) {
    this->Pointer = Pointer;
    uint32_t n = 0;
    for (auto &Var : Class->getCodeGen()->getVars()) {
        CodeGenClassVar *CGV = new CodeGenClassVar(CGM, (ASTClassAttribute *) Var->getVar(), Class->getCodeGen()->getType(), n++);
        CGV->setInstance(Pointer);

        this->Vars.insert(std::make_pair(Var->getVar()->getName(), CGV));
    }
//    CodeGenVar::Init();
}

llvm::StoreInst *CodeGenInstance::Store(llvm::Value *Val) {
    return CodeGenVar::Store(Val);
}

llvm::LoadInst *CodeGenInstance::Load() {
    return CodeGenVar::Load();
}

llvm::Value *CodeGenInstance::getValue() {
    return CodeGenVar::getValue();
}

llvm::Value *CodeGenInstance::getPointer() {
    return CodeGenVar::getPointer();
}

CodeGenClassVar *CodeGenInstance::getVar(llvm::StringRef Name) {
    return this->Vars.lookup(Name);
}
