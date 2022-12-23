//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenInstance.cpp - Instance Generator implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenInstance.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenClassVar.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"

using namespace fly;

CodeGenInstance::CodeGenInstance(CodeGenModule *CGM, ASTClass *Class) : Class(Class) {

    // Create CodeGen fo Class Vars
    uint32_t Index = 0;
    for (auto &Var : Class->getVars()) {
        // Create ClassVar CodeGen
        CodeGenClassVar *CG = new CodeGenClassVar(CGM, Var.second, Class->getCodeGen()->getType(), Index++);
        Var.second->setCodeGen(CG);
    }

    // CodeGenVar.Init() FIXME
//    if (Var->getType()->isClass()) {
//        ((ASTClassType *) Var->getType())->getDef()->getCodeGen()->InvokeDefaultConstructor(Pointer);
//    }
}

void CodeGenInstance::InvokeDefaultConstructor(llvm::Value *Instance) {
    for (auto &Var : Class->getVars()) {
        ((CodeGenClassVar *) Var.second->getCodeGen())->setClassInstance(Instance);
    }
}