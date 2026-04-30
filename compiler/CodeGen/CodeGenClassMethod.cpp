//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassMethod.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClassMethod.h"

#include "AST/ASTClass.h"
#include "AST/ASTFunction.h"
#include "Basic/Debug.h"
#include "CodeGen/CodeGen.h"
#include "Sema/SemaBlockStmt.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenClassMethod.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenVar.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaNameSpace.h"

#include <AST/ASTType.h>
#include <AST/ASTVar.h>
#include <Sema/SemaError.h>

using namespace fly;

CodeGenClassMethod::CodeGenClassMethod(CodeGenModule *CGM, SemaClassMethod *Sema, llvm::StructType *Type, size_t Index) :
	CodeGenFunctionBase(CGM, Sema), ClassType(Type), Index(Index), Static(Sema->isStatic()) {

	SemaClassType *Class = Sema->getClass();
	//Id = toIdentifier(Sema);

    // Generate return type
	if (Sema->isConstructor()) {
		RetType = CodeGen::VoidTy;
	} else {
		GenReturnType();
		// Validate return type
		if (!RetType) {
			CGM->Diag(diag::err_codegen_invalid_type);
			RetType = CodeGen::VoidTy;
		}
	}

    // Add ErrorHandler to params, Struct doesn't use ErrorHandler
    if (Class->getAST().getClassKind() != ASTClassKind::STRUCT) {
        ParamTypes.push_back(CodeGen::ErrorPtrTy);
    }

    // Add the class type pointer to the params
	if (!Sema->isStatic()) {
		ClassTypePtr = ClassType->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
		ParamTypes.push_back(ClassTypePtr);
	}

    // Generate param types
    GenParamTypes(CGM, ParamTypes, Sema);

    // Validate all types before creating function
    for (auto &Ty : ParamTypes) {
        if (!Ty) {
            CGM->Diag(diag::err_codegen_invalid_type);
            return; // Cannot create function with invalid parameter types
        }
    }

    // Set LLVM Function Name %CLASSNAME_MANGLEDNAME
    FnType = llvm::FunctionType::get(RetType, ParamTypes, false);

	// Build name as ClassName + MangleName so that e.g. TestClass constructor becomes TestClass_F9TestClass
	std::string Name = std::string(Class->getAST().getName()) + Mangle(Sema);
	// Only concrete methods get an LLVM function — interface methods and abstract methods are blueprints only
	SemaClassMethod *ClassMethodSema = static_cast<SemaClassMethod *>(Sema);
	if (Class->getClassKind() != SemaClassKind::INTERFACE && !ClassMethodSema->isAbstract()) {
		Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Name, CGM->getModule());
	}
}

size_t CodeGenClassMethod::getIndex() const {
	return Index;
}

bool CodeGenClassMethod::isStatic() const {
    return Static;
}

void CodeGenClassMethod::GenBody() {
    FLY_DEBUG_START("CodeGenFunctionBase", "GenBody");

	SemaClassMethod *ClassMethod = (SemaClassMethod *) Sema;
	SemaClassType *Class = ClassMethod->getClass();

	if (Class->getClassKind() == SemaClassKind::INTERFACE || ClassMethod->isAbstract()) {
		// Interface methods and abstract methods are blueprints — no body
		return;
	}

    setInsertPoint();

	// Alloca Error Handler
    if (Class->getAST().getClassKind() != ASTClassKind::STRUCT) {
    	Sema->getErrorHandler()->accept(*CGM);
    }

    // Alloca Class Instance
	llvm::AllocaInst * InstancePtr = nullptr;
    if (ClassMethod->isStatic()) {

    	// Alloca local vars and set up param pointers
    	AllocaLocalVars();

		// Static Method, store error handler
    	Sema->getErrorHandler()->getCodeGen()->StoreErrorHandler(Fn->getArg(0));

    	// Store params starting at index 1 (after error handler)
    	StoreParams(1);

    } else {

    	// Alloca Class Instance Pointer
    	InstancePtr = CGM->Builder->CreateAlloca(ClassTypePtr);

    	// Set CodeGen Class Instance Pointer
    	ClassMethod->getThis()->getCodeGen()->setPointer(InstancePtr);

    	//Alloca, Store, Load the second arg which is the instance
    	llvm::Argument *ClassInstancePtr = nullptr;

    	// Only for no Struct Class
    	if (Class->getClassKind() == SemaClassKind::STRUCT) {
    		// 0 is the class instance
    		// 1..n are the function params
    		unsigned const int InstanceArgIdx = 0;
    		unsigned const int StartArgIdx = 1;

    		ClassInstancePtr = Fn->getArg(InstanceArgIdx);

    		// Alloca Params
    		// Alloca Local Vars & Params
    		AllocaLocalVars();

    		// Save Class instance and get Pointer
    		ClassMethod->getThis()->getCodeGen()->Store(ClassInstancePtr);

    		// Alloca Function Local Vars and generate body
    		StoreParams(StartArgIdx);
    	} else {

    		// 0 is Error Handler
    		// 1 is the class instance
    		// 2..n are the function params
    		unsigned const int ErrorHandlerArgIdx = 0;
    		unsigned const int InstanceArgIdx = 1;
    		unsigned const int StartArgIdx = 2;

    		// Get the Class Instance Pointer: 0 is Error Handler, 1 is the instance
    		ClassInstancePtr = Fn->getArg(InstanceArgIdx);

    		// Alloca Params
    		// Alloca Local Vars & Params
    		AllocaLocalVars();

    		// Store Error Handler Var
    		Sema->getErrorHandler()->getCodeGen()->StoreErrorHandler(Fn->getArg(ErrorHandlerArgIdx));

    		// Save Class instance and get Pointer
    		ClassMethod->getThis()->getCodeGen()->Store(ClassInstancePtr);

    		//Write the vtable in constructor
    		// if (ClassMethod->isConstructor()) {
    		// 	CodeGenVarBase * CGV = ClassMethod->getThis()->getCodeGen();
    		// 	llvm::Value * VTablePtr = CGM->Builder->CreateInBoundsGEP(CGV->getType(), CGV->getValue(),  {CGM->Zero, CGM->Zero});
    		// 	CGM->Builder->CreateStore(Class->getCodeGen()->getVTable(), VTablePtr);
    		// }

    		// Alloca Function Local Vars and generate body
    		StoreParams(StartArgIdx);
    	}

    	// Load the instance: load only if used
    	// ClassMethod->getThis()->getCodeGen()->Load();

    	// CodeGen Class Attributes
    	for (auto &AttributeEntry: Class->getAttributes()) {
    		SemaClassAttribute *Attribute = AttributeEntry.getValue();

    		// Static attributes are backed by a GlobalVariable; their pointer
    		// must not be overwritten with the instance pointer.
    		if (Attribute->isStatic()) continue;

    		// Set Pointer for Class Attribute
    		Attribute->getCodeGen()->setPointer(InstancePtr);

    		// Set Value for all Attributes
    		// Already DONE in init_ctor()
    		// if (ClassMethod->isConstructor()) {
    		// 	llvm::Value *V = CGM->GenExpr(Attribute->getAST().getExpr());
    		// 	Attribute->getCodeGen()->Store(V);
    		// }
    	}
    }

	if (Sema->getBody()) {
		Sema->getBody()->accept(*CGM);
	}

	// Add return Void
	CheckReturnVoid();
}

std::string CodeGenClassMethod::toIdentifier(SemaClassMethod *ClassMethod) {
	FLY_DEBUG_START("CodeGenClassMethod", "toIdentifier");
	// For class methods, use the mangled name which includes the class name
	SemaNameSpace *NameSpace = ClassMethod->getClass()->getModule().getNameSpace();
	return CGM->toIdentifier(ClassMethod->getAST().getName(), NameSpace);
}

