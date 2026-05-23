//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenClass.cpp - class type code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClass.h"

#include "AST/ASTExpr.h"
#include "AST/ASTName.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenVar.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaModule.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"

#include <AST/ASTVar.h>
#include <Basic/Debug.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassMethod.h>
#include <llvm/Option/Arg.h>

using namespace fly;

CodeGenClass::CodeGenClass(CodeGenModule *CGM, SemaClassType *Sema, bool isExternal) : CodeGenType(CGM), Sema(Sema), IsExternal(isExternal) {
	Id = toIdentifier(Sema);

	// Generate Class Type
	Type = llvm::StructType::create(CGM->LLVMCtx, Id);
	T = Type; // Also set base class CodeGenType::T for polymorphic access
	TypePtr = Type->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());

	// Create CodeGenVar for Class Instance
	CodeGenVar *CGV = new CodeGenVar(CGM, Sema->getThis(), Type);
	Sema->getThis()->setCodeGen(CGV);

	// Create the Init Constructor Function only for concrete (non-abstract, non-interface) classes
	if (Sema->getClassKind() != SemaClassKind::INTERFACE && !Sema->isAbstract()) {
		CreateInitConstructor();
	}

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

	if (!IsExternal) {
		// Generate per-base vtables and thunks for multiple inheritance
		// Must be called after Type->setBody so struct layout is available for byte-offset computation
		if (Sema->getClassKind() == SemaClassKind::CLASS) {
			CreateBaseVTables();
		}

		// Generate Init Constructor Body only for concrete classes
		if (Sema->getClassKind() != SemaClassKind::INTERFACE && !Sema->isAbstract()) {
			GenInitConstructorBody();
		}
	}
}

std::string CodeGenClass::toIdentifier(SemaClassType *ClassType) {
	FLY_DEBUG_SCOPE("CodeGenClass", "toIdentifier");
	llvm::StringRef Name = ClassType->getAST().getName();
	SemaNameSpace *NameSpace = ClassType->getModule().getNameSpace();
	return CGM->toIdentifier(Name, NameSpace);
}

void CodeGenClass::CreateVTable() {
	llvm::SmallVector<llvm::Constant *, 4> VTableArrayValues;

	// Create the VTable Global Variable
	if (Sema->getClassKind() == SemaClassKind::CLASS ||
	    Sema->getClassKind() == SemaClassKind::INTERFACE) {

		// VTable is the first element of the class type body
		BodyType.push_back(CodeGen::Int8PtrPtrTy);

		// --- Dynamic calculate offset-to-top per Class ---

		// Create the NullPtr to current Class Type pointer*
		// llvm::ConstantPointerNull *NullPtr = llvm::ConstantPointerNull::get(TypePtr);

		// Calculate GEP on NullPtr
		// llvm::Constant *GEP = llvm::ConstantExpr::getGetElementPtr(Type, NullPtr, CodeGen::Zero);

		// Convert int to ptr
		// i8* inttoptr (i64 0 to i8*), ; offset-to-top = 0
		// FIXME
		llvm::ConstantInt *OffsetInt = llvm::ConstantInt::get(CodeGen::Int64Ty, 0);
		llvm::Constant *OffsetToTop = llvm::ConstantExpr::getIntToPtr(OffsetInt, CodeGen::Int8PtrTy);

		// Create the VTable Initializer
		VTableArrayValues.push_back(OffsetToTop); // First is the OffsetType

		// Generate methods only for CLASS (not INTERFACE — interface methods are blueprints only)
		if (Sema->getClassKind() == SemaClassKind::CLASS) {
			for (auto &Node : Sema->getNodes()) {
				// Skip non-method nodes (e.g., SemaClassAttribute)
				if (Node->getKind() != SemaKind::METHOD) {
					continue;
				}
				SemaClassMethod *Method = static_cast<SemaClassMethod *>(Node);
				CodeGenClassMethod *CG;

				// CodeGen not yet generated
				if (Method->getCodeGen() == nullptr) {
					// Create CodeGen for Method: index + offset-to-type
					CG = new CodeGenClassMethod(CGM, Method, Type, Methods.size() + 1);
					Method->setCodeGen(CG);

					// Add to Class Methods list
					Methods.push_back(CG);

					// Abstract methods have no body — do not schedule for code generation.
					// External classes are defined in the library; never schedule their methods.
					if (!Method->isAbstract() && !IsExternal) {
						CGM->Functions.push_back(Method);
					}
				} else {
					// CodeGen generated from super class
					CG = Method->getCodeGen();
				}
			}
		} else if (Sema->getClassKind() == SemaClassKind::INTERFACE) {
			// Interface methods have no function body but need a CodeGen carrying their
			// vtable index so that CodeGenExpr can resolve Method->getCodeGen()->getIndex()
			// when emitting vtable dispatch from standalone functions (e.g. readAll(Reader r)).
			// Without this, getCodeGen() returns nullptr and the compiler crashes.
			for (auto &Node : Sema->getNodes()) {
				if (Node->getKind() != SemaKind::METHOD) continue;
				SemaClassMethod *Method = static_cast<SemaClassMethod *>(Node);
				if (Method->isConstructor() || Method->isStatic()) continue;
				if (Method->getCodeGen() == nullptr) {
					// Index starts at 1: slot 0 in the vtable is the offset-to-top.
					CodeGenClassMethod *CG = new CodeGenClassMethod(CGM, Method, Type, Methods.size() + 1);
					Method->setCodeGen(CG);
					Methods.push_back(CG);
					// Interface methods are abstract — no function body to schedule.
				}
			}
		}

		// Populate vtable: null for abstract/interface methods, function ptr for concrete methods
		for (CodeGenClassMethod *Method : Methods) {
			llvm::Function *MethodFn = Method->getFunction();
			if (MethodFn) {
				llvm::Constant *MethodIntPtr = llvm::ConstantExpr::getBitCast(MethodFn, CodeGen::Int8PtrTy);
				VTableArrayValues.push_back(MethodIntPtr);
			} else {
				// Abstract method: null vtable slot
				VTableArrayValues.push_back(llvm::Constant::getNullValue(CodeGen::Int8PtrTy));
			}
		}

		// Create an array of i8**
		llvm::ArrayType *ArrayOfInt8Ptr = llvm::ArrayType::get(CodeGen::Int8PtrTy, VTableArrayValues.size());

		// Create the VTable Constant Struct Value
		llvm::Constant *ArrayValue = llvm::ConstantArray::get(ArrayOfInt8Ptr, VTableArrayValues);

		// External classes already have their vtable defined in the library.
		std::string VTableName = "vtable." + Id;
		if (IsExternal) {
			VTable = nullptr;
		} else {
			VTable = new llvm::GlobalVariable(
				*CGM->Module, ArrayOfInt8Ptr, true,
				llvm::GlobalValue::ExternalLinkage, ArrayValue, VTableName);
		}
	}
}

void CodeGenClass::CreateBaseVTables() {
	std::string VTableName = "vtable." + Id;
	const llvm::StructLayout *Layout = CGM->Module->getDataLayout().getStructLayout(Type);

	size_t FieldIdx = 1; // Field 0 is Derived's own vtable pointer; base subobjects start at 1
	for (auto &Base : Sema->getBaseClasses()) {
		// STRUCT bases have no vtable — skip them
		if (Base->getClassKind() == SemaClassKind::STRUCT) {
			VTableBases.push_back(nullptr);
			FieldIdx++;
			continue;
		}

		// For INTERFACE: always create per-base vtable (all methods must be overridden).
		// For CLASS/ABSTRACT: only create per-base vtable if at least one method is overridden.
		bool NeedsPerBaseVTable = (Base->getClassKind() == SemaClassKind::INTERFACE);
		if (!NeedsPerBaseVTable) {
			for (auto &Node : Base->getNodes()) {
				if (Node->getKind() != SemaKind::METHOD) continue;
				SemaClassMethod *M = static_cast<SemaClassMethod *>(Node);
				if (M->isConstructor() || M->isStatic()) continue;
				if (FindOverrideInDerived(Sema, M) != nullptr) {
					NeedsPerBaseVTable = true;
					break;
				}
			}
		}

		if (!NeedsPerBaseVTable) {
			VTableBases.push_back(nullptr);
			FieldIdx++;
			continue;
		}

		// Compute byte offset of this base subobject within the derived struct
		uint64_t BaseByteOffset = Layout->getElementOffset(FieldIdx);

		// Build the per-base vtable entries
		llvm::SmallVector<llvm::Constant *, 8> VTableValues;

		// Slot 0: offset-to-top (negative byte offset)
		llvm::ConstantInt *OffsetInt = llvm::ConstantInt::get(CodeGen::Int64Ty, -(int64_t)BaseByteOffset);
		llvm::Constant *OffsetToTop = llvm::ConstantExpr::getIntToPtr(OffsetInt, CodeGen::Int8PtrTy);
		VTableValues.push_back(OffsetToTop);

		// Slots 1..N: one per virtual instance method of the base
		size_t MethodIdx = 1;
		for (auto &Node : Base->getNodes()) {
			if (Node->getKind() != SemaKind::METHOD) continue;
			SemaClassMethod *BaseMethod = static_cast<SemaClassMethod *>(Node);
			if (BaseMethod->isConstructor() || BaseMethod->isStatic()) continue;

			// Ensure base method has a CodeGenClassMethod so dispatch can read its vtable index.
			// Interface methods never get one from CreateVTable(), so we create it here.
			if (BaseMethod->getCodeGen() == nullptr) {
				if (Base->getCodeGen() == nullptr) {
					MethodIdx++;
					continue;
				}
				CodeGenClassMethod *CG = new CodeGenClassMethod(
					CGM, BaseMethod, Base->getCodeGen()->getType(), MethodIdx);
				BaseMethod->setCodeGen(CG);
			}

			// Find the concrete override in the derived class
			SemaClassMethod *Override = FindOverrideInDerived(Sema, BaseMethod);
			llvm::Constant *Slot;
			if (Override && Override->getCodeGen() && Override->getCodeGen()->getFunction()) {
				// Derived overrides this method: thunk adjusts this ptr before calling derived impl
				llvm::Function *Thunk = CreateThunk(BaseMethod, Override, BaseByteOffset);
				Slot = llvm::ConstantExpr::getBitCast(Thunk, CodeGen::Int8PtrTy);
			} else if (BaseMethod->getCodeGen() && BaseMethod->getCodeGen()->getFunction()) {
				// Concrete base method not overridden: dispatch directly to base implementation.
				// No thunk needed: the this ptr already points to the base subobject.
				Slot = llvm::ConstantExpr::getBitCast(BaseMethod->getCodeGen()->getFunction(), CodeGen::Int8PtrTy);
			} else {
				// Abstract method with no implementation (should not be reachable in a valid program)
				Slot = llvm::Constant::getNullValue(CodeGen::Int8PtrTy);
			}
			VTableValues.push_back(Slot);
			MethodIdx++;
		}

		// Create the per-base vtable global variable
		llvm::ArrayType *ArrayTy = llvm::ArrayType::get(CodeGen::Int8PtrTy, VTableValues.size());
		llvm::Constant *ArrayVal = llvm::ConstantArray::get(ArrayTy, VTableValues);
		std::string BaseVTableName = VTableName + "." + Base->getAST().getName().str();
		llvm::GlobalVariable *BaseVTable = new llvm::GlobalVariable(
			*CGM->Module, ArrayTy, true,
			llvm::GlobalValue::ExternalLinkage, ArrayVal, BaseVTableName);
		VTableBases.push_back(BaseVTable);

		FieldIdx++;
	}
}

llvm::Function *CodeGenClass::CreateThunk(SemaClassMethod *BaseMethod, SemaClassMethod *Override,
                                           uint64_t BaseOffset) {
	CodeGenClassMethod *BaseCG = BaseMethod->getCodeGen();
	llvm::FunctionType *ThunkType = BaseCG->getFunctionType();

	// Name: thunk.DerivedName.BaseName.methodName
	std::string ThunkName = "thunk." + Id + "." +
		BaseMethod->getClass()->getAST().getName().str() + "." +
		BaseMethod->getName().str();

	// Avoid duplicate thunks (diamond inheritance)
	if (llvm::Function *Existing = CGM->getModule()->getFunction(ThunkName)) {
		return Existing;
	}

	llvm::Function *Thunk = llvm::Function::Create(
		ThunkType, llvm::GlobalValue::InternalLinkage, ThunkName, CGM->getModule());

	llvm::BasicBlock *Entry = llvm::BasicBlock::Create(CGM->LLVMCtx, "entry", Thunk);
	llvm::IRBuilder<> Builder(Entry);

	llvm::SmallVector<llvm::Value *, 8> Args;

	// STRUCT classes have no error-handler param; all others have it as arg 0
	bool IsStruct = (BaseMethod->getClass()->getAST().getClassKind() == ASTClassKind::STRUCT);
	unsigned InstanceArgIdx = IsStruct ? 0 : 1;

	// Forward error handler if present
	if (!IsStruct) {
		Args.push_back(Thunk->getArg(0));
	}

	// Adjust this pointer: base_subobj_ptr - BaseOffset = derived_ptr
	llvm::Value *BaseThis = Thunk->getArg(InstanceArgIdx);
	llvm::Value *BaseInt  = Builder.CreatePtrToInt(BaseThis, CodeGen::Int64Ty);
	llvm::Value *AdjInt   = Builder.CreateSub(BaseInt,
		llvm::ConstantInt::get(CodeGen::Int64Ty, BaseOffset));
	llvm::Value *DerivedThis = Builder.CreateIntToPtr(AdjInt, BaseThis->getType());
	Args.push_back(DerivedThis);

	// Forward remaining params unchanged
	for (unsigned I = InstanceArgIdx + 1; I < Thunk->arg_size(); I++) {
		Args.push_back(Thunk->getArg(I));
	}

	// Call the concrete derived implementation
	CodeGenClassMethod *OverrideCG = Override->getCodeGen();
	llvm::CallInst *Call = Builder.CreateCall(
		OverrideCG->getFunctionType(), OverrideCG->getFunction(), Args);

	if (ThunkType->getReturnType()->isVoidTy()) {
		Builder.CreateRetVoid();
	} else {
		Builder.CreateRet(Call);
	}

	return Thunk;
}

SemaClassMethod *CodeGenClass::FindOverrideInDerived(SemaClassType *Derived, SemaClassMethod *BaseMethod) {
	for (auto &Node : Derived->getNodes()) {
		if (Node->getKind() != SemaKind::METHOD) continue;
		SemaClassMethod *Method = static_cast<SemaClassMethod *>(Node);
		if (Method->getOverridden() == BaseMethod) {
			return Method;
		}
	}
	return nullptr;
}


void CodeGenClass::CreateBaseInfo(llvm::SmallVector<SemaClassType *, 4> BaseClasses) {
	for (auto &BaseClass : BaseClasses) {
		BaseClass->accept(*CGM);

		// Add Base Class Type to the BodyType
		BodyType.push_back(BaseClass->getCodeGen()->getType());
	}
}

void CodeGenClass::CreateAttributes() {
	// Set CodeGen Attributes
	if (!Sema->getAttributes().empty() &&
		(Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::STRUCT)) {

		// add var to the type
		for (auto &AttributeEntry : Sema->getAttributes()) {
			SemaClassAttribute *Attribute = AttributeEntry.getValue();
			Attribute->getType()->accept(*CGM);
			llvm::Type *AttrType = Attribute->getType()->getCodeGen()->getType();

			// Check if the ClassAttribute is a static attribute
			CodeGenVar *CGV;
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
	llvm::GlobalValue::LinkageTypes Linkage = IsExternal
		? llvm::GlobalValue::ExternalLinkage
		: llvm::GlobalValue::LinkOnceODRLinkage;
	InitConstructor = llvm::Function::Create(FnType, Linkage, CtorId, CGM->getModule());
}

void CodeGenClass::GenInitConstructorBody() {
	// Create the Entry BasicBlock
	llvm::BasicBlock *Entry = llvm::BasicBlock::Create(CGM->LLVMCtx, "entry", InitConstructor);
	CGM->Builder->SetInsertPoint(Entry);
	llvm::Argument *InstanceArg = InitConstructor->getArg(0);

	// Alloca, Store, Load the first arg which is the instance
	llvm::AllocaInst *InstancePtr = CGM->Builder->CreateAlloca(TypePtr);
	CGM->Builder->CreateStore(InstanceArg, InstancePtr);
	llvm::LoadInst *Load = CGM->Builder->CreateLoad(TypePtr, InstancePtr);

	// Only for Class and Interface: Initialize Base Classes
	if (Sema->getClassKind() == SemaClassKind::CLASS || Sema->getClassKind() == SemaClassKind::INTERFACE) {

		// Store the VTable pointer into the instance
		llvm::Value * VTablePtr = CGM->Builder->CreateInBoundsGEP(Type, Load,  {CodeGen::Zero, CodeGen::Zero});
		llvm::Value * VTableBitCast = CGM->Builder->CreateBitCast(VTable, CodeGen::Int8PtrPtrTy);
		CGM->Builder->CreateStore(VTableBitCast, VTablePtr);

		// Initialize base class subobjects and store per-base vtable pointers
		size_t BaseIndex = 1; // Start at 1 because 0 is the vtable pointer
		for (size_t I = 0; I < Sema->getBaseClasses().size(); I++) {
			SemaClassType *Base = Sema->getBaseClasses()[I];
			llvm::ConstantInt *FieldConst = llvm::ConstantInt::get(CodeGen::Int32Ty, BaseIndex);
			llvm::Value *BaseSubobjPtr = CGM->Builder->CreateInBoundsGEP(Type, Load, {CodeGen::Zero, FieldConst});

			// Call InitConstructor for concrete CLASS and STRUCT bases
			if ((Base->getClassKind() == SemaClassKind::CLASS && !Base->isAbstract()) ||
			    Base->getClassKind() == SemaClassKind::STRUCT) {
				CGM->Builder->CreateCall(Base->getCodeGen()->getInitConstructor(), {BaseSubobjPtr});
			}

			// Store per-base vtable pointer into the base subobject's vtable slot
			if (I < VTableBases.size() && VTableBases[I] != nullptr) {
				llvm::Value *BaseVTablePtrPtr = CGM->Builder->CreateStructGEP(
					Base->getCodeGen()->getType(), BaseSubobjPtr, 0);
				llvm::Value *BaseVTableCast = CGM->Builder->CreateBitCast(
					VTableBases[I], CodeGen::Int8PtrPtrTy);
				CGM->Builder->CreateStore(BaseVTableCast, BaseVTablePtrPtr);
			}

			BaseIndex++;
		}
	}

	// CodeGen Class Attributes
	for (auto &AttrEntry : Sema->getAttributes()) {
		SemaClassAttribute *Attr = AttrEntry.getValue();

		// Static attributes are backed by a GlobalVariable; they are not part of
		// the instance struct and must not be initialized here.
		if (Attr->isStatic()) continue;

		/** Fixed
		* llvm::ArrayRef<llvm::Value *> IdxList = {
		*	CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, Index)
		* };
		*  IdxList is initialized from a std::initializer_list
		*  and stored as an ArrayRef, but the initializer list's backing array is destroyed at the end of that
		*  statement, leaving IdxList.Data dangling. In a Release -O3 build, the stack memory gets reused and the
		*  second element reads as null.
		**/

		// Initialize attribute: use programmer-supplied default if present, else zero.
		llvm::Value *Pointer = CGM->Builder->CreateInBoundsGEP(Type, Load,
			{CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, Attr->getCodeGen()->getIndex())});
		Attr->getCodeGen()->setPointer(Pointer);
		if (Attr->getInitExpr()) {
			Attr->getInitExpr()->accept(*CGM);
			CGM->Builder->CreateStore(Attr->getInitExpr()->getCodeGen()->getValue(), Pointer);
		} else {
			Attr->getCodeGen()->StoreDefaultValue();
		}
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
	size_t BaseIndex = 1; // 0 is the vtable pointer
	for (auto &B : Sema->getBaseClasses()) {
		llvm::ConstantInt *Index = llvm::ConstantInt::get(CodeGen::Int32Ty, BaseIndex);
		if (Base->isEquals(B)) {
			return CGM->Builder->CreateInBoundsGEP(Type, InstancePtr, {CodeGen::Zero, Index});
		}
		// Base is a proper ancestor of B — recurse into B's sub-object
		if (Base->isBaseOrEquals(B)) {
			llvm::Value *BSubobjPtr = CGM->Builder->CreateInBoundsGEP(Type, InstancePtr, {CodeGen::Zero, Index});
			if (CodeGenClass *BCG = B->getCodeGen()) {
				return BCG->getBaseInstance(BSubobjPtr, Base);
			}
		}
		BaseIndex++;
	}
	return nullptr;
}

llvm::Value *CodeGenClass::Downcast(llvm::Type *ToType, llvm::Value *InstancePtr) {
	llvm::Value *Obj = CGM->Builder->CreateBitCast(InstancePtr, TypePtr);
	// TODO: verify ToType is a base class of this class and adjust vtable pointer
	return Obj;
}


