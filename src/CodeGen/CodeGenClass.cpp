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

	CreateVTableType(TypeVector);
	CreateInheritTypes(TypeVector);
	CreateFieldTypes(TypeVector);
	CreateSharedPtrType(TypeVector);

	// %type = type { %vtable_type, %...inherit_types, %...field_types, %shared_ptr_type }
	Type->setBody(TypeVector);
}

void CodeGenClass::CreateVTableType(llvm::SmallVector<llvm::Type *, 4> &TypeVector) {
	llvm::SmallVector<llvm::Type *, 4> VTableVector;

    // Generate Constructors
    if (Sema->getAST()->getClassKind() == ASTClassKind::CLASS ||
    	Sema->getAST()->getClassKind() == ASTClassKind::STRUCT) {

        // Add Constructors
        for (auto &Entry: Sema->getConstructors()) {
			SemaClassMethod * Constructor = Entry.getValue();

            // Create Constructor CodeGen for Constructor
            CodeGenClassMethod *CG = new CodeGenClassMethod(CGM, Constructor, TypePtr, VTableVector.size());
            Constructor->setCodeGen(CG);
            Constructors.push_back(CG);
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
        		CG = new CodeGenClassMethod(CGM, Method, TypePtr, VTableVector.size());
        		Method->setCodeGen(CG);

        		// Add to Class Methods
        		Methods.push_back(CG);
        	} else {
        		// CodeGen generated from super class
        		CG = Method->getCodeGen();
        	}

        	// Add to VTable
        	if (!Method->isStatic()) {
        		// only instance methods
        		// Create the VTable Struct Type
        		// %vtable_type = type { i32(%Foo*)* }
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

}

void CodeGenClass::CreateFieldTypes(llvm::SmallVector<llvm::Type *, 4> &TypeVector) {
	// Set CodeGen Attributes
	if (!Sema->getAttributes().empty() &&
		Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::STRUCT) {

		// add var to the type
		for (auto &AttributeEntry: Sema->getAttributes()) {
			SemaClassAttribute *Attribute = AttributeEntry.getValue();
			llvm::Type *AttrType = CGM->GenType(Attribute->getType());

			// Se un attributo è pubblico aggiungilo nella shared ptr
			// Altrimenti aggiungilo nella Struct

			// Add to Class Var types list
			TypeVector.push_back(AttrType);
		}
	}
}

void CodeGenClass::CreateSharedPtrType(llvm::SmallVector<llvm::Type *, 4> &TypeVector) {
	// Se non esistono sotto classi, allora allora puoi creare shared_ptr
	// Altrimenti prendi shared_ptr della sotto	classe
	// aggiungi gli attributi pubblici senza duplicati
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

const SmallVector<CodeGenClassMethod *, 4> &CodeGenClass::getConstructors() const {
    return Constructors;
}

const SmallVector<CodeGenClassMethod *, 4> &CodeGenClass::getMethods() const {
    return Methods;
}
