//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClass.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaNameSpace.h"
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassMethod.h>
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CodeGenClass::CodeGenClass(CodeGenModule *CGM, SemaClassType *Sema, bool isExternal) : CGM(CGM), Sema(Sema) {
    std::string TypeName = CodeGen::toIdentifier(Sema->getAST()->getName(), Sema->getModule()->getNameSpace()->getName());

    // Generate Class Type
    Type = llvm::StructType::create(CGM->LLVMCtx, TypeName);
	TypePtr = Type->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());

    // Generate VTable from Class and Interface
    if (Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::INTERFACE) {
        VTableType = llvm::StructType::create(CGM->LLVMCtx, TypeName + "_vtable");
    }

	// Create the Type Vector
	llvm::SmallVector<llvm::Type *, 4> TypeVector;

	// Create CodeGenVar for Class Instance
	CodeGenVar *CGV = new CodeGenVar(CGM, Sema->getThis(), Type);
	Sema->getThis()->setCodeGen(CGV);

	CreateVTableType(TypeVector);
	CreateInheritTypes(TypeVector);
	CreateFieldTypes(TypeVector);

	// %type = type { %vtable_type, %...inherit_types, %...field_types }
	Type->setBody(TypeVector);
}

void CodeGenClass::CreateVTableType(llvm::SmallVector<llvm::Type *, 4> &TypeVector) {
	// Create the VTable Struct Type
	// %vtable_type = type { i32(%Foo*)* }
	llvm::SmallVector<llvm::Type *, 4> VTableVector;

    // Generate Constructors
    if (Sema->getAST()->getClassKind() == ASTClassKind::CLASS ||
    	Sema->getAST()->getClassKind() == ASTClassKind::STRUCT) {

        // Add Constructors
        for (auto &Entry: Sema->getConstructors()) {
			SemaClassMethod * Constructor = Entry.getValue();

            // Create Constructor CodeGen for Constructor
            CodeGenClassMethod *CG = new CodeGenClassMethod(CGM, Constructor, Type, VTableVector.size());
            Constructor->setCodeGen(CG);

        	// Add to Class Constructors
            Constructors.push_back(CG);

        	// Add to VTable Struct Type
        	llvm::PointerType * FnPtrType = CG->getFunctionType()->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
        	VTableVector.push_back(FnPtrType);
        }
    }

    // Set CodeGen Methods
    if (Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::INTERFACE) {

        for (auto &Entry: Sema->getMethods()) {
        	SemaClassMethod *Method = Entry.getValue();

        	CodeGenClassMethod * CG;
        	// CodeGen not yet generated
        	if (Method->getCodeGen() == nullptr) {

        		// Create CodeGen for Method
        		CG = new CodeGenClassMethod(CGM, Method, Type, VTableVector.size());
        		Method->setCodeGen(CG);

        		// Add to Class Methods
        		Methods.push_back(CG);
        	} else {
        		// CodeGen generated from super class
        		CG = Method->getCodeGen();
        	}

        	// Add to VTable
        	if (!Method->isStatic()) {
        		// Add to VTable only instance methods
        		llvm::PointerType * FnPtrType = CG->getFunctionType()->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
        		VTableVector.push_back(FnPtrType);
        	}

        }
        VTableType->setBody(VTableVector);

        // Add VTable as First element
        TypeVector.push_back(VTableType->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace()));
    }
}

void CodeGenClass::CreateInheritTypes(llvm::SmallVector<llvm::Type *, 4> &TypeVector) {
	for (auto &BaseClass : Sema->getBaseClasses()) {
		// SemaClassInstance *BaseThis = BaseEntry.getSecond();

		// Create CodeGen for Base Class
		if (BaseClass->getCodeGen() == nullptr) {
			CodeGenClass * CGC = CGM->GenClass(BaseClass, false);
		}

		// Get the Base Class Type
		llvm::StructType * BaseType = BaseClass->getCodeGen()->getType();

		// Populate Base Types
		BaseTypes.push_back(BaseType);

		// Create CodeGenVar for Base Class Instance
		// llvm::StructType *BaseType = BaseThis->getClass()->getCodeGen()->getType();
		// CodeGenVar *CGV = new CodeGenVar(CGM, BaseThis, BaseType, TypeVector.size());
		// BaseThis->setCodeGen(CGV);

		// Add Inherit Type
		TypeVector.push_back(BaseType);
	}
}

void CodeGenClass::CreateFieldTypes(llvm::SmallVector<llvm::Type *, 4> &TypeVector) {
	// Set CodeGen Attributes
	if (!Sema->getAttributes().empty() &&
		Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::STRUCT) {

		// add var to the type
		for (auto &AttributeEntry: Sema->getAttributes()) {
			SemaClassAttribute *Attribute = AttributeEntry.getValue();
			llvm::Type *AttrType = CGM->GenType(Attribute->getType());

			// Check if the ClassAttribute is a static attribute
			CodeGenVarBase * CGV;
			if (Attribute->isStatic()) {
				llvm::Value *ParentPointer = new llvm::GlobalVariable(*CGM->Module, AttrType, Sema->isConstant(),
					llvm::GlobalValue::ExternalLinkage, nullptr);
				CGV = new CodeGenVar(CGM, Attribute, AttrType);
				CGV->setPointer(ParentPointer);
			} else {
				// Create CodeGenVar for Attribute
				CGV = new CodeGenVar(CGM, Attribute, AttrType, TypeVector.size());
			}
			Attribute->setCodeGen(CGV);

			// Add to Class Var types list
			TypeVector.push_back(AttrType);
		}
	}
}

llvm::StructType *CodeGenClass::getType() {
    return Type;
}

llvm::PointerType *CodeGenClass::getTypePtr() {
    return TypePtr;
}

llvm::StructType *CodeGenClass::getVTableType() {
    return VTableType;
}

llvm::Value * CodeGenClass::NewInstance() {
	return NewInstance(Sema);
}

const SmallVector<CodeGenClassMethod *, 4> &CodeGenClass::getConstructors() const {
    return Constructors;
}

const SmallVector<CodeGenClassMethod *, 4> &CodeGenClass::getMethods() const {
    return Methods;
}

llvm::Value * CodeGenClass::getBaseInstance(llvm::Value *InstancePtr, llvm::StructType *Base) {
	unsigned int I = 1; // Start at 1 because 0 is the vtable pointer
	for (llvm::StructType *BaseType : BaseTypes) {
		if (Base == BaseType) {
			return CGM->Builder->CreateStructGEP(Type, InstancePtr, I);
		}
		I++;
	}

	// Error: Base class type not found
	return nullptr;
}

llvm::Value * CodeGenClass::NewInstance(SemaClassType *ClassType) {
	// Allocate memory for the new instance and all its baseclass instances
	llvm::Type *PtrSizedIntTy = CGM->Module->getDataLayout().getIntPtrType(CGM->LLVMCtx);
	uint64_t ObjectSize = CGM->Module->getDataLayout().getTypeAllocSize(ClassType->getCodeGen()->getType());
	llvm::Constant *ObjectSizeVal = llvm::ConstantInt::get(PtrSizedIntTy, ObjectSize);

	// @malloc data type struct
	return llvm::CallInst::CreateMalloc(CGM->Builder->GetInsertBlock(),
		PtrSizedIntTy, Type, ObjectSizeVal, nullptr, nullptr);
}
