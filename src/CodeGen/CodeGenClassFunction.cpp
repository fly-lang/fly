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
#include "CodeGen/CodeGenVar.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaClassType.h"
#include "AST/ASTClass.h"
#include "AST/ASTFunction.h"
#include "Basic/Debug.h"
#include "CodeGen/CodeGenVar.h"

#include <AST/ASTTypeRef.h>
#include <AST/ASTVar.h>

using namespace fly;

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, SemaClassMethod *Sema, llvm::PointerType *TypePtr) :
	CodeGenFunctionBase(CGM, Sema) {

	SemaClassType *Class = Sema->getClass();

    // Generate return type
	if (Sema->isConstructor()) {
		RetType = CGM->VoidTy;
	} else {
		GenReturnType();
	}

    // Add ErrorHandler to params, Struct doesn't use ErrorHandler
    if (Class->getAST()->getClassKind() != ASTClassKind::STRUCT) {
        ParamTypes.push_back(CGM->ErrorPtrTy);
    }

    // Add the instance var of the class type to the parameters of the function
    if (TypePtr)
        ParamTypes.push_back(TypePtr);

    // Generate param types
    GenParamTypes(CGM, ParamTypes, Sema);

    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)
    FnType = llvm::FunctionType::get(RetType, ParamTypes, false);

    std::string Id = CodeGen::toIdentifier(Sema->getAST()->getName(), Class->getModule()->getNameSpace()->getName(), Class->getAST()->getName());
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Id, CGM->getModule());
}

void CodeGenClassFunction::GenBody() {
	SemaClassMethod *ClassMethod = (SemaClassMethod *) Sema;
	SemaClassType *Class = ClassMethod->getClass();

    FLY_DEBUG_START("CodeGenFunctionBase", "GenBody");
    llvm::Type *ClassType = Class->getCodeGen()->getType();
    setInsertPoint();

    // the first argument is the error handler
    if (Class->getAST()->getClassKind() != ASTClassKind::STRUCT) {
        ErrorHandler = Fn->getArg(0);

        // Alloca Error Handler Var
        AllocaErrorHandler();
    }

    // Alloca Method Class Instance
    CodeGenVar *CGI = nullptr;
    if (!ClassMethod->isStatic()) {
        CGI = new CodeGenVar(CGM, ClassType);
        CGI->Alloca();
    }

    // Alloca Local Vars
    AllocaLocalVars();

    if (Class->getAST()->getClassKind() != ASTClassKind::STRUCT) {
        // Store Error Handler Var
        StoreErrorHandler(false);
    }

    // Instance Class Method (not static)
    if (CGI) {

        //Alloca, Store, Load the second arg which is the instance
        llvm::Argument *ClassInstancePtr = Class->getAST()->getClassKind() == ASTClassKind::STRUCT ? Fn->getArg(0) : Fn->getArg(1);

        // Save Class instance and get Pointer
        CGI->Store(ClassInstancePtr);
        CGI->Load();

        // Set var Index offset in the struct type
        size_t Index = Class->getAST()->getClassKind() == ASTClassKind::STRUCT ? 0 : 1;
        // All Class Vars
        for (auto &AttributeEntry : ClassMethod->getClass()->getAttributes()) {
        	SemaClassAttribute *Attribute = AttributeEntry.getValue();

            // Set CodeGen Class Instance
            llvm::Type *Ty = CGM->GenType(Attribute->getAST()->getTypeRef()->getSema());
            CodeGenClassVar *CGV = new CodeGenClassVar(CGM, Ty, CGI, Index);
            Attribute->setCodeGen(CGV);

            // Store attribute default value
            if (ClassMethod->isConstructor()) {
            	llvm::Value *AttrValue;
            	if (Attribute->getAST()->getExpr())
					AttrValue = CGM->GenExpr(Attribute->getAST()->getExpr());
            	else
            		AttrValue = CGM->GenValue(Attribute->getType(), Attribute->getType()->getDefaultValue());
                CGV->Store(AttrValue);
            }
            Index++;
        }
    }

    // Alloca Function Local Vars and generate body
    StoreParams(false);
    CGM->GenBlock(this, Sema->getAST()->getBody());

    // Add return Void
    if (FnType->getReturnType()->isVoidTy()) {
        CGM->Builder->CreateRetVoid();
    }
}
