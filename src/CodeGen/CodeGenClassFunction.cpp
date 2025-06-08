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
#include <Sema/SemaErrorHandler.h>

using namespace fly;

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, SemaClassMethod *Sema) :
	CodeGenFunctionBase(CGM, Sema), ClassTypePtr(Sema->getClass()->getCodeGen()->getTypePtr()) {

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

    // Add the class type pointer to the params
    ParamTypes.push_back(ClassTypePtr);

    // Generate param types
    GenParamTypes(CGM, ParamTypes, Sema);

    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)
    FnType = llvm::FunctionType::get(RetType, ParamTypes, false);

	std::string FuncName = (Class->getAST()->getName() + Sema->getMangledName()).str();
    std::string Id = CodeGen::toIdentifier(FuncName, Class->getModule()->getNameSpace()->getName());
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Id, CGM->getModule());
}

void CodeGenClassFunction::GenBody() {
    FLY_DEBUG_START("CodeGenFunctionBase", "GenBody");

	SemaClassMethod *ClassMethod = (SemaClassMethod *) Sema;
	SemaClassType *Class = ClassMethod->getClass();

    llvm::Type *ClassType = Class->getCodeGen()->getType();
    setInsertPoint();

	// Alloca Error Handler
    if (Class->getAST()->getClassKind() != ASTClassKind::STRUCT) {
    	AllocaErrorHandler();
    }

    // Alloca Class Instance
    if (!ClassMethod->isStatic()) {
    	InstancePtr = CGM->Builder->CreateAlloca(ClassTypePtr);
    }

    // Alloca Local Vars
    AllocaLocalVars();

	// Only for no Struct Class
    if (Class->getClassKind() == SemaClassKind::CLASS) {
        // Store Error Handler Var
    	Sema->getErrorHandler()->getCodeGen()->StoreErrorHandler(Fn->getArg(0));
    }

    // Instance Class Method (not static)
    if (InstancePtr) {

        //Alloca, Store, Load the second arg which is the instance
        llvm::Argument *ClassInstancePtr = Class->getAST()->getClassKind() == ASTClassKind::STRUCT ? Fn->getArg(0) : Fn->getArg(1);

    	// Set var Index offset in the struct type
    	size_t Index = Class->getAST()->getClassKind() == ASTClassKind::STRUCT ? 0 : 1;

        // Save Class instance and get Pointer
        CGM->Builder->CreateStore(ClassInstancePtr, InstancePtr);
        CGM->Builder->CreateLoad(InstancePtr);

        // All Class Vars
        for (auto &CGAttribute : ClassMethod->getClass()->getCodeGen()->getAttributes()) {

        	CGAttribute->setInstancePtr(InstancePtr);

            // Store attribute default value
            if (ClassMethod->isConstructor()) {
            	llvm::Value *AttrValue;
     //        	if (CGAttribute->getAST()->getExpr())
					// AttrValue = CGM->GenExpr(CGAttribute->getAST()->getExpr());
     //        	else
     //        		AttrValue = CGM->GenValue(CGAttribute->getType(), CGAttribute->getType()->getDefaultValue());
                //CGAttribute->Store(AttrValue);
            }
            Index++;
        }
    }

    // Alloca Function Local Vars and generate body
    StoreParams();
    CGM->GenBlock(this, Sema->getAST()->getBody());

    // Add return Void
    if (FnType->getReturnType()->isVoidTy()) {
        CGM->Builder->CreateRetVoid();
    }
}
