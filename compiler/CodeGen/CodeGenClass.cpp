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

	// Phase 1: Create the LLVM struct type and register the CodeGen var for 'this'.
	// This must happen before setCodeGen() is called on Sema (done by the caller
	// in visit(SemaClassType)) so that self-referential return type lookups during
	// Phase 2 (Build()) can find the struct type via getCodeGen()->getType().
	Type = llvm::StructType::create(CGM->LLVMCtx, Id);
	T = Type;
	TypePtr = Type->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());

	CodeGenVar *CGV = new CodeGenVar(CGM, Sema->getThis(), Type);
	Sema->getThis()->setCodeGen(CGV);

	// Create the Init Constructor Function signature (no body yet).
	if (Sema->getClassKind() != SemaClassKind::INTERFACE && !Sema->isAbstract()) {
		CreateInitConstructor();
	}
	// Phase 2 (Build()) must be called by visit(SemaClassType) after setCodeGen().
}

void CodeGenClass::Build() {
	// Phase 2: Complete class code generation. Called after Sema->setCodeGen(this) so
	// that self-referential return types (e.g. pushScope() → SymbolTable) can resolve
	// the CodeGen via getCodeGen()->getType() without triggering infinite recursion.
	CreateVTable();
	CreateBaseInfo(Sema->getBaseClasses());
	CreateAttributes();
	Type->setBody(BodyType);

	if (!IsExternal) {
		// CreateBaseVTables / GenInitConstructorBody need the struct's byte layout,
		// which requires every base subobject to be fully sized. During a cyclic
		// build (a method param type — e.g. accept(ASTVisitor) — transitively
		// re-enters this class while a base is still opaque) the type is not yet
		// sized. Defer those offset-dependent steps to a post-pass (drained at the
		// end of GenerateDeclarations) once all types are complete.
		if (Type->isSized()) {
			FinishBuild();
		} else {
			CGM->DeferredClassFinish.push_back(this);
		}
	}
}

void CodeGenClass::FinishBuild() {
	if (Sema->getClassKind() == SemaClassKind::CLASS) {
		CreateBaseVTables();
	}
	if (Sema->getClassKind() != SemaClassKind::INTERFACE && !Sema->isAbstract()) {
		GenInitConstructorBody();
	}
}

std::string CodeGenClass::toIdentifier(SemaClassType *ClassType) {
	FLY_DEBUG_SCOPE("CodeGenClass", "toIdentifier");
	// Use mangled name for generic specializations (e.g. "List_I" for List<int>).
	llvm::StringRef Name = ClassType->getMangledName().empty()
	    ? ClassType->getAST().getName()
	    : llvm::StringRef(ClassType->getMangledName());
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

		uint64_t BaseByteOffset = Layout->getElementOffset(FieldIdx);
		VTableBases.push_back(NeedsPerBaseVTable
			? BuildPerBaseVTable(Base, BaseByteOffset)
			: nullptr);

		// Also re-point any TRANSITIVE base subobjects (grandparents reached through
		// this base) whose methods this most-derived class overrides.
		CollectTransitiveBaseVTables(Base, BaseByteOffset);

		FieldIdx++;
	}
}

// BuildPerBaseVTable — a per-base vtable for `Base`'s subobject sitting at
// `byteOffset` within this most-derived class. Slot 0 is the offset-to-top
// (-byteOffset); slots 1..N mirror Base's main vtable (same indices), with this
// class's overrides routed through this-adjusting thunks.
llvm::GlobalVariable *CodeGenClass::BuildPerBaseVTable(SemaClassType *Base, uint64_t byteOffset) {
	llvm::SmallVector<llvm::Constant *, 8> VTableValues;

	llvm::ConstantInt *OffsetInt = llvm::ConstantInt::get(CodeGen::Int64Ty, -(int64_t)byteOffset);
	VTableValues.push_back(llvm::ConstantExpr::getIntToPtr(OffsetInt, CodeGen::Int8PtrTy));

	size_t MethodIdx = 1;
	for (auto &Node : Base->getNodes()) {
		if (Node->getKind() != SemaKind::METHOD) continue;
		SemaClassMethod *BaseMethod = static_cast<SemaClassMethod *>(Node);

		// Interface methods never get a CodeGen from CreateVTable() — create here.
		if (BaseMethod->getCodeGen() == nullptr) {
			if (Base->getCodeGen() == nullptr) {
				MethodIdx++;
				continue;
			}
			CodeGenClassMethod *CG = new CodeGenClassMethod(
				CGM, BaseMethod, Base->getCodeGen()->getType(), MethodIdx);
			BaseMethod->setCodeGen(CG);
		}

		// Ctor/static slots exist only for index alignment; null is safe (never
		// dispatched, and a synthesized default ctor may be declared-but-undefined).
		if (BaseMethod->isConstructor() || BaseMethod->isStatic()) {
			VTableValues.push_back(llvm::Constant::getNullValue(CodeGen::Int8PtrTy));
			MethodIdx++;
			continue;
		}

		SemaClassMethod *Override = FindOverrideInDerived(Sema, BaseMethod);
		llvm::Constant *Slot;
		if (Override && Override->getCodeGen() && Override->getCodeGen()->getFunction()) {
			llvm::Function *Thunk = CreateThunk(BaseMethod, Override, byteOffset);
			Slot = llvm::ConstantExpr::getBitCast(Thunk, CodeGen::Int8PtrTy);
		} else if (BaseMethod->getCodeGen() && BaseMethod->getCodeGen()->getFunction()) {
			Slot = llvm::ConstantExpr::getBitCast(BaseMethod->getCodeGen()->getFunction(), CodeGen::Int8PtrTy);
		} else {
			Slot = llvm::Constant::getNullValue(CodeGen::Int8PtrTy);
		}
		VTableValues.push_back(Slot);
		MethodIdx++;
	}

	llvm::ArrayType *ArrayTy = llvm::ArrayType::get(CodeGen::Int8PtrTy, VTableValues.size());
	llvm::Constant *ArrayVal = llvm::ConstantArray::get(ArrayTy, VTableValues);
	std::string BaseVTableName = "vtable." + Id + "." + Base->getAST().getName().str();
	// Disambiguate the same base appearing at multiple offsets (e.g. diamond).
	if (CGM->getModule()->getNamedGlobal(BaseVTableName))
		BaseVTableName += "." + std::to_string(byteOffset);
	return new llvm::GlobalVariable(
		*CGM->Module, ArrayTy, true,
		llvm::GlobalValue::ExternalLinkage, ArrayVal, BaseVTableName);
}

// CollectTransitiveBaseVTables — recurse into Base's own bases. For any whose
// methods this most-derived class overrides, build a per-base vtable and record
// it (by byte offset within this class) so the constructor can re-point the
// nested subobject's vptr to this class's override.
void CodeGenClass::CollectTransitiveBaseVTables(SemaClassType *Base, uint64_t baseOffsetInDerived) {
	if (Base->getCodeGen() == nullptr) return;
	llvm::StructType *BaseTy = Base->getCodeGen()->getType();
	// The base's struct body must be complete to compute nested subobject offsets.
	if (!BaseTy->isSized()) return;
	const llvm::StructLayout *BaseLayout = CGM->Module->getDataLayout().getStructLayout(BaseTy);

	size_t FieldIdx = 1; // field 0 is Base's own vptr; its base subobjects start at 1
	for (auto &BB : Base->getBaseClasses()) {
		if (BB->getClassKind() == SemaClassKind::STRUCT) { FieldIdx++; continue; }
		uint64_t BBOffset = baseOffsetInDerived + BaseLayout->getElementOffset(FieldIdx);

		bool Overrides = false;
		for (auto &Node : BB->getNodes()) {
			if (Node->getKind() != SemaKind::METHOD) continue;
			SemaClassMethod *M = static_cast<SemaClassMethod *>(Node);
			if (M->isConstructor() || M->isStatic()) continue;
			if (FindOverrideInDerived(Sema, M) != nullptr) { Overrides = true; break; }
		}
		if (Overrides)
			ExtraBaseVTables.push_back({BBOffset, BuildPerBaseVTable(BB, BBOffset)});

		CollectTransitiveBaseVTables(BB, BBOffset);
		FieldIdx++;
	}
}

llvm::Function *CodeGenClass::CreateThunk(SemaClassMethod *BaseMethod, SemaClassMethod *Override,
                                           uint64_t BaseOffset) {
	CodeGenClassMethod *BaseCG = BaseMethod->getCodeGen();
	llvm::FunctionType *ThunkType = BaseCG->getFunctionType();

	// Name: thunk.DerivedName.BaseName.methodName.vtableIndex
	// The trailing vtable index disambiguates OVERLOADED methods that share a
	// name (e.g. a visitor's many visit(...) overloads): without it the
	// name-based de-dup below would collapse every overload's thunk into the
	// first one, so all overloads would wrongly dispatch to a single override.
	std::string ThunkName = "thunk." + Id + "." +
		BaseMethod->getClass()->getAST().getName().str() + "." +
		BaseMethod->getName().str() + "." + std::to_string(BaseCG->getIndex());

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
		// Walk the override chain: Method overrides BaseMethod if BaseMethod
		// appears anywhere in its getOverridden() chain. A single-link check
		// misses skip-level overrides where an intermediate class also overrides
		// the method (e.g. D:C:B:A with D and C both overriding A's method —
		// D.who.getOverridden() is C.who, not A.who).
		for (SemaClassMethod *O = Method->getOverridden(); O != nullptr; O = O->getOverridden()) {
			if (O == BaseMethod) {
				return Method;
			}
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

			// Reference-type fields (class / interface) are heap pointers, NOT embedded
			// structs: embedding a struct that (transitively) contains its own type makes
			// an infinitely-sized/cyclic LLVM struct (StructType::isSized stack overflow).
			// Only STRUCT-kind types stay embedded by value (Fly value semantics).
			if (Attribute->getType()->isClass() &&
			    static_cast<SemaClassType *>(Attribute->getType())->getClassKind() != SemaClassKind::STRUCT) {
				AttrType = llvm::PointerType::getUnqual(CGM->LLVMCtx);
			}

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

			// Store per-base vtable pointer into the base subobject's vtable slot.
			// The vtable pointer is field 0 of the base subobject, i.e. at offset 0,
			// so BaseSubobjPtr already addresses it — store directly instead of a
			// StructGEP through Base's struct type, which may still be incomplete
			// here (e.g. a base class generated after this derived constructor).
			if (I < VTableBases.size() && VTableBases[I] != nullptr) {
				llvm::Value *BaseVTableCast = CGM->Builder->CreateBitCast(
					VTableBases[I], CodeGen::Int8PtrPtrTy);
				CGM->Builder->CreateStore(BaseVTableCast, BaseSubobjPtr);
			}

			BaseIndex++;
		}

		// Re-point any TRANSITIVE base subobjects this class overrides methods of:
		// the direct bases' init_ctor (above) set their nested vptrs to their OWN
		// views, so overwrite them here with this class's per-base vtables. The vptr
		// is the first field of each subobject, i.e. at the subobject's byte offset.
		for (auto &Entry : ExtraBaseVTables) {
			llvm::Value *I8This = CGM->Builder->CreateBitCast(Load, CodeGen::Int8PtrTy);
			llvm::Value *FieldPtr = CGM->Builder->CreateGEP(
				llvm::Type::getInt8Ty(CGM->LLVMCtx), I8This,
				llvm::ConstantInt::get(CodeGen::Int64Ty, Entry.first));
			llvm::Value *VTCast = CGM->Builder->CreateBitCast(Entry.second, CodeGen::Int8PtrPtrTy);
			CGM->Builder->CreateStore(VTCast, FieldPtr);
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
	// CLASS/INTERFACE: field 0 is the vtable pointer, base subobjects start at 1.
	// STRUCT: no vtable, base subobjects start at 0.
	size_t BaseIndex = (Sema->getClassKind() == SemaClassKind::STRUCT) ? 0 : 1;
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


