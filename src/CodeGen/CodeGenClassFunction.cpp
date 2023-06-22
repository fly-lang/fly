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
#include "Basic/Debug.h"

using namespace fly;

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, ASTClassFunction *AST) : CodeGenFunctionBase(CGM, AST) {

}

Function *CodeGenClassFunction::Create() {
    ASTClassFunction *ClassFunction = ((ASTClassFunction *) getAST());
    ASTClass *Class = ClassFunction->getClass();
    PointerType *ClassType = Class->getCodeGen()->getTypePtr();

    llvm::Type *RetType;
    llvm::SmallVector<llvm::Type *, 8> ParamTypes;
    if (ClassFunction->isConstructor()) { // only for constructors
        RetType = CGM->VoidTy;
        ParamTypes.push_back(ClassType);
    } else if (ClassFunction->isStatic()) {
        RetType = CGM->GenType(ClassFunction->getType());
    } else {
        RetType = CGM->GenType(ClassFunction->getType());
        ParamTypes.push_back(ClassType);
    }
    GenTypes(ParamTypes, ClassFunction->getParams());

    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)
    std::string Name = CodeGen::toIdentifier(getAST()->getName(), Class->getNameSpace()->getName(), Class->getName());
    FnTy = llvm::FunctionType::get(RetType, ParamTypes, ClassFunction->getParams()->getEllipsis() != nullptr);
    Fn = llvm::Function::Create(FnTy, llvm::GlobalValue::ExternalLinkage, Name, CGM->getModule());

    return Fn;
}

void CodeGenClassFunction::GenBody() {
    FLY_DEBUG("CodeGenFunctionBase", "GenBody");
    ASTClass *Class = ((ASTClassFunction *) AST)->getClass();
    PointerType *ClassType = Class->getCodeGen()->getTypePtr();
    setInsertPoint();

    // if is Constructor Add return a pointer to memory struct
    if (!((ASTClassFunction *) AST)->isStatic()) {

        //Alloca, Store, Load first arg which is the instance
        llvm::Argument *ClassTypePtr = Fn->getArg(0);
        AllocaInst *Instance = CGM->Builder->CreateAlloca(ClassType);
        CGM->Builder->CreateStore(ClassTypePtr, Instance);
        llvm::LoadInst *Load = CGM->Builder->CreateLoad(Instance);

        // All Class Vars
        for (auto &Entry : Class->getVars()) {
            ASTClassVar *Var = Entry.second;

            // Set CodeGen Class Instance
            CodeGenClassVar *CGVar = (CodeGenClassVar *) Var->getCodeGen();
            CGVar->setInstance(Load);
            CGVar->Init();

            // Save all default var values
            if (((ASTClassFunction *) AST)->isConstructor()) {
                llvm::Value *V = CGM->GenExpr(Fn, Var->getType(), Var->getExpr());
                CGVar->Store(V);
            }
        }
    }

    AllocaVars();
    CGM->GenBlock(Fn, AST->getBody()->getContent());

    // Add return Void
    BasicBlock &BB = *Fn->getBasicBlockList().end();
    Instruction &I = *BB.end();
    if (FnTy->getReturnType()->isVoidTy()) {
        CGM->Builder->CreateRetVoid();
    }
}
