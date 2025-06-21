//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassMethod.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClassMethod.h"
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

CodeGenClassMethod::CodeGenClassMethod(CodeGenModule *CGM, SemaClassMethod *Sema, size_t Index) :
	CodeGenFunctionBase(CGM, Sema), ClassTypePtr(Sema->getClass()->getCodeGen()->getTypePtr()),
	Index(Index) {

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
	if (!Sema->isStatic()) {
		ParamTypes.push_back(ClassTypePtr);
	}

    // Generate param types
    GenParamTypes(CGM, ParamTypes, Sema);

    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)
    FnType = llvm::FunctionType::get(RetType, ParamTypes, false);

	std::string FuncName = (Class->getAST()->getName() + Sema->getMangledName()).str();
    std::string Id = CodeGen::toIdentifier(FuncName, Class->getModule()->getNameSpace()->getName());
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Id, CGM->getModule());
}

size_t CodeGenClassMethod::getIndex() const {
	return Index;
}

void CodeGenClassMethod::GenBody() {
    FLY_DEBUG_START("CodeGenFunctionBase", "GenBody");

	SemaClassMethod *ClassMethod = (SemaClassMethod *) Sema;
	SemaClassType *Class = ClassMethod->getClass();

    setInsertPoint();

	// Alloca Error Handler
    if (Class->getAST()->getClassKind() != ASTClassKind::STRUCT) {
    	AllocaErrorHandler();
    }

    // Alloca Class Instance
	llvm::AllocaInst * InstancePtr = nullptr;
    if (ClassMethod->isStatic()) {

		// Static Method, add error handler
    	Sema->getErrorHandler()->getCodeGen()->StoreErrorHandler(Fn->getArg(0));
    } else {
    	// FIXME replace with SemaVar - InstancePtr
    	InstancePtr = CGM->Builder->CreateAlloca(ClassTypePtr);
    	llvm::Type *ClassType = Class->getCodeGen()->getType();

    	// CodeGen Class Instance
    	Class->getThis()->setCodeGen(new CodeGenVar(CGM, Class->getThis(), ClassType, InstancePtr));

    	//Alloca, Store, Load the second arg which is the instance
    	llvm::Argument *ClassInstancePtr = nullptr;

    	// Only for no Struct Class
    	if (Class->getClassKind() == SemaClassKind::STRUCT) {
    		ClassInstancePtr = Fn->getArg(0);

    		// Save Class instance and get Pointer
    		Class->getThis()->getCodeGen()->Store(ClassInstancePtr);
    	} else {
    		ClassInstancePtr = Fn->getArg(1);

    		// Store Error Handler Var
    		Sema->getErrorHandler()->getCodeGen()->StoreErrorHandler(Fn->getArg(0));

    		// Save Class instance and get Pointer
    		Class->getThis()->getCodeGen()->Store(ClassInstancePtr);
    	}

    	Class->getThis()->getCodeGen()->Load();

    	// CodeGen Class Attributes
    	for (auto &AttributeEntry: Class->getAttributes()) {
    		SemaClassAttribute *Attribute = AttributeEntry.getValue();
    		Attribute->setCodeGen(new CodeGenVar(CGM, Attribute, ClassType, InstancePtr));

    		// Set Value for all Attributes
    		if (ClassMethod->isConstructor()) {
    			llvm::Value *V = CGM->GenExpr(Attribute->getAST()->getExpr());
    			Attribute->getCodeGen()->Store(V);
    		}
    	}
    }

    // Alloca Local Vars
    AllocaLocalVars();

    // Alloca Function Local Vars and generate body
    StoreParams();
    CGM->GenBlock(this, Sema->getAST()->getBody());

    // Add return Void
    if (FnType->getReturnType()->isVoidTy()) {
        CGM->Builder->CreateRetVoid();
    }
}
