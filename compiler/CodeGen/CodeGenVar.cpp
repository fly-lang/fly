//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenVar.cpp - variable code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenVar.h"

#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenArrayValue.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaValue.h"

#include <Sema/SemaCall.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaMember.h>
#include <Sema/SemaVar.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

CodeGenVar::CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T) : CodeGenExpr(CGM),
	Sema(Sema), T(T) {

}

CodeGenVar::CodeGenVar(CodeGenModule *CGM, SemaVar *Sema, llvm::Type *T, size_t Index) : CodeGenExpr(CGM),
	Sema(Sema), T(T), Index(Index) {

}

llvm::Type *CodeGenVar::getType() {
    return T;
}

llvm::AllocaInst *CodeGenVar::Alloca() {

	if (T == CodeGen::ArrayTy || T == CodeGen::StringTy) {
		// Value-type structs: allocate the struct directly
		this->Pointer = CGM->Builder->CreateAlloca(T);
		// Zero-initialise a string slot ({null, 0}) at the point of allocation. An early
		// `return` in a nested block runs the full scope cleanup, which frees every string
		// local in the frame — including ones whose declaration the control flow has not
		// reached yet. Without this, that cleanup loads an uninitialised slot and frees a
		// garbage pointer; with it, the freed pointer is null (free(null) is a safe no-op).
		if (T == CodeGen::StringTy)
			CGM->Builder->CreateStore(llvm::Constant::getNullValue(CodeGen::StringTy), this->Pointer);
	} else if (T->isStructTy()) {
		llvm::PointerType *PtrTy = T->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
		this->Pointer = CGM->Builder->CreateAlloca(PtrTy);

		// For STRUCT kind: pre-allocate data on the caller's stack and point ptr_slot at it.
		// CLASS/INTERFACE keep the ptr_slot uninitialised here; addArgs() will malloc for them.
		SemaType *SemaTy = Sema->getType();
		if (SemaTy && SemaTy->getKind() == SemaKind::TYPE_CLASS &&
		    static_cast<SemaClassType *>(SemaTy)->getClassKind() == SemaClassKind::STRUCT) {
			llvm::AllocaInst *DataAlloca = CGM->Builder->CreateAlloca(T);
			CGM->Builder->CreateStore(DataAlloca, this->Pointer);
		}
	} else {
		// Alloca for non-struct types
		// Check if the type is bool (i1) and convert it to i8
		this->Pointer = CGM->Builder->CreateAlloca(T->isIntegerTy(1) ? CodeGen::Int8Ty : T);
	}
	return llvm::cast<llvm::AllocaInst>(this->Pointer);
}

llvm::StoreInst *CodeGenVar::Store(llvm::Value *Val) {
    this->LoadBlock = nullptr;
    this->LoadI = nullptr;
    // Fix Architecture Compatibility of bool i1 to i8
    if (T->isIntegerTy(1)) {
        Val = CGM->Builder->CreateZExt(Val, CodeGen::Int8Ty);
    }
    // Allow class ↔ long: ptrtoint / inttoptr for pointer-sized integer interchange.
    if (T->isIntegerTy(64) && Val->getType()->isPointerTy()) {
        Val = CGM->Builder->CreatePtrToInt(Val, CodeGen::Int64Ty);
    } else if (T->isPointerTy() && Val->getType()->isIntegerTy(64)) {
        Val = CGM->Builder->CreateIntToPtr(Val, llvm::PointerType::getUnqual(CGM->LLVMCtx));
    }
	return CGM->Builder->CreateStore(Val, getPointer());
}

llvm::Value *CodeGenVar::StoreArrayValue(CodeGenArrayValue *ArrayValue) {
	llvm::Value *ArrayPtr = ArrayValue->getValue();
	const std::vector<llvm::Value *> &Values = ArrayValue->getValues();
	if (!Values.empty()) {
		// Store each value into the allocated array data
		for (size_t i = 0; i < Values.size(); i++) {
			llvm::Value *Index = llvm::ConstantInt::get(CodeGen::IntPtrTy, i);
			llvm::Value *ElemPtr = CGM->Builder->CreateGEP(ArrayValue->getElementType(), ArrayPtr, Index);
			CGM->Builder->CreateStore(Values[i], ElemPtr);
		}
		return ArrayPtr; // No need to return a store instruction for the array struct itself
	}

	// Store Null pointer
	llvm::Value *NullPtr = llvm::Constant::getNullValue(CodeGen::ArrayTy->getPointerTo());
	return CGM->Builder->CreateStore(NullPtr, getPointer());
}

llvm::Value *CodeGenVar::getDefaultValue(llvm::Type *T) {
	if (T == CodeGen::StringTy) {
		// Zero-initialize the string struct: { null, 0 }
		return llvm::Constant::getNullValue(T);
	}
	llvm::Value *DefaultValue = nullptr;
	switch (T->getTypeID()) {
		case llvm::Type::IntegerTyID:
			DefaultValue = llvm::ConstantInt::get(T, 0);
			break;
		case llvm::Type::FloatTyID:
		case llvm::Type::DoubleTyID:
			DefaultValue = llvm::ConstantFP::get(T, 0.0);
			break;
		case llvm::Type::PointerTyID:
			DefaultValue = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(T));
			break;
		case llvm::Type::StructTyID:
			// Zero-initialize the embedded struct value
			DefaultValue = llvm::Constant::getNullValue(T);
			break;
		default:
			CGM->Diag(diag::err_invalid_behavior);
			break;
	}
	return DefaultValue;
}

llvm::StoreInst *CodeGenVar::StoreDefaultValue() {
	SemaType *Ty = Sema->getType();

	// Check if this is a dynamic array (CodeGen::ArrayTy structure)
	if (Ty->isArray()) {
		SemaArrayType *ArrayTy = static_cast<SemaArrayType *>(Ty);
		llvm::Type *ElementType = ArrayTy->getElementType()->getCodeGen()->getType();

		// Get the size from expression or constant
		llvm::Value *Size = nullptr;
		bool RuntimeCheck = false;

		// Check if size expression exists and is a constant value
		if (ArrayTy->getSizeExpr()) {
			if (ArrayTy->getSizeExpr()->getKind() == SemaKind::VALUE) {

				// Check if size is zero
				llvm::APInt Int = static_cast<SemaIntValue *>(ArrayTy->getSizeExpr())->getValue();
				if (Int == 0) {
					// Size is zero, store null pointer
					llvm::Value *NullPtr = llvm::Constant::getNullValue(CodeGen::ArrayTy->getPointerTo());
					return CGM->Builder->CreateStore(NullPtr, getPointer());
				}
			} else {
				// Size is not a constant, need runtime check
				RuntimeCheck = true;
			}
			Size = ArrayTy->getSizeExpr()->getCodeGen()->getValue();
		} else if (ArrayTy->getSize() > 0) {

			// Size is a positive constant, can be used directly
			Size = llvm::ConstantInt::get(CodeGen::Int64Ty, ArrayTy->getSize());
		} else {

			// Size is zero or unspecified, store null pointer
			llvm::Value *NullPtr = llvm::Constant::getNullValue(CodeGen::ArrayTy->getPointerTo());
			return CGM->Builder->CreateStore(NullPtr, getPointer());
		}

		llvm::BasicBlock *MallocBB = nullptr;
		llvm::BasicBlock *ContBB = nullptr;

		if (RuntimeCheck) {
			// Create basic blocks for conditional execution
			MallocBB = llvm::BasicBlock::Create(CGM->getLLVMCtx(), "array.malloc", CGM->Builder->GetInsertBlock()->getParent());
			ContBB = llvm::BasicBlock::Create(CGM->getLLVMCtx(), "array.default", CGM->Builder->GetInsertBlock()->getParent());

			// Runtime check: Size > 0
			llvm::Value *IsPositive = CGM->Builder->CreateICmpSGT(Size, llvm::ConstantInt::get(Size->getType(), 0));
			CGM->Builder->CreateCondBr(IsPositive, MallocBB, ContBB);

			CGM->Builder->SetInsertPoint(MallocBB);
		}

		// Use i8 0 for memset value
		llvm::ConstantInt *ZeroInt8 = llvm::ConstantInt::get(CodeGen::Int8Ty, 0);

		// Allocate array data in heap: malloc(Size * sizeof(ElementType))
		llvm::TypeSize ElemAllocSize = CGM->Module->getDataLayout().getTypeAllocSize(ElementType);
		llvm::Value *ElemAllocSizeVal = llvm::ConstantInt::get(Size->getType(), ElemAllocSize.getFixedValue());
		llvm::Value *TotalAllocSize = CGM->Builder->CreateMul(Size, ElemAllocSizeVal);
		llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
			"malloc",
			llvm::FunctionType::get(
				llvm::PointerType::getUnqual(CGM->LLVMCtx),
				{CodeGen::IntPtrTy},
				false));
		llvm::Value *TotalAllocSizeCast = CGM->Builder->CreateIntCast(TotalAllocSize, CodeGen::IntPtrTy, false);
		llvm::Value *DataPtr = CGM->Builder->CreateCall(MallocFn, {TotalAllocSizeCast});

		// Zero-initialize the data array (TotalAllocSize already computed above)
		CGM->Builder->CreateMemSet(DataPtr, ZeroInt8, TotalAllocSize, llvm::MaybeAlign());

		// Get pointer to field 0 (data pointer)
		llvm::Value *Field0Ptr = CGM->Builder->CreateStructGEP(CodeGen::ArrayTy, this->Pointer, 0);
		CGM->Builder->CreateStore(DataPtr, Field0Ptr);

		// Get pointer to field 1 (dims)
		llvm::Value *Field1Ptr = CGM->Builder->CreateStructGEP(CodeGen::ArrayTy, this->Pointer, 1);
		llvm::Value *DimsValue = CGM->Builder->CreateIntCast(Size, CodeGen::IntPtrTy, false);
		CGM->Builder->CreateStore(DimsValue, Field1Ptr);

		if (RuntimeCheck) {
			CGM->Builder->CreateBr(ContBB);
			CGM->Builder->SetInsertPoint(ContBB);
		}

		return nullptr; // Array structure already stored
	}

	// Non-array types
	llvm::Type *CodeGenTy = Ty->getCodeGen()->getType();

	// The default must match the actual storage SLOT type `T`, not the semantic type.
	// A reference-type field (class / interface) is a pointer slot — its default is a
	// null pointer. Storing the semantic struct's zeroinitializer would write a whole
	// struct into an 8-byte pointer slot and corrupt the adjacent heap.
	if (this->T && this->T->isPointerTy()) {
		return Store(llvm::Constant::getNullValue(this->T));
	}

	// Struct types (non-string) require different defaults depending on storage:
	//   Local variable: Alloca() creates 'alloca ptr' (pointer-to-struct) → default is null ptr
	//   Class attribute: setPointer(GEP) points into the struct body (embedded) → default is zeroinitializer
	if (CodeGenTy->isStructTy() && CodeGenTy != CodeGen::StringTy) {
		llvm::Value *DefaultValue = llvm::isa<llvm::AllocaInst>(this->Pointer)
			? llvm::Constant::getNullValue(llvm::PointerType::getUnqual(CGM->LLVMCtx))
			: llvm::Constant::getNullValue(CodeGenTy);
		return Store(DefaultValue);
	}

	llvm::Value *DefaultValue = getDefaultValue(CodeGenTy);
	return Store(DefaultValue);
}

llvm::LoadInst *CodeGenVar::Load() {
    this->LoadBlock = CGM->Builder->GetInsertBlock();
    // Value-type structs (string, array) load their full struct value.
    // Other struct types (class, error) are stored as a ptr-to-struct; load the pointer.
    llvm::Type *LoadTy = (T->isStructTy() && T != CodeGen::StringTy)
        ? llvm::PointerType::getUnqual(CGM->LLVMCtx)
        : T;
    this->LoadI = CGM->Builder->CreateLoad(LoadTy, getPointer());
    return this->LoadI;
}

llvm::Value *CodeGenVar::getValue() {
    // For class-type const parameters (passed by value in fly's calling convention):
    // StoreParams sets getPointer() to the raw function argument (not an alloca).
    // Load() would dereference through the argument pointer — one level too deep.
    // Instead, return the argument pointer directly: it IS the class instance pointer.
    // ReadOnly is set by StoreParams for const params; non-const params ARE double pointers.
    if (T->isStructTy() && T != CodeGen::StringTy && T != CodeGen::ArrayTy) {
        if (!llvm::isa<llvm::AllocaInst>(this->Pointer) && this->ReadOnly) {
            return this->Pointer;
        }
    }
    if (!this->LoadI || this->LoadBlock != CGM->Builder->GetInsertBlock()) {
        return Load();
    }
    return this->LoadI;
}

void CodeGenVar::resetLoad() {
    this->LoadI = nullptr;
    this->LoadBlock = nullptr;
}

llvm::Value *CodeGenVar::getPointer() {
	// if (Sema->getKind() == SemaKind::MEMBER) { // FIXME
	// 	assert(this->Pointer && "Pointer must be set for MemberVar");
	//
	// 	// If Pointer is not set, create it
	// 	llvm::PointerType *PtrType = llvm::cast<llvm::PointerType>(this->Pointer->getType());
	// 	llvm::ArrayRef<llvm::Value *> IdxList = {CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, Index)};
	// 	this->Pointer = CGM->Builder->CreateInBoundsGEP(PtrType->getElementType(), this->Pointer, IdxList);
	// } else if (Sema->getKind() == SemaKind::ATTRIBUTE) {
	// 	assert(this->Pointer && "Pointer must be set for ClassAttribute");
	// 	SemaClassAttribute * Attribute = static_cast<SemaClassAttribute *>(Sema);
	//
	// 	if (!Attribute->isStatic()) {
	// 		CodeGenVar *CGV = static_cast<SemaClassInstance *>(Attribute->getParent())->getCodeGen();
	// 		llvm::ArrayRef<llvm::Value *> IdxList = {CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, Attribute->getCodeGen()->getIndex())};
	// 		this->Pointer = CGM->Builder->CreateInBoundsGEP(CGV->getType(), CGV->getValue(), IdxList);
	// 	}
	// }
	// else if (Sema->getVarKind() == SemaVarKind::CLASS_INSTANCE) {
	// 	// Check ClassType for setting Index
	// 	SemaClassInstance * This = static_cast<SemaClassInstance *>(Sema);
	//
	// 	if (This->getParent()) {
	// 		CodeGenVarBase *CGV = This->getParent()->getCodeGen();
	// 		llvm::ArrayRef<llvm::Value *> IdxList = {CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, This->getCodeGen()->getIndex())};
	// 		this->Pointer = CGM->Builder->CreateInBoundsGEP(CGV->getType(), CGV->getValue(), IdxList);
	// 	}
	// }

    return this->Pointer;
}

void CodeGenVar::setPointer(llvm::Value *Pointer) {
    this->Pointer = Pointer;
	this->LoadI = nullptr;
}

size_t CodeGenVar::getIndex() {
	return Index;
}

bool CodeGenVar::isReadOnly() const {
    return ReadOnly;
}

void CodeGenVar::setReadOnly(bool ReadOnly) {
    this->ReadOnly = ReadOnly;
}
