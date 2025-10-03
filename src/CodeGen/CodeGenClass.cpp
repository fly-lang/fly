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

#include <AST/ASTVar.h>
#include <llvm/Option/Arg.h>

using namespace fly;

CodeGenClass::CodeGenClass(CodeGenModule *CGM, SemaClassType *Sema, bool isExternal) : CGM(CGM), Sema(Sema) {
	std::string TypeName = CodeGen::toIdentifier(
		Sema->getAST()->getName(), Sema->getModule()->getNameSpace()->getName());

	// Generate Class Type
	Type = llvm::StructType::create(CGM->LLVMCtx, TypeName);
	TypePtr = Type->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());

	// Generate VTable from Class and Interface
	if (Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::INTERFACE) {
		VTableType = llvm::StructType::create(CGM->LLVMCtx, TypeName + "_vtable");
	}

	// Create CodeGenVar for Class Instance
	CodeGenVar *CGV = new CodeGenVar(CGM, Sema->getThis(), Type);
	Sema->getThis()->setCodeGen(CGV);

	// Create the Init Constructor Function
	CreateInitConstructor();

	CreateVTableType();
	CreateBaseTypes();
	CreateAttributeTypes();
	CreateVTable();

	// %type = type { %vtable_type, %...inherit_types, %...field_types }
	Type->setBody(BodyTypes);

	// Generate Init Constructor Body
	GenInitConstructorBody();
}

void CodeGenClass::CreateVTableType() {
	// Create the VTable Struct Type
	// %vtable_type = type { i32, void (%ClassType*)*, i32(%Foo*)* }

	// offset-to-top is the first field of vtable
	if (Sema->getAST()->getClassKind() == ASTClassKind::CLASS) {
		llvm::Type *OffsetType = CGM->Int32Ty;
		VTableMethodTypes.push_back(OffsetType);
	}

	// Generate Constructors only for Class
	// Struct uses only the Init Constructor
    // Interface doesn't have constructors
	if (Sema->getAST()->getClassKind() == ASTClassKind::CLASS) {

		// Add Init Constructor Fn Type
		// void (%ClassType*)*
		// llvm::PointerType *InitCtorPtrType = InitConstructor->getFunctionType()->getPointerTo(
		// 	CGM->Module->getDataLayout().getAllocaAddrSpace());
		// VTableMethodTypes.push_back(InitCtorPtrType);

		// Add Constructors
		for (auto &Entry : Sema->getConstructors()) {
			SemaClassMethod *Constructor = Entry.getValue();

			// Create Constructor CodeGen for Constructor
			CodeGenClassMethod *CG = new CodeGenClassMethod(CGM, Constructor, Type, VTableMethodTypes.size());
			Constructor->setCodeGen(CG);

			// Add to Class Constructors
			Methods.push_back(CG);

			// Add to VTable Struct Type
			llvm::PointerType *FnPtrType = CG->getFunctionType()->getPointerTo(
				CGM->Module->getDataLayout().getAllocaAddrSpace());
			VTableMethodTypes.push_back(FnPtrType);
		}
	}

	// Set CodeGen Methods
	if (Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::INTERFACE) {

		for (auto &Entry : Sema->getMethods()) {
			SemaClassMethod *Method = Entry.getValue();

			CodeGenClassMethod *CG;
			// CodeGen not yet generated
			if (Method->getCodeGen() == nullptr) {

				// Create CodeGen for Method
				CG = new CodeGenClassMethod(CGM, Method, Type, VTableMethodTypes.size());
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
				llvm::PointerType *FnPtrType = CG->getFunctionType()->getPointerTo(
					CGM->Module->getDataLayout().getAllocaAddrSpace());
				VTableMethodTypes.push_back(FnPtrType);
			}

		}

		// Set VTable Body
		VTableType->setBody(VTableMethodTypes);

		// Add VTable as First element
		BodyTypes.push_back(VTableType->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace()));
	}
}

void CodeGenClass::CollectBaseTypesRecursive(CodeGenClass *CGC, llvm::SmallVector<llvm::Value *, 4> CurrentIdx,
	unsigned Idx) {
	// Per ogni base class di CGC
	for (auto &BaseClass : CGC->Sema->getBaseClasses()) {

		// Need a CodeGen
		if (BaseClass->getCodeGen() == nullptr) {
			CGM->GenClass(BaseClass, false);
		}

		// Tipo LLVM della base
		llvm::StructType *BaseTy = BaseClass->getCodeGen()->getType();

		// // Clone indexes with current indexes
		llvm::SmallVector<llvm::Value *, 4> NewIdx = CurrentIdx;

		// Add current index
		NewIdx.push_back(llvm::ConstantInt::get(CGM->Int32Ty, Idx));

		// llvm::SmallVector<llvm::Constant*,4> NewIdx(CurrentIdx);
		// NewIdx.push_back(llvm::ConstantInt::get(CGM->Int32Ty, idx));

		// Recursive: continue to search into base classes of this base class
		// Start from idx=1 because 0 is vtable*
		BaseClass->getCodeGen()->CollectBaseTypesRecursive(
			BaseClass->getCodeGen(), NewIdx, BaseClass->getClassKind() == SemaClassKind::STRUCT ? 0 : 1);

		// create and populate BaseType
		// BaseType *BT = new BaseType();
		// BT->Type = BaseTy;
		// BT->Index = std::move(newIdx);

		// Salvo la base
		BaseTypes.push_back(new BaseType({BaseTy, NewIdx, BaseClass->getCodeGen()->InitConstructor, BaseClass->getCodeGen()->BaseTypes}));

		// Increment index for next base class
		Idx++;

		// Add Base Type to BodyTypes only for direct bases
		if (CurrentIdx.size() == 1) {
			BodyTypes.push_back(BaseTy);
		}
	}
}

void CodeGenClass::CreateBaseTypes() {
	BaseTypes.clear();

	// Populate RootIdx with 0 because base classes will have index starting from 1 Ex. {0, 1}, {0, 2}, ...
	llvm::SmallVector<llvm::Value *, 4> RootIdx;
	RootIdx.push_back(llvm::ConstantInt::get(CGM->Int32Ty, 0));

	// start from idx=1 because 0 is vtable*
	unsigned StartIndex = Sema->getClassKind() == SemaClassKind::STRUCT ? 0 : 1;

	// Collect all base types recursively
	CollectBaseTypesRecursive(this, RootIdx, StartIndex);
}

void CodeGenClass::CreateAttributeTypes() {
	// Set CodeGen Attributes
	if (!Sema->getAttributes().empty() &&
	    Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::STRUCT) {

		// add var to the type
		for (auto &AttributeEntry : Sema->getAttributes()) {
			SemaClassAttribute *Attribute = AttributeEntry.getValue();
			llvm::Type *AttrType = CGM->GenType(Attribute->getType());

			// Check if the ClassAttribute is a static attribute
			CodeGenVarBase *CGV;
			if (Attribute->isStatic()) {
				llvm::Value *ParentPointer = new llvm::GlobalVariable(
					*CGM->Module, AttrType, Sema->isConstant(),
					llvm::GlobalValue::ExternalLinkage, nullptr);
				CGV = new CodeGenVar(CGM, Attribute, AttrType);
				CGV->setPointer(ParentPointer);
			} else {
				// Create CodeGenVar for Attribute
				CGV = new CodeGenVar(CGM, Attribute, AttrType, BodyTypes.size());
			}
			Attribute->setCodeGen(CGV);

			// Add to Class Var types list
			BodyTypes.push_back(AttrType);
		}
	}
}

void CodeGenClass::CreateVTable() {
	// Create the VTable Global Variable
	if (Sema->getAST()->getClassKind() == ASTClassKind::CLASS ||
	    Sema->getAST()->getClassKind() == ASTClassKind::INTERFACE) {

		// --- Dynamic calculate offset-to-top per Class ---

		// Create the NullPtr to current Class Type pointer*
		llvm::ConstantPointerNull *NullPtr = llvm::ConstantPointerNull::get(TypePtr);

		// Calculate GEP on NullPtr
		llvm::Constant *GEP = llvm::ConstantExpr::getGetElementPtr(Type, NullPtr, CGM->Zero);

		// Convert pointer into int32 (ptrtoint)
		llvm::Constant *OffsetInt = llvm::ConstantExpr::getPtrToInt(GEP, CGM->Int32Ty);

		// Create the VTable Initializer
		VTableValues.push_back(OffsetInt); // First is the OffsetType

		// Second is the Init Constructor
		// VTableValues.push_back(InitConstructor);

		// Add Null Function Pointer for each method in VTable
		for (CodeGenClassMethod *Method : Methods) {
			VTableValues.push_back(Method->getFunction());
		}

		// Create the VTable Constant Struct Value
		llvm::Constant *VTableValue = llvm::ConstantStruct::get(VTableType, VTableValues);

		// Create the VTable Global Variable
		std::string VTableName = (Sema->getAST()->getName() + "_vtable").str();
		std::string Id = CodeGen::toIdentifier(VTableName, Sema->getModule()->getNameSpace()->getName());
		VTable = new llvm::GlobalVariable(
			*CGM->Module, VTableType, true,
			llvm::GlobalValue::ExternalLinkage, VTableValue, Id);
	}
}

void CodeGenClass::CreateInitConstructor() {
	// Create the Init Constructor Name
	std::string FuncName = (Sema->getAST()->getName() + ".init_ctor").str();
	std::string Id = CodeGen::toIdentifier(FuncName, Sema->getModule()->getNameSpace()->getName());

	// Create the Init Constructor Function
	llvm::SmallVector<llvm::Type *, 8> ParamTypes;
	ParamTypes.push_back(TypePtr);
	llvm::FunctionType *FnType = llvm::FunctionType::get(TypePtr, ParamTypes, false);
	InitConstructor = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Id, CGM->getModule());
}

void CodeGenClass::GenInitConstructorBody() {
	// Create the Entry BasicBlock
	llvm::BasicBlock *Entry = llvm::BasicBlock::Create(CGM->LLVMCtx, "entry", InitConstructor);
	CGM->Builder->SetInsertPoint(Entry);
	llvm::Argument *InstanceArg = InitConstructor->getArg(0);

	// Alloca, Store, Load the first arg which is the instance
	llvm::AllocaInst *InstancePtr = CGM->Builder->CreateAlloca(TypePtr);
	CGM->Builder->CreateStore(InstanceArg, InstancePtr);
	llvm::LoadInst *Load = CGM->Builder->CreateLoad(InstancePtr);

	// Store the VTable pointer into the instance
	if (Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::INTERFACE) {
		llvm::Value * VTablePtr = CGM->Builder->CreateInBoundsGEP(Type, Load,  {CGM->Zero, CGM->Zero});
		CGM->Builder->CreateStore(VTable, VTablePtr);
	}

	// Call InitConstructor of all base classes
	for (auto &B : BaseTypes) {
		llvm::Value *BaseInstancePtr = CGM->Builder->CreateInBoundsGEP(Type, Load, B->Index);
		CGM->Builder->CreateCall(B->InitConstructor, {BaseInstancePtr});
	}

	// CodeGen Class Attributes
	for (auto &AttrEntry : Sema->getAttributes()) {
		SemaClassAttribute *Attr = AttrEntry.getValue();

		// Set Value for all Attributes
		llvm::Value *V = CGM->GenExpr(Attr->getAST()->getExpr());

		llvm::ArrayRef<llvm::Value *> IdxList = {
			CGM->Zero, llvm::ConstantInt::get(CGM->Int32Ty, Attr->getCodeGen()->getIndex())};
		llvm::Value *Pointer = CGM->Builder->CreateInBoundsGEP(Type, Load, IdxList);
		CGM->Builder->CreateStore(V, Pointer);
	}

	CGM->Builder->CreateRet(Load);
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

llvm::GlobalVariable *CodeGenClass::getVTable() {
	return VTable;
}

llvm::Function *CodeGenClass::getInitConstructor() {
	return InitConstructor;
}

// const SmallVector<CodeGenClassMethod *, 4> &CodeGenClass::getConstructors() const {
//     return Constructors;
// }

const SmallVector<CodeGenClassMethod *, 4> &CodeGenClass::getMethods() const {
	return Methods;
}

llvm::Value *CodeGenClass::getBaseInstance(llvm::Value *InstancePtr, llvm::StructType *Base) {
	unsigned int I = 1; // Start at 1 because 0 is the vtable pointer
	for (BaseType *B : BaseTypes) {
		if (Base == B->Type) {
			return CGM->Builder->CreateInBoundsGEP(Type, InstancePtr, B->Index);

			// return llvm::ConstantExpr::getGetElementPtr(Type, InstancePtr, B->Index);

			// TODO: search also into base classes of base classes
		}
		I++;
	}

	// Error: Base class type not found
	return nullptr;
}

llvm::Value *CodeGenClass::Downcast(llvm::Type *ToType, llvm::Value *InstancePtr) {
	llvm::Value *Obj = CGM->Builder->CreateBitCast(InstancePtr, TypePtr);

	// TODO: check if ToType is a base class of this class
	// llvm::ArrayRef<llvm::Value *> Idx = getBaseInstance(ToType);
	// llvm::Value *slot = CGM->Builder->CreateStructGEP(Type, Obj, /*vtable field index*/ Idx);
	// CGM->Builder->CreateStore(vtablePtr, slot);

	return Obj;
}

llvm::Value *CodeGenClass::NewInstance() {
	return NewInstance(Sema);
}

llvm::Value *CodeGenClass::NewInstance(SemaClassType *ClassType) {
	llvm::Type *PtrSizedIntTy = CGM->Module->getDataLayout().getIntPtrType(CGM->LLVMCtx);

	// sizeof(ClassType)
	llvm::Constant *SizeOf = llvm::ConstantExpr::getSizeOf(ClassType->getCodeGen()->getType());

	// malloc declaration
	llvm::FunctionCallee MallocFn =
		CGM->Module->getOrInsertFunction(
			"malloc",
			llvm::FunctionType::get(
				llvm::Type::getInt8PtrTy(CGM->LLVMCtx),
				{PtrSizedIntTy},
				false
				)
			);

	// cast sizeof to pointer-sized integer
	llvm::Value *SizeInt = CGM->Builder->CreateIntCast(SizeOf, PtrSizedIntTy, false);

	// call malloc
	llvm::Value *MallocCall = CGM->Builder->CreateCall(MallocFn, {SizeInt});

	// bitcast to %Class*
	llvm::Value *Obj = CGM->Builder->CreateBitCast(
		MallocCall,
		ClassType->getCodeGen()->getType()->getPointerTo()
		);

	llvm::CallInst *InitCall = CGM->Builder->CreateCall(InitConstructor, {Obj});

	return InitCall;
}