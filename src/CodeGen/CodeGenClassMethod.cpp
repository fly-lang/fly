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

CodeGenClassMethod::CodeGenClassMethod(CodeGenModule *CGM, SemaClassMethod *Sema, llvm::StructType *Type, size_t Index) :
	CodeGenFunctionBase(CGM, Sema), ClassType(Type), Index(Index) {

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
		ClassTypePtr = ClassType->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
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
    if (ClassMethod->getClass()->getAST()->getClassKind() != ASTClassKind::STRUCT) {
    	AllocaErrorHandler();
    }

    // Alloca Class Instance
	llvm::AllocaInst * InstancePtr = nullptr;
    if (ClassMethod->isStatic()) {

		// Static Method, add error handler
    	Sema->getErrorHandler()->getCodeGen()->StoreErrorHandler(Fn->getArg(0));

    } else {

    	// Alloca Class Instance Pointer
    	InstancePtr = CGM->Builder->CreateAlloca(ClassTypePtr);

    	// Set CodeGen Class Instance Pointer
    	ClassMethod->getThis()->getCodeGen()->setPointer(InstancePtr);

    	//Alloca, Store, Load the second arg which is the instance
    	llvm::Argument *ClassInstancePtr = nullptr;

    	// Only for no Struct Class
    	if (Class->getClassKind() == SemaClassKind::STRUCT) {
    		ClassInstancePtr = Fn->getArg(0);

    		// Alloca Params
    		// Alloca Local Vars & Params
    		AllocaLocalVars();

    		// Save Class instance and get Pointer
    		ClassMethod->getThis()->getCodeGen()->Store(ClassInstancePtr);

    		// Alloca Function Local Vars and generate body
    		StoreParams(1);
    	} else {

    		// Get the Class Instance Pointer
    		ClassInstancePtr = Fn->getArg(1);

    		// Alloca Params
    		// Alloca Local Vars & Params
    		AllocaLocalVars();

    		// Store Error Handler Var
    		Sema->getErrorHandler()->getCodeGen()->StoreErrorHandler(Fn->getArg(0));

    		// Save Class instance and get Pointer
    		ClassMethod->getThis()->getCodeGen()->Store(ClassInstancePtr);

    		// Alloca Function Local Vars and generate body
    		StoreParams(2);
    	}

    	ClassMethod->getThis()->getCodeGen()->Load();

    	// CodeGen Class Attributes
    	for (auto &AttributeEntry: Class->getAttributes()) {
    		SemaClassAttribute *Attribute = AttributeEntry.getValue();

    		// Set Pointer for Class Attribute
    		Attribute->getCodeGen()->setPointer(InstancePtr);

    		// Set Value for all Attributes
    		if (ClassMethod->isConstructor()) {
    			llvm::Value *V = CGM->GenExpr(Attribute->getAST()->getExpr());
    			Attribute->getCodeGen()->Store(V);
    		}
    	}
    }

    CGM->GenBlock(this, Sema->getAST()->getBody());

    // Add return Void
	CheckReturnVoid();
}
