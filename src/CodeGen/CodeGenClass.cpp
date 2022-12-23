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

    // Create Struct Type
    uint32_t n = 0;
    llvm::SmallVector<llvm::Type *, 4> StructTypes;
    for (auto &Var : Class->getVars()) {
        llvm::Type *FieldType = CGM->GenType(Var.second->getType());
        StructTypes.push_back(FieldType);
        CodeGenClassVar *CGV = new CodeGenClassVar(CGM, Var.second, Type, n++);
        Var.second->setCodeGen(CGV);
    }
    // TODO if Type == Class->Type cannot be resolved from GenType()
    std::string Id = CodeGen::toIdentifier(Class->getName(), Class->getNameSpace()->getName());
    Type = llvm::StructType::create(CGM->LLVMCtx, StructTypes, Id);
}

void CodeGenClass::Generate() {

    // Create Constructors
    for (auto &Vect : AST->getConstructors()) {
        for (auto ClassFunction : Vect.second) {
            // Create ClassFunction CodeGen for Constructor
            CodeGenClassFunction *CG = new CodeGenClassFunction(CGM, ClassFunction);
            CG->PreParams.push_back(Type);
            CG->Create();
            ClassFunction->setCodeGen(CG);
            Constructors.push_back(CG);
        }
    }

    // Create Functions
    for (auto &Map : AST->getMethods()) {
        for (auto &Vect : Map.second) {
            for (auto ClassFunction : Vect.second) {
                // Create ClassFunction CodeGen
                CodeGenClassFunction *CG = new CodeGenClassFunction(CGM, ClassFunction);
                CG->PreParams.push_back(Type);
                CG->Create();
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

const SmallVector<CodeGenClassFunction *, 4> &CodeGenClass::getConstructors() const {
    return Constructors;
}

const SmallVector<CodeGenClassFunction *, 4> &CodeGenClass::getFunctions() const {
    return Functions;
}
