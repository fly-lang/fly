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

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, ASTClassFunction *AST, llvm::PointerType *TypePtr) : CodeGenFunctionBase(CGM, AST) {
    ASTClass *Class = AST->getClass();

    // Generate return type
    GenReturnType();

    // Generate Params Types
    llvm::SmallVector<llvm::Type *, 8> ParamTypes;
    if (Class->getClassKind() != ASTClassKind::STRUCT) { // Add error argument only for class and interface
        ParamTypes.push_back(CGM->ErrorType->getPointerTo(0));
    }
    if (TypePtr) // Instance method
        ParamTypes.push_back(TypePtr);
    GenParamTypes(CGM, ParamTypes, AST->getParams());

    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)
    FnType = llvm::FunctionType::get(RetType, ParamTypes, AST->getParams()->getEllipsis() != nullptr);

    std::string Name = CodeGen::toIdentifier(getAST()->getName(), Class->getNameSpace()->getName(), Class->getName());
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Name, CGM->getModule());
}

void CodeGenClassFunction::GenBody() {
    FLY_DEBUG("CodeGenFunctionBase", "GenBody");
    ASTClass *Class = ((ASTClassFunction *) AST)->getClass();
    Type *ClassType = Class->getCodeGen()->getTypePtr();
    setInsertPoint();

    // the first is the error
    if (Class->getClassKind() != ASTClassKind::STRUCT)
        ErrorVar = Fn->getArg(0);

    // Class Method (not static)
    if (!((ASTClassFunction *) AST)->isStatic()) {

        //Alloca, Store, Load the second arg which is the instance
        llvm::Argument *ClassTypePtr = Class->getClassKind() == ASTClassKind::STRUCT ? Fn->getArg(0) : Fn->getArg(1);

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
                llvm::Value *V = CGM->GenExpr(this, Var->getType(), Var->getExpr());
                CGVar->Store(V);
            }
        }
    }

    // Alloca Function Local Vars and generate body
    AllocaVars();
    StoreParams(false);
    CGM->GenBlock(this, AST->getBody()->getContent());

    // Add return Void
    if (FnType->getReturnType()->isVoidTy()) {
        CGM->Builder->CreateRetVoid();
    }
}
