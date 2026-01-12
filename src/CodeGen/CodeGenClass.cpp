//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClass.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClass.h"

#include "AST/ASTExpr.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenVar.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaModule.h"

#include "llvm/IR/DerivedTypes.h"

#include <AST/ASTVar.h>
#include <Basic/Debug.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassMethod.h>
#include <llvm/Option/Arg.h>

using namespace fly;

CodeGenClass::CodeGenClass(CodeGenModule *CGM, SemaClassType *Sema, bool isExternal) : CGM(CGM), Sema(Sema) {
	Id = toIdentifier(Sema);

	// Generate Class Type
	Type = llvm::StructType::create(CGM->LLVMCtx, Id);
	TypePtr = Type->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());

	// Create CodeGenVar for Class Instance
	CodeGenVar *CGV = new CodeGenVar(CGM, Sema->getThis(), Type);
	Sema->getThis()->setCodeGen(CGV);

	// Create the Init Constructor Function
	CreateInitConstructor();

	// Create the VTable Type (pointer to global vtable var)
	// %class = { i8** %vtable, ... }
	// Create the VTable Global Vars
	// @vtable.Derived.Base1 = constant [2 x i8*]
	// [
	//   i8 *inttoptr(i64 0 to i8 *),
	//   i8 *bitcast(void(%class.Base1*)*@ Derived_f to i8 *)
	// ]
	CreateVTable();

	// Create the Base Class info
	CreateBaseInfo(Sema->getBaseClasses());

	// Create attributes inside the class
	CreateAttributes();

	// %type = type { %vtable_type, %...inherit_types, %...field_types }
	Type->setBody(BodyType);

	// Generate Init Constructor Body
	GenInitConstructorBody();
}

std::string CodeGenClass::toIdentifier(SemaClassType *ClassType) {
	FLY_DEBUG_START("CodeGenClass", "toIdentifier");
	llvm::StringRef Name = ClassType->getAST().getName();
	SemaNameSpace *NameSpace = ClassType->getModule()->getNameSpace();
	return CGM->toIdentifier(Name, NameSpace);
}

void CodeGenClass::CreateVTable() {
	llvm::SmallVector<llvm::Constant *, 4> VTableArrayValues;

	// Create the VTable Global Variable
	if (Sema->getClassKind() == SemaClassKind::CLASS ||
	    Sema->getClassKind() == SemaClassKind::INTERFACE) {

		// VTable is the first element of the class type body
		BodyType.push_back(CGM->Int8PtrPtrTy);

		// --- Dynamic calculate offset-to-top per Class ---

		// Create the NullPtr to current Class Type pointer*
		// llvm::ConstantPointerNull *NullPtr = llvm::ConstantPointerNull::get(TypePtr);

		// Calculate GEP on NullPtr
		// llvm::Constant *GEP = llvm::ConstantExpr::getGetElementPtr(Type, NullPtr, CGM->Zero);

		// Convert int to ptr
		// i8* inttoptr (i64 0 to i8*), ; offset-to-top = 0
		// FIXME
		llvm::ConstantInt *OffsetInt = llvm::ConstantInt::get(CGM->Int64Ty, 0);
		llvm::Constant *OffsetToTop = llvm::ConstantExpr::getIntToPtr(OffsetInt, CGM->Int8PtrTy);

		// Create the VTable Initializer
		VTableArrayValues.push_back(OffsetToTop); // First is the OffsetType

		// Generate Constructors only for Class
		// Struct uses only the Init Constructor
		// Interface doesn't have constructors
		if (Sema->getClassKind() == SemaClassKind::CLASS) {

			// Add Init Constructor Fn Type
			// void (%ClassType*)*
			// llvm::PointerType *InitCtorPtrType = InitConstructor->getFunctionType()->getPointerTo(
			// 	CGM->Module->getDataLayout().getAllocaAddrSpace());
			// VTableMethodTypes.push_back(InitCtorPtrType);

			// Add Constructors
			for (auto &Node : Sema->getNodes()) {
				SemaClassMethod *Method = static_cast<SemaClassMethod *>(Node);
				CodeGenClassMethod *CG;

				// CodeGen not yet generated
				if (Method->getCodeGen() == nullptr) {

					// Create CodeGen for Method: index + offset-to-type
					CG = new CodeGenClassMethod(CGM, Method, Type, Methods.size() + 1);
					Method->setCodeGen(CG);

					// Add to Class Methods
					Methods.push_back(CG);
					CGM->CGFunctions.push_back(CG);
				} else {
					// CodeGen generated from super class
					CG = Method->getCodeGen();
				}

				// Add to VTable
				if (!Method->isStatic()) {
					// Add to VTable only instance methods
					llvm::PointerType *FnPtrType = CG->getFunctionType()->getPointerTo(
						CGM->Module->getDataLayout().getAllocaAddrSpace());
				}
			}
		}

		// Add Null Function Pointer for each method in VTable
		for (CodeGenClassMethod *Method : Methods) {
			llvm::Constant * MethodIntPtr = llvm::ConstantExpr::getBitCast(Method->getFunction(), CGM->Int8PtrTy);
			VTableArrayValues.push_back(MethodIntPtr);
		}

		// Create an array of i8**
		llvm::ArrayType *ArrayOfInt8Ptr = llvm::ArrayType::get(CGM->Int8PtrTy, VTableArrayValues.size());

		// Create the VTable Constant Struct Value
		llvm::Constant *ArrayValue = llvm::ConstantArray::get(ArrayOfInt8Ptr, VTableArrayValues);

		// Create the VTable Global Variable
		std::string VTableName = "vtable." + Id;
		VTable = new llvm::GlobalVariable(
			*CGM->Module, ArrayOfInt8Ptr, true,
			llvm::GlobalValue::ExternalLinkage, ArrayValue, VTableName);

		CreateBaseVTables(VTableName, Sema, 0);
	}
}

void CodeGenClass::CreateBaseVTables(std::string VTableName, SemaClassType *Derived, size_t Offset) {
	// for (auto &BaseClass : Derived->getBaseClasses()) {
	// 	std::string VTableBaseName = VTableName + "." + BaseClass->getAST().getName().str();
	// 	new llvm::GlobalVariable(*CGM->Module, ArrayOfInt8Ptr, true,
	// 		llvm::GlobalValue::ExternalLinkage, ArrayValue, VTableName);
	// 	CreateBaseVTables(VTableName, BaseClass, Offset);
	// }
}


void CodeGenClass::CreateBaseInfo(llvm::SmallVector<SemaClassType *, 4> BaseClasses) {
	for (auto &BaseClass : BaseClasses) {

		// Need a CodeGen
		if (BaseClass->getCodeGen() == nullptr) {
			CGM->GenClass(BaseClass, false);
		}

		// Add Base Class Type to the BodyType
		BodyType.push_back(BaseClass->getCodeGen()->getType());
	}
}

void CodeGenClass::CreateAttributes() {
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
				CGV = new CodeGenVar(CGM, Attribute, AttrType, BodyType.size());
			}
			Attribute->setCodeGen(CGV);

			// Add to Class Var types list
			BodyType.push_back(AttrType);
		}
	}
}

void CodeGenClass::CreateInitConstructor() {
	// Create the Init Constructor Name
	std::string CtorId = Id + ".init_ctor";

	// Create the Init Constructor Function
	llvm::SmallVector<llvm::Type *, 8> ParamTypes;
	ParamTypes.push_back(TypePtr);
	llvm::FunctionType *FnType = llvm::FunctionType::get(TypePtr, ParamTypes, false);
	InitConstructor = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, CtorId, CGM->getModule());
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

	// Only for Class and Interface: Initialize Base Classes
	if (Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::INTERFACE) {

		// Store the VTable pointer into the instance
		llvm::Value * VTablePtr = CGM->Builder->CreateInBoundsGEP(Type, Load,  {CGM->Zero, CGM->Zero});
		llvm::Value * VTableBitCast = CGM->Builder->CreateBitCast(VTable, CGM->Int8PtrPtrTy);
		CGM->Builder->CreateStore(VTableBitCast, VTablePtr);

		// Call InitConstructor of all base classes
		size_t BaseIndex = 1; // Start at 1 because 0 is the vtable pointer
		for (auto &Base : Sema->getBaseClasses()) {
			if (Base->getClassKind() == SemaClassKind::CLASS) {
				llvm::ConstantInt *Index = llvm::ConstantInt::get(CGM->Int32Ty, BaseIndex); // Get th index from GlobalVariable of Vtable
				llvm::Value *BaseInstancePtr = CGM->Builder->CreateInBoundsGEP(Type, Load, {CGM->Zero, Index});
				CGM->Builder->CreateCall(Base->getCodeGen()->getInitConstructor(), {BaseInstancePtr});
			}
			BaseIndex ++;
		}
	}

	// CodeGen Class Attributes
	for (auto &AttrEntry : Sema->getAttributes()) {
		SemaClassAttribute *Attr = AttrEntry.getValue();

		// Set Value for all Attributes
		llvm::Value *V = CGM->GenExpr(Attr->getAST()->getExpr()->getSema());

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

llvm::Value *CodeGenClass::getBaseInstance(llvm::Value *InstancePtr, SemaClassType *Base) {
	size_t BaseIndex = 1; // Start at 1 because 0 is the vtable pointer
	for (auto &B : Sema->getBaseClasses()) {
		if (Base->isEquals(B)) {
			llvm::ConstantInt *Index = llvm::ConstantInt::get(CGM->Int32Ty, BaseIndex);
			// TODO search into base classes of base classes
			return CGM->Builder->CreateInBoundsGEP(Type, InstancePtr, {CGM->Zero, Index});

			// return llvm::ConstantExpr::getGetElementPtr(Type, InstancePtr, B->Index);

			// TODO: search also into base classes of base classes
		}
		BaseIndex++;
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