//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassFunction.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClassFunction.h"
#include "CodeGen/CodeGenClassVar.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClass.h"

using namespace fly;

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, ASTClassFunction *AST) : CodeGenFunctionBase(CGM, AST) {

}

Function *CodeGenClassFunction::Create() {
    ASTClassFunction *ClassFunction = ((ASTClassFunction *) getAST());
    ASTClass *Class = ClassFunction->getClass();

    if (ClassFunction->isConstructor()) { // only for constructors
        FnTy = GenFuncType(CGM->VoidTy, ClassFunction->getParams());
    } else {
        FnTy = GenFuncType(ClassFunction->getType(), ClassFunction->getParams());
    }
    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)
    std::string Name = CodeGen::toIdentifier(getAST()->getName(), Class->getNameSpace()->getName(), Class->getName());
    Fn = llvm::Function::Create(FnTy, llvm::GlobalValue::ExternalLinkage, Name, CGM->getModule());

    setInsertPoint();
    
    // if is Constructor Add return a pointer to memory struct
    if (!ClassFunction->isStatic()) {

        //Alloca, Store, Load first arg which is the instance
        Argument *ClassTypePtr = Fn->getArg(0);
        Type *ClassType = ClassTypePtr->getType();
        AllocaInst *Instance = CGM->Builder->CreateAlloca(ClassType);
        CGM->Builder->CreateStore(ClassTypePtr, Instance);
        LoadInst *Load = CGM->Builder->CreateLoad(Instance);

        // All Class Vars
        for (auto &Entry : Class->getVars()) {
            ASTClassVar *Var = Entry.second;

            // Set CodeGen Class Instance
            CodeGenClassVar *CGVar = Var->getCodeGen();
            CGVar->Init(Load);

            // Save all default var values
            if (ClassFunction->isConstructor()) {
                llvm::Value *V = CGM->GenExpr(Fn, Var->getType(), Var->getExpr());
                CGVar->Store(V);
            }
        }

        AllocaVars();
        CGM->GenBlock(Fn, ClassFunction->getBody()->getContent());

        if (ClassFunction->isConstructor()) {
            CGM->Builder->CreateRetVoid();
        }
    } else {
        AllocaVars();
        CGM->GenBlock(Fn, ClassFunction->getBody()->getContent());
    }

    return Fn;
}
