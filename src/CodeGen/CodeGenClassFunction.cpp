//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassFunction.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClassFunction.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClass.h"

using namespace fly;

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, ASTClassFunction *AST) : CodeGenFunctionBase(CGM, AST) {

}

Function *CodeGenClassFunction::Create() {
    ASTClassFunction *ClassFunction = ((ASTClassFunction *) getAST());
    ASTClass *Class = ClassFunction->getClass();

    if (ClassFunction->isConstructor()) {
        FnTy = GenFuncType(CGM->VoidTy, ClassFunction->getParams());
    } else {
        FnTy = GenFuncType(ClassFunction->getType(), ClassFunction->getParams());
    }
    Fn = llvm::Function::Create(FnTy, llvm::GlobalValue::ExternalLinkage, "", CGM->getModule());

    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)

    std::string Id = CodeGen::toIdentifier(getAST()->getName(), Class->getNameSpace()->getName(), Class->getName());
    Fn->setName(Id);

    setInsertPoint();
    
    // if is Constructor Add return a pointer to memory struct
    if (ClassFunction->isConstructor()) {
        llvm::StructType *T = Class->getCodeGen()->getType();
        llvm::Type *Param = T->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
        AllocaInst *I = CGM->Builder->CreateAlloca(Param);
        CGM->Builder->CreateStore(Fn->getArg(0), I);
        LoadInst *Load = CGM->Builder->CreateLoad(I);

        // TODO Save all default var values

        AllocaVars();
        CGM->GenBlock(Fn, ClassFunction->getBody()->getContent());
        CGM->Builder->CreateRetVoid();
    } else {
        AllocaVars();
        CGM->GenBlock(Fn, ClassFunction->getBody()->getContent());
    }

    return Fn;
}
