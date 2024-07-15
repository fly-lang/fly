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
    if (AST->getClass()->getClassKind() != ASTClassKind::STRUCT) {
        ParamTypes.push_back(CGM->ErrorPtrTy);
    }

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
    llvm::Type *ClassType = Class->getCodeGen()->getType();
    setInsertPoint();

    // the first argument is the error handler
    if (Class->getClassKind() != ASTClassKind::STRUCT) {
        ErrorHandler = Fn->getArg(0);

        // Alloca Error Handler Var
        AllocaErrorHandler();
    }

    // Alloca Method Class Instance
    CodeGenVar *CGI = nullptr;
    if (!((ASTClassMethod *) AST)->isStatic()) {
        CGI = new CodeGenVar(CGM, ClassType);
        CGI->Alloca();
    }

    // Alloca Local Vars
    AllocaLocalVars();

    if (Class->getClassKind() != ASTClassKind::STRUCT) {
        // Store Error Handler Var
        StoreErrorHandler(false);
    }

    // Instance Class Method (not static)
    if (CGI) {

        //Alloca, Store, Load the second arg which is the instance
        llvm::Argument *ClassInstancePtr = Class->getClassKind() == ASTClassKind::STRUCT ? Fn->getArg(0) : Fn->getArg(1);

        // Save Class instance and get Pointer
        CGI->Store(ClassInstancePtr);
        CGI->Load();

        // Set var Index offset in the struct type
        uint32_t Index = Class->getClassKind() == ASTClassKind::STRUCT ? 0 : 1;
        // All Class Vars
        ASTClassMethod *ClassMethod = (ASTClassMethod *) AST;
        for (auto &Attribute : ClassMethod->getClass()->getAttributes()) {

            // Set CodeGen Class Instance
            llvm::Type *Ty = CGM->GenType(Attribute->getType());
            CodeGenVar *CGV = new CodeGenVar(CGM, Ty, CGI, Index); // FIXME replace con CodeGenClassVar
            Attribute->setCodeGen(CGV);

            // Store attribute default value
            if (ClassMethod->isConstructor()) {
                Value *AttrValue = CGM->GenExpr(Attribute->getExpr());
                CGV->Store(AttrValue);
            }
            Index++;
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
