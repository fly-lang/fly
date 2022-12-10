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
        CodeGenClassVar *CG = new CodeGenClassVar(CGM, Var.second, Type, Index++);
        Var.second->setCodeGen(CG);
        Vars.push_back(CG);
    }

    for (auto &Map : AST->getMethods()) {
        for (auto Vect : Map.second) {
            for (auto ClassFunction : Vect.second) {
                // Create ClassFunction CodeGen
                CodeGenClassFunction *CG = new CodeGenClassFunction(CGM, ClassFunction);
                ClassFunction->setCodeGen(CG);
                Functions.push_back(CG);
            }
        }
    }
}

llvm::StructType *CodeGenClass::getType() {
    return Type;
}

const SmallVector<CodeGenClassVar *, 4> &CodeGenClass::getVars() const {
    return Vars;
}

const SmallVector<CodeGenClassFunction *, 4> &CodeGenClass::getFunctions() const {
    return Functions;
}

void CodeGenClass::InvokeDefaultConstructor(llvm::Value *Instance) {
    for (auto &Var : AST->getVars()) {
        ((CodeGenClassVar *) Var.second->getCodeGen())->setClassInstance(Instance);
    }
}
