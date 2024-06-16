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
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
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
    AllocaInst *ClassInstance = nullptr;
    if (!((ASTClassMethod *) AST)->isStatic()) {
        ClassInstance = CGM->Builder->CreateAlloca(ClassType);
    }
    AllocaLocalVars();

    StoreErrorHandler(false);

    // Instance Class Method (not static)
    if (ClassInstance) {

        //Alloca, Store, Load the second arg which is the instance
        llvm::Argument *ClassTypePtr = Class->getClassKind() == ASTClassKind::STRUCT ? Fn->getArg(0) : Fn->getArg(1);

        // Save Class instance and get Pointer
        CGM->Builder->CreateStore(ClassTypePtr, ClassInstance);
        llvm::LoadInst *LoadClassInstance = CGM->Builder->CreateLoad(ClassInstance);

        // All Class Vars
        ASTClassMethod *ClassMethod = (ASTClassMethod *) AST;
        for (auto &AttrEntry : ClassMethod->getClass()->getAttributes()) {
            ASTClassAttribute *Attr = AttrEntry.second;
            // Set CodeGen Class Instance
            CodeGenClassVar *CGCV = (CodeGenClassVar *) Attr->getCodeGen();
            CGCV->setInstance(LoadClassInstance);
            CGCV->Init();
            if (ClassMethod->isConstructor())
                CGCV->Store(CGM->GenExpr(Attr->getExpr()));
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
