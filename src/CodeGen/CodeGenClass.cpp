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
    llvm::SmallVector<llvm::Type *, 4> StructTypes;
    for (auto &Var : Class->getVars()) {
        llvm::Type *FieldType = CGM->GenType(Var.second->getType());
        StructTypes.push_back(FieldType);
    }
    // TODO if Type == Class->Type cannot be resolved from GenType()
    std::string Name = CodeGen::toIdentifier(Class->getName(), Class->getNameSpace()->getName());
    Type = llvm::StructType::create(CGM->LLVMCtx, StructTypes, Name);
}

void CodeGenClass::Generate() {

    // Set CodeGen ClassVar
    uint32_t n = 0;
    for (auto &Var : AST->getVars()) {
        CodeGenClassVar *CGV = new CodeGenClassVar(CGM, Var.second, Type, n++);
        Vars.push_back(CGV);
        Var.second->setCodeGen(CGV);
    }

    // Create Constructors
    for (auto &Vect : AST->getConstructors()) {
        for (auto ClassFunction : Vect.second) {
            // Create ClassFunction CodeGen for Constructor
            CodeGenClassFunction *CGCF = new CodeGenClassFunction(CGM, ClassFunction);
            CGCF->Create();
            ClassFunction->setCodeGen(CGCF);
            Constructors.push_back(CGCF);
        }
    }

    // Create Functions
    for (auto &Map : AST->getMethods()) {
        for (auto &Vect : Map.second) {
            for (auto ClassFunction : Vect.second) {
                // Create ClassFunction CodeGen
                CodeGenClassFunction *CGCF = new CodeGenClassFunction(CGM, ClassFunction);
                CGCF->Create();
                ClassFunction->setCodeGen(CGCF);
                Functions.push_back(CGCF);
            }
        }
    }
}

llvm::StructType *CodeGenClass::getType() {
    return Type;
}

llvm::PointerType *CodeGenClass::getTypePtr() {
    return Type->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
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
