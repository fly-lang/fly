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
#include "CodeGen/CodeGenError.h"
#include "AST/ASTModule.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClass.h"
#include "Basic/Debug.h"

using namespace fly;

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, ASTClassMethod *AST, llvm::PointerType *TypePtr) : CodeGenFunctionBase(CGM, AST) {
    ASTClass *Class = AST->getClass();

    // Generate return type
    GenReturnType();

    // Add ErrorHandler to params
    CodeGenError *CGE = (CodeGenError *) AST->getErrorHandler()->getCodeGen();
    ParamTypes.push_back(CGE->getType());

    // Add the instance var of the class type to the parameters of the function
    if (TypePtr)
        ParamTypes.push_back(TypePtr);

    // Generate param types
    GenParamTypes(CGM, ParamTypes, AST->getParams());

    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)
    FnType = llvm::FunctionType::get(RetType, ParamTypes, AST->getEllipsis() != nullptr);

    std::string Id = CodeGen::toIdentifier(AST->getName(), Class->getNameSpace()->getName(), Class->getName());
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Id, CGM->getModule());
}

void CodeGenClassFunction::GenBody() {
    FLY_DEBUG("CodeGenFunctionBase", "GenBody");
    ASTClass *Class = ((ASTClassMethod *) AST)->getClass();
    Type *ClassType = Class->getCodeGen()->getTypePtr();
    setInsertPoint();

    // the first argument is the error
    if (Class->getClassKind() != ASTClassKind::STRUCT)
        ErrorHandler = Fn->getArg(0);

    // Alloca Vars
    AllocaErrorHandler();
    AllocaInst *Instance = nullptr;
    if (!((ASTClassMethod *) AST)->isStatic()) {
        Instance = CGM->Builder->CreateAlloca(ClassType);
    }
    AllocaVars();

    StoreErrorHandler(false);

    // Class Method (not static)
    if (Instance) {

        //Alloca, Store, Load the second arg which is the instance
        llvm::Argument *ClassTypePtr = Class->getClassKind() == ASTClassKind::STRUCT ? Fn->getArg(0) : Fn->getArg(1);

        // Save Class instance and get Pointer
        CGM->Builder->CreateStore(ClassTypePtr, Instance);
        llvm::LoadInst *LoadInstance = CGM->Builder->CreateLoad(Instance);

        // All Class Vars
        for (auto &Entry : Class->getAttributes()) {
            ASTClassAttribute *Var = Entry.second;

            // Set CodeGen Class Instance
            CodeGenClassVar *CGVar = (CodeGenClassVar *) Var->getCodeGen();
            CGVar->setInstance(LoadInstance);
            CGVar->Init();

            // Save all default var values
            if (((ASTClassMethod *) AST)->isConstructor()) {

                // TODO execute PreConstructor
//                llvm::Value *V = CGM->GenExpr(this, Var->getType(), Var->getExpr());
//                CGVar->Store(V);
            }
        }
    }

    // Alloca Function Local Vars and generate body
    StoreParams(false);
    CGM->GenBlock(this, AST->getBody());

    // Add return Void
    if (FnType->getReturnType()->isVoidTy()) {
        CGM->Builder->CreateRetVoid();
    }
}
