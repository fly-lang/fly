//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClass.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTNameSpace.h"
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CodeGenClass::CodeGenClass(CodeGenModule *CGM, ASTClass *Class, bool isExternal) : CGM(CGM), AST(Class) {
    std::string Id = CodeGen::toIdentifier(Class->getName(), Class->getNameSpace()->getName());
    llvm::SmallVector<llvm::Type *, 4> StructTypes;
    for (auto &Var : Class->getVars()) {
        llvm::Type *FieldType = CGM->GenType(Var.second->getType());
        StructTypes.push_back(FieldType);
    }

    // Create Struct Type
    Type = llvm::StructType::create(CGM->LLVMCtx, StructTypes, Id);

    // Create CodeGen fo Class Vars
    uint32_t Index = 0;
    for (auto &Var : AST->getVars()) {
        // Create ClassVar CodeGen
        CodeGenClassVar *CGCV = new CodeGenClassVar(CGM, Var.second, Type, Index++);
        Var.second->setCodeGen(CGCV);
    }
}

llvm::StructType *CodeGenClass::getType() {
    return Type;
}

void CodeGenClass::InvokeDefaultConstructor(llvm::Value *Instance) {
    for (auto &Var : AST->getVars()) {
        ((CodeGenClassVar *) Var.second->getCodeGen())->setClassInstance(Instance);
    }
}
