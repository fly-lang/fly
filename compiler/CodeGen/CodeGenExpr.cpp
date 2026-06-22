//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenExpr.cpp - expression code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenExpr.h"
#include "CodeGen/CodeGenStdLibCLang.h"
#include "CodeGen/CodeGenStdLibLLVM.h"
#include "CodeGen/CodeGenStdLibRuntime.h"

#include "AST/ASTBinary.h"
#include "AST/ASTCast.h"
#include "AST/ASTExpr.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTModule.h"
#include "AST/ASTTernary.h"
#include "Sema/SemaModule.h"
#include "AST/ASTType.h"
#include "AST/ASTUnary.h"
#include "Basic/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenHelper.h"
#include "Sema/SemaBinary.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaCast.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaEnumEntry.h"
#include "Sema/SemaEnumAccessor.h"
#include "Sema/SemaEnumType.h"
#include "CodeGen/CodeGenEnum.h"
#include "Sema/SemaTernary.h"
#include "Sema/SemaUnary.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/GlobalVariable.h"

#include <AST/ASTArg.h>
#include <AST/ASTCall.h>
#include <CodeGen/CodeGenClass.h>
#include <CodeGen/CodeGenFunctionBase.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaClassMethod.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaError.h>
#include <Sema/SemaFunctionBase.h>
#include <Sema/SemaCall.h>
#include <Sema/SemaMember.h>
#include <Sema/SemaParam.h>
#include <Sema/SemaType.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaVar.h>

using namespace fly;

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM) : CodeGenBase(), CGM(CGM), Builder(CGM->getBuilder()) {
	FLY_DEBUG_SCOPE("CodeGenExpr", "CodeGenExpr");
}

llvm::Value *CodeGenExpr::getValue() {
	return V;
}

void CodeGenExpr::GenExpr(SemaBoolValue *Sema) {
	V = Sema->getValue() ?
		llvm::ConstantInt::getTrue(CGM->LLVMCtx) : llvm::ConstantInt::getFalse(CGM->LLVMCtx);
}

void CodeGenExpr::GenExpr(SemaIntValue *Sema) {
	SemaType *Type = Sema->getType();
	// Even though the AST node is SemaIntValue (the literal has no decimal point in source),
	// PromoteTypes may have rewritten its type to float to satisfy mixed-type arithmetic
	// (e.g. "1 + 2.5" promotes the integer literal 1 to float before codegen).
	// SemaIntValue records the literal's lexical form, not its final expression type,
	// so we must consult getType() here rather than assuming the type is always integral.
	if (Type->isFloat()) {
		double FPVal = (double)Sema->getValue().getZExtValue();
		switch (static_cast<SemaFloatType *>(Type)->getFloatKind()) {
			case SemaFloatTypeKind::TYPE_FLOAT:
				V = llvm::ConstantFP::get(CodeGen::FloatTy, FPVal); break;
			case SemaFloatTypeKind::TYPE_DOUBLE:
				V = llvm::ConstantFP::get(CodeGen::DoubleTy, FPVal); break;
		}
		return;
	}
	SemaIntTypeKind IntKind = static_cast<SemaIntType *>(Type)->getIntKind();
	switch (IntKind) {
		case SemaIntTypeKind::TYPE_BYTE:
			V = llvm::ConstantInt::get(CGM->LLVMCtx, Sema->getValue().trunc(8));
			break;
		case SemaIntTypeKind::TYPE_USHORT:
		case SemaIntTypeKind::TYPE_SHORT:
			V = llvm::ConstantInt::get(CGM->LLVMCtx, Sema->getValue().trunc(16));
			break;
		case SemaIntTypeKind::TYPE_UINT:
		case SemaIntTypeKind::TYPE_INT:
			V = llvm::ConstantInt::get(CGM->LLVMCtx, Sema->getValue().trunc(32));
			break;
		case SemaIntTypeKind::TYPE_ULONG:
		case SemaIntTypeKind::TYPE_LONG:
			V = llvm::ConstantInt::get(CGM->LLVMCtx, Sema->getValue().trunc(64));
			break;
	}
}

void CodeGenExpr::GenExpr(SemaFloatValue *Sema) {
	SemaType *Type = Sema->getType();
	SemaFloatTypeKind FPKind = static_cast<SemaFloatType *>(Type)->getFloatKind();
	switch (FPKind) {
		case SemaFloatTypeKind::TYPE_FLOAT:
			V = llvm::ConstantFP::get(CodeGen::FloatTy, Sema->getValue());
			break;
		case SemaFloatTypeKind::TYPE_DOUBLE:
			V = llvm::ConstantFP::get(CodeGen::DoubleTy, Sema->getValue());
			break;
	}
}

void CodeGenExpr::GenExpr(SemaComplexValue *Sema) {
	llvm::Constant *Real = llvm::ConstantFP::get(CodeGen::DoubleTy, Sema->getReal());
	llvm::Constant *Imag = llvm::ConstantFP::get(CodeGen::DoubleTy, Sema->getImag());
	V = llvm::ConstantStruct::get(CodeGen::ComplexTy, {Real, Imag});
}

void CodeGenExpr::GenExpr(SemaStringValue *Sema) {
	llvm::Constant *Ptr = Builder->CreateGlobalStringPtr(Sema->getValue());
	llvm::Constant *Size = llvm::ConstantInt::get(CodeGen::Int32Ty, Sema->getValue().size());
	V = llvm::ConstantStruct::get(CodeGen::StringTy, {Ptr, Size});
}

void CodeGenExpr::GenExpr(SemaStructValue *Sema) {
	SemaType *Type = Sema->getType();
	if (Type && Type->isClass()) {
		Type->accept(*CGM);
		SemaClassType *ClassType = static_cast<SemaClassType *>(Type);
		llvm::StructType *StructLLVMType = ClassType->getCodeGen()->getType();
		if (StructLLVMType) {
			llvm::AllocaInst *Alloca = Builder->CreateAlloca(StructLLVMType);
			uint64_t StructSize = CGM->Module->getDataLayout().getTypeAllocSize(StructLLVMType);
			Builder->CreateMemSet(Alloca, llvm::ConstantInt::get(CodeGen::Int8Ty, 0),
				StructSize, llvm::MaybeAlign());
			for (auto &Entry : Sema->getValues()) {
				llvm::StringRef FieldName = Entry.getKey();
				SemaValue *FieldVal = Entry.getValue();
				SemaClassAttribute *Attr = ClassType->LookupAttribute(FieldName);
				if (Attr && Attr->getCodeGen()) {
					size_t FieldIdx = Attr->getCodeGen()->getIndex();
					llvm::Value *FieldPtr = Builder->CreateStructGEP(StructLLVMType, Alloca, FieldIdx);
					FieldVal->accept(*CGM);
					llvm::Value *Val = FieldVal->getCodeGen()->getValue();
					Builder->CreateStore(Val, FieldPtr);
				}
			}
			V = Alloca;
			return;
		}
	}
	V = llvm::ConstantPointerNull::get(CodeGen::VoidPtrTy);
}

void CodeGenExpr::GenExpr(SemaNullValue *Sema) {
	SemaType *Type = Sema->getType();
	if (Type) {
		Type->accept(*CGM);
		llvm::Type *LLVMType = Type->getCodeGen() ? Type->getCodeGen()->getType() : nullptr;
		if (LLVMType) {
			if (LLVMType->isPointerTy()) {
				V = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(LLVMType));
				return;
			}
			if (LLVMType->isStructTy()) {
				V = llvm::ConstantPointerNull::get(LLVMType->getPointerTo());
				return;
			}
		}
	}
	V = llvm::ConstantPointerNull::get(CodeGen::VoidPtrTy);
}

void CodeGenExpr::GenExpr(SemaUnsetValue *Sema) {
	// Generate a constant integer 0 for unset enum value
	V = llvm::ConstantInt::get(CodeGen::Int32Ty, 0);
}

void CodeGenExpr::GenExpr(SemaEnumEntry *Sema) {
	if (Sema->getCodeGen()) {
		V = Sema->getCodeGen()->getValue();
	} else {
		V = llvm::ConstantInt::get(CodeGen::Int32Ty, Sema->getIndex());
	}
}

void CodeGenExpr::GenExpr(SemaEnumAccessor *Sema) {
	SemaEnumEntry *Entry = Sema->getEntry();

	// ── .value → i32 ──────────────────────────────────────────────────────────
	if (!Sema->isName()) {
		if (Entry) {
			// Literal entry: fold to its constant index.
			V = llvm::ConstantInt::get(CodeGen::Int32Ty, Entry->getIndex());
		} else {
			// Variable: the enum *is* its i32 value.
			SemaVar *Var = Sema->getVar();
			Var->accept(*CGM);
			V = Var->getCodeGen()->getValue();
		}
		return;
	}

	// ── .name → String struct {i8*, i32} ──────────────────────────────────────
	if (Entry) {
		// Literal entry: fold to a compile-time string constant.
		llvm::StringRef Name = Entry->getName();
		llvm::Constant *Ptr = Builder->CreateGlobalStringPtr(Name);
		llvm::Constant *Size = llvm::ConstantInt::get(CodeGen::Int32Ty, Name.size());
		V = llvm::ConstantStruct::get(CodeGen::StringTy, {Ptr, Size});
		return;
	}

	// Variable: load the name from the enum's runtime names table, indexed by
	// the variable's i32 value.
	SemaVar *Var = Sema->getVar();
	Var->accept(*CGM);
	llvm::Value *Idx = Var->getCodeGen()->getValue();

	SemaEnumType *EnumType = Sema->getEnumType();
	EnumType->accept(*CGM); // ensure the CodeGenEnum exists
	CodeGenEnum *CGE = static_cast<CodeGenEnum *>(EnumType->getCodeGen());
	llvm::GlobalVariable *Table = CGE->getNamesTable();

	llvm::Value *ElemPtr = Builder->CreateInBoundsGEP(
		Table->getValueType(), Table, {CodeGen::Zero, Idx});
	V = Builder->CreateLoad(CodeGen::StringTy, ElemPtr);
}

void CodeGenExpr::GenExpr(SemaVar *Sema) {

	// Class Instance
	if (Sema->getKind() == SemaKind::INSTANCE_VAR) {
		SemaClassInstance *Instance = static_cast<SemaClassInstance *>(Sema);

		// Check if the Instance belong to a base class, need to set the pointer to the main instance
		if (Instance->getParent()) {
			Instance->getCodeGen()->setPointer(Instance->getParent()->getCodeGen()->getPointer());
		}
	}

	// Class Attribute
	else if (Sema->getKind() == SemaKind::ATTRIBUTE) {
		SemaClassAttribute * ClassAttribute = static_cast<SemaClassAttribute *>(Sema);

		// Check if the ClassAttribute is a static attribute
		if (!ClassAttribute->isStatic()) {
			SemaClassType &DeclClass = ClassAttribute->getClass();
			llvm::Value *Instance = DeclClass.getThis()->getCodeGen()->getValue();
			// For an INHERITED field, start from the current method's `this` (the
			// most-derived instance) and navigate the FULL base-subobject chain to the
			// declaring class (recursively, for grandparent+ fields). The declaring
			// base class's getThis() is only valid for the current class itself.
			if (CGM->CurrentFunction && CGM->CurrentFunction->getKind() == SemaKind::METHOD) {
				SemaClassMethod *M = static_cast<SemaClassMethod *>(CGM->CurrentFunction);
				if (!M->isStatic() && M->getClass()) {
					llvm::Value *ThisVal = M->getThis()->getCodeGen()->getValue();
					if (M->getClass() != &DeclClass) {
						llvm::Value *Adj = M->getClass()->getCodeGen()->getBaseInstance(ThisVal, &DeclClass);
						Instance = Adj ? Adj : ThisVal;
					} else {
						Instance = ThisVal;
					}
				}
			}
			llvm::StructType *StructTy = DeclClass.getCodeGen()->getType();
			size_t FieldIdx = ClassAttribute->getCodeGen()->getIndex();
			llvm::Value *FieldPtr = Builder->CreateInBoundsGEP(StructTy, Instance,
				{CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, FieldIdx)});
			ClassAttribute->getCodeGen()->setPointer(FieldPtr);
		}
	}

	// No Parent
	else {
		Sema->accept(*CGM);
	}

	V = Sema->getCodeGen()->getValue();
}


void CodeGenExpr::GenExpr(SemaCall *Sema) {

    // The function arguments
    llvm::SmallVector<llvm::Value *, 8> Args;

	// This is the new instance pointer if this is a class method call
	llvm::Value *InstancePtr = nullptr;

    // Add error as first parameter
    if (Sema->getFunction()->getKind() == SemaKind::METHOD &&
    	!static_cast<SemaClassMethod *>(Sema->getFunction())->isStatic()) {

    	// Check is Constructor
        SemaClassMethod *Method = static_cast<SemaClassMethod *>(Sema->getFunction());

    	// Add Error parameter if the class is a not a Struct
        if (Method->getClass()->getClassKind() != SemaClassKind::STRUCT) {
            // Use the call's dedicated error handler if present, otherwise fall back to the
            // current function's error handler (set by CodeGenFunction::GenBody)
            CodeGenError *EH = (Sema->getErrorHandler() && Sema->getErrorHandler()->getCodeGen())
                                    ? Sema->getErrorHandler()->getCodeGen()
                                    : CGM->CurrentErrorHandler;
            if (EH)
                Args.push_back(EH->getValue()); // Error is a Pointer
        }

    	// Get Class CodeGen
    	CodeGenClass * CGClass = Method->getClass()->getCodeGen();

		// Call class constructor
    	if (Method->isConstructor()) {

    		// Allocate memory for the new instance in InstancePtr
    		if (Sema->getAST().getCallKind() == ASTCallKind::CALL_NEW ||
    			Sema->getAST().getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
    			Sema->getAST().getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
    			Sema->getAST().getCallKind() == ASTCallKind::CALL_NEW_WEAK) {
    			if (Sema->getAST().getCallKind() == ASTCallKind::CALL_NEW_SHARED) {
    				// Shared alloc: [i64 refcount | T data] in one malloc block.
    				// The class only knows about its own layout; the refcount header
    				// is a smart-alloc concern handled here in CodeGenExpr.
    				llvm::Type *PtrSizedIntTy = CGM->Module->getDataLayout().getIntPtrType(CGM->LLVMCtx);
    				llvm::Type *I64Ty = llvm::Type::getInt64Ty(CGM->LLVMCtx);
    				llvm::Type *I8Ty  = llvm::Type::getInt8Ty(CGM->LLVMCtx);
    				llvm::StructType *WrapperTy = llvm::StructType::get(
    					CGM->LLVMCtx, {I64Ty, CGClass->getType()});
    				llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
    					"malloc",
    					llvm::FunctionType::get(
    						llvm::PointerType::getUnqual(CGM->LLVMCtx), {PtrSizedIntTy}, false));
    				llvm::Value *RawPtr = Builder->CreateCall(MallocFn,
    					{Builder->CreateIntCast(
    						llvm::ConstantExpr::getSizeOf(WrapperTy), PtrSizedIntTy, false)});
    				Builder->CreateStore(llvm::ConstantInt::get(I64Ty, 1), RawPtr);
    				InstancePtr = Builder->CreateGEP(I8Ty, RawPtr,
    					llvm::ConstantInt::get(PtrSizedIntTy, 8));
    				InstancePtr = Builder->CreateCall(CGClass->getInitConstructor(), {InstancePtr});
    			} else {
    				// Plain `new` for STRUCT → stack-allocate; no heap involved.
    				// Smart-pointer variants (unique/weak) for STRUCT still use heap because the
    				// runtime will call free() on scope exit.
    				bool isStackStruct =
    					Method->getClass()->getClassKind() == SemaClassKind::STRUCT &&
    					Sema->getAST().getCallKind() == ASTCallKind::CALL_NEW;
    				if (isStackStruct) {
    					llvm::AllocaInst *StackPtr = Builder->CreateAlloca(CGClass->getType());
    					InstancePtr = Builder->CreateCall(CGClass->getInitConstructor(), {StackPtr});
    				} else {
    					// CLASS/INTERFACE or smart-alloc variant: malloc(sizeof(T)) + init_ctor.
    					llvm::Type *PtrSizedIntTy = CGM->Module->getDataLayout().getIntPtrType(CGM->LLVMCtx);
    					llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
    						"malloc",
    						llvm::FunctionType::get(
    							llvm::PointerType::getUnqual(CGM->LLVMCtx), {PtrSizedIntTy}, false));
    					llvm::Value *RawPtr = Builder->CreateCall(MallocFn,
    						{Builder->CreateIntCast(
    							llvm::ConstantExpr::getSizeOf(CGClass->getType()), PtrSizedIntTy, false)});
    					InstancePtr = Builder->CreateCall(CGClass->getInitConstructor(), {RawPtr});
    				}
    			}

    		} else if (Sema->isSuper()) {
    			// super(...): call the base constructor on the BASE SUBOBJECT of the
    			// enclosing (derived) `this`. Method->getClass() is the base class.
    			SemaClassMethod *Enclosing = static_cast<SemaClassMethod *>(CGM->CurrentFunction);
    			llvm::Value *DerivedThis = Enclosing->getThis()->getCodeGen()->getValue();
    			InstancePtr = Enclosing->getClass()->getCodeGen()->getBaseInstance(
    				DerivedThis, Method->getClass());
    		} else {
			    // this is a constructor without new
    			// You are inside a class method call
    			// Polymorphism: Take the right This Instance for the constructor of the base class
    			InstancePtr = Method->getThis()->getCodeGen()->getValue();
    		}

    		// Add Instance parameter
    		Args.push_back(InstancePtr);
    		addArgs(Sema, Args);

    		// Call the Class constructor
    		if (Method->getClass()->getClassKind() != SemaClassKind::STRUCT)
    			Builder->CreateCall(Method->getCodeGen()->getFunction(), Args);

    		V = InstancePtr;

    		// fly.bridge.CLang: capture lib literal into CLangLibMap after allocation
    		if (Method->getClass()->getName() == "CLang" &&
    		    CodeGenHelper::FlattenNS(Method->getClass()->getModule().getAST().getNameSpace()) == "fly_bridge") {
    		    CodeGenStdLibCLang(CGM, Builder, V).GenConstructorCapture(Sema, InstancePtr);
    		}

    		return;
    	}

    	// Call is not a constructor, so it must be a method call

    	// fly.bridge.CLang::call() — intercept before vtable dispatch
    	if (Method->getName() == "call" &&
    	    Method->getClass()->getName() == "CLang" &&
    	    CodeGenHelper::FlattenNS(Method->getClass()->getModule().getAST().getNameSpace()) == "fly_bridge") {
    	    CodeGenStdLibCLang(CGM, Builder, V).GenBridgeMethodCall(Sema);
    	    return;
    	}

    	if (Sema->getParent()) {

    		SemaClassType * ParentClass = static_cast<SemaClassType *>(Sema->getParent()->getType());
    		Sema->getParent()->accept(*CGM);
    		InstancePtr = Sema->getParent()->getCodeGen()->getValue();

    		// Inherited method (declared in a base, possibly a grandparent+): navigate the
    		// FULL base-subobject chain so the callee gets the right `this`. getBaseInstance
    		// recurses; a single-level isBase() check would miss grandparent methods.
    		if (ParentClass && Method->getClass() != ParentClass) {
    			llvm::Value *Adj = ParentClass->getCodeGen()->getBaseInstance(InstancePtr, Method->getClass());
    			if (Adj) InstancePtr = Adj;
    		}

    	} else {
    		// Direct method call inside a class (implicit `this` or super.method()). Start
    		// from the current method's `this` and navigate to the called method's declaring
    		// class. Do NOT eagerly evaluate the called class's getThis() — for a super call
    		// the called class is a base, whose `this` alloca lives in another function, and
    		// emitting its load here would leave a cross-function reference in this body.
    		SemaClassMethod *Cur = (CGM->CurrentFunction && CGM->CurrentFunction->getKind() == SemaKind::METHOD)
    			? static_cast<SemaClassMethod *>(CGM->CurrentFunction) : nullptr;
    		if (Cur && !Cur->isStatic() && Cur->getClass()) {
    			llvm::Value *ThisVal = Cur->getThis()->getCodeGen()->getValue();
    			if (Cur->getClass() != Method->getClass()) {
    				llvm::Value *Adj = Cur->getClass()->getCodeGen()->getBaseInstance(ThisVal, Method->getClass());
    				InstancePtr = Adj ? Adj : ThisVal;
    			} else {
    				InstancePtr = ThisVal;
    			}
    		} else {
    			InstancePtr = Method->getClass()->getThis()->getCodeGen()->getValue();
    		}
    	}

    	// Add Instance parameter
    	Args.push_back(InstancePtr);
    	addArgs(Sema, Args);

    	// Create the function pointer by vtable is polymorphic
    	// or by the function pointer if is static
    	llvm::Value * FuncPtr;
    	if (Sema->getParent()) {

    		// Get the VTable pointer
    		// %as_base1 = bitcast %class.Derived* %d to %class.Base1*
    		// %vptr1_ptr = getelementptr %class.Base1, %class.Base1* %as_base1, i32 0, i32 0
    		// %vptr1 = load i8**, i8*** %vptr1_ptr
    		llvm::Value * VTablePtrPtr = Builder->CreateStructGEP(CGClass->getType(), InstancePtr, 0);
    		llvm::LoadInst * VTablePtr = Builder->CreateLoad(CodeGen::Int8PtrPtrTy, VTablePtrPtr);

    		// TODO
    		// Calculate the offset of the method in the VTable

    		// Get the Method index in the VTable
    		// %fn1_ptr = getelementptr i8*, i8** %vptr1, i64 1
    		// %fn1_i8 = load i8*, i8** %fn1_ptr
    		// %fn1 = bitcast i8* %fn1_i8 to void (%class.Base1*)*
    		llvm::Value * FuncPtrPtr = Builder->CreateGEP(CodeGen::Int8PtrTy, VTablePtr, llvm::ConstantInt::get(CodeGen::Int64Ty, Method->getCodeGen()->getIndex()));
    		FuncPtr = Builder->CreateLoad(CodeGen::Int8PtrTy, FuncPtrPtr);
    		// With opaque pointers, no bitcast needed — the loaded ptr is already the right type

    	} else {
    		FuncPtr = Method->getCodeGen()->getFunction();
    	}

    	// Create the function call
    	V = Builder->CreateCall(Method->getCodeGen()->getFunctionType(), FuncPtr, Args);

    	// If the method has a return type, load the result from the hidden out variable.
    	if (Sema->getOutVar() != nullptr) {
    		SemaLocalVar *OutVar = Sema->getOutVar();
    		OutVar->getType()->accept(*CGM);
    		llvm::Type *OutTy = OutVar->getType()->getCodeGen()->getType();
    		// For struct out-params the alloca holds a heap ptr (pre-allocated in addArgs).
    		// Load the ptr so the result is a File*/Stat*/... compatible with field access.
    		// Covers STRUCT, CLASS, and INTERFACE kinds — all use alloca ptr convention.
    		bool isFlyStruct = OutTy->isStructTy() &&
    		    OutTy != CodeGen::StringTy && OutTy != CodeGen::ArrayTy &&
    		    OutVar->getType()->getKind() == SemaKind::TYPE_CLASS;
    		if (isFlyStruct)
    			V = Builder->CreateLoad(llvm::PointerType::getUnqual(CGM->LLVMCtx),
    			                        OutVar->getCodeGen()->getPointer());
    		else
    			V = Builder->CreateLoad(OutTy, OutVar->getCodeGen()->getPointer());
    	}

    } else {

        // fly.llvm → emit LLVM intrinsics directly (no err_ctx, no mangling).
        // fly.runtime → call C symbols by exact name (no err_ctx, no mangling).
        const std::string &NS = Sema->getFunction()->getNamespaceName();
        if (NS == "fly_llvm")    { CodeGenStdLibLLVM(CGM, Builder, V).GenCall(Sema);    return; }
        if (NS == "fly_runtime") { CodeGenStdLibRuntime(CGM, Builder, V).GenCall(Sema); return; }

    	// Add Error parameter
        Args.push_back(CGM->CurrentErrorHandler->getValue()); // Error is a Pointer
    	addArgs(Sema, Args);

    	// External function (declared in .fly.h header, implemented in runtime)
    	if (Sema->getFunction()->getCodeGen() == nullptr) {
    		std::string MangledName = CodeGenHelper::Mangle(Sema->getFunction());

    		// Build LLVM param types: void* error + all params by pointer
    		llvm::SmallVector<llvm::Type *, 8> ParamTypes;
    		ParamTypes.push_back(CodeGen::VoidPtrTy);
    		CodeGenFunctionBase::GenParamTypes(CGM, ParamTypes, Sema->getFunction());

    		// Functions with a declared return type use the out-param convention:
    		// the hidden 'out' pointer is already in ParamTypes; LLVM return type is void.
    		llvm::Type *LLVMRetTy = CodeGen::VoidTy;
    		if (Sema->getOutVar() == nullptr) {
    			SemaType *RetSema = Sema->getFunction()->getReturnType();
    			RetSema->accept(*CGM);
    			LLVMRetTy = RetSema->getCodeGen()->getType();
    		}

    		llvm::FunctionType *FnTy = llvm::FunctionType::get(LLVMRetTy, ParamTypes, false);
    		llvm::FunctionCallee Callee = CGM->Module->getOrInsertFunction(MangledName, FnTy);
    		V = Builder->CreateCall(Callee, Args);

    		if (Sema->getOutVar() != nullptr) {
    			SemaLocalVar *OutVar = Sema->getOutVar();
    			OutVar->getType()->accept(*CGM);
    			llvm::Type *OutTy = OutVar->getType()->getCodeGen()->getType();
    			bool isFlyStruct = OutTy->isStructTy() &&
    			    OutTy != CodeGen::StringTy && OutTy != CodeGen::ArrayTy &&
    			    OutVar->getType()->getKind() == SemaKind::TYPE_CLASS;
    			if (isFlyStruct)
    				V = Builder->CreateLoad(llvm::PointerType::getUnqual(CGM->LLVMCtx),
    				                        OutVar->getCodeGen()->getPointer());
    			else
    				V = Builder->CreateLoad(OutTy, OutVar->getCodeGen()->getPointer());
    		}
    		return;
    	}

    	llvm::Function *Fn = Sema->getFunction()->getCodeGen()->getFunction();
    	V = Builder->CreateCall(Fn, Args);
    	if (Sema->getOutVar() != nullptr) {
    		SemaLocalVar *OutVar = Sema->getOutVar();
    		OutVar->getType()->accept(*CGM);
    		llvm::Type *OutTy = OutVar->getType()->getCodeGen()->getType();
    		bool isFlyStruct = OutTy->isStructTy() &&
    		    OutTy != CodeGen::StringTy && OutTy != CodeGen::ArrayTy &&
    		    OutVar->getType()->getKind() == SemaKind::TYPE_CLASS;
    		if (isFlyStruct)
    			V = Builder->CreateLoad(llvm::PointerType::getUnqual(CGM->LLVMCtx),
    			                        OutVar->getCodeGen()->getPointer());
    		else
    			V = Builder->CreateLoad(OutTy, OutVar->getCodeGen()->getPointer());
    	}
    }
}

void CodeGenExpr::GenExpr(SemaMember *Sema) {
SemaExpr *Ref = Sema->getRef();
	if (Ref->getKind() == SemaKind::ATTRIBUTE) {
		SemaClassAttribute *Attr = static_cast<SemaClassAttribute *>(Ref);

		// Static attributes are backed by a GlobalVariable — use it directly,
		// no need to generate the parent or compute a GEP.
		if (Attr->isStatic()) {
			Sema->setCodeGen(Attr->getCodeGen());
			V = Attr->getCodeGen()->getPointer();
			return;
		}

		// Non-static: Generate Parent and create GEP into the struct instance.
		Sema->getParent()->accept(*CGM);
		llvm::Value *InstancePtr = Sema->getParent()->getCodeGen()->getValue();

		Sema->getType()->accept(*CGM);
		llvm::Type *Ty = Sema->getType()->getCodeGen()->getType();
		size_t Index = Attr->getCodeGen()->getIndex();

		// Feature 5b: if the attribute belongs to a base class but the parent is typed
		// as the derived class, navigate through the embedded base subobject(s) first.
		// Use the recursive getBaseInstance so grandparent+ fields work (it walks the
		// full chain), instead of looking only in the parent's direct base list.
		SemaClassType &AttrClass = Attr->getClass();
		SemaType *ParentType = Sema->getParent()->getType();
		if (ParentType && ParentType->isClass()) {
			SemaClassType *ParentClass = static_cast<SemaClassType *>(ParentType);
			if (ParentClass != &AttrClass) {
				llvm::Value *Adj = ParentClass->getCodeGen()->getBaseInstance(InstancePtr, &AttrClass);
				if (Adj) InstancePtr = Adj;
			}
		}

		// Create GEP to get pointer to the specific field in the struct
		AttrClass.getCodeGen()->getType(); // ensure class CodeGen type is created
		llvm::StructType *StructTy = AttrClass.getCodeGen()->getType();
		/** Fixed
		* llvm::ArrayRef<llvm::Value *> IdxList = {
		*	CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, Index)
		* };
		*  IdxList is initialized from a std::initializer_list
		*  and stored as an ArrayRef, but the initializer list's backing array is destroyed at the end of that
		*  statement, leaving IdxList.Data dangling. In a Release -O3 build, the stack memory gets reused and the
		*  second element reads as null.
		**/
		llvm::Value *FieldPtr = Builder->CreateInBoundsGEP(StructTy, InstancePtr,
			{CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, Index)});

		CodeGenVar *CGV = new CodeGenVar(CGM, Attr, Ty, Index);
		CGV->setPointer(FieldPtr);
		Sema->setCodeGen(CGV);
		// Do NOT load the value here – let callers decide (getValue vs Store).
		// For rvalue usage, getValue() will generate the load on demand.
		V = FieldPtr; // expose the raw pointer as V for internal use
	} else if (Ref->getKind() == SemaKind::ENUM_ENTRY) {
		Ref->accept(*CGM);
		V = Ref->getCodeGen()->getValue();
	} else {
		CGM->Diag(diag::err_invalid_behavior);
	}
}

void CodeGenExpr::GenExpr(SemaCast *Sema) {
	// Evaluate the inner expression first
	Sema->getExpr()->accept(*CGM);
	llvm::Value *SrcVal = Sema->getExpr()->getCodeGen()->getValue();

	SemaType *FromType = Sema->getExpr()->getType();
	SemaType *ToType = Sema->getToType(); // Sema->getType() == ToType

	// Helper: map SemaIntTypeKind to LLVM integer type
	auto intLLVMType = [](SemaIntTypeKind k) -> llvm::Type * {
		switch (k) {
			case SemaIntTypeKind::TYPE_BYTE:                 return CodeGen::Int8Ty;
			case SemaIntTypeKind::TYPE_SHORT:
			case SemaIntTypeKind::TYPE_USHORT:               return CodeGen::Int16Ty;
			case SemaIntTypeKind::TYPE_INT:
			case SemaIntTypeKind::TYPE_UINT:                 return CodeGen::Int32Ty;
			case SemaIntTypeKind::TYPE_LONG:
			case SemaIntTypeKind::TYPE_ULONG:                return CodeGen::Int64Ty;
			default:                                         return CodeGen::Int32Ty;
		}
	};

	switch (ToType->getKind()) {

		case SemaKind::TYPE_BOOL: {
			// Cast to bool: non-zero → true
			if (FromType->isInteger() || FromType->getKind() == SemaKind::TYPE_BOOL) {
				V = Builder->CreateICmpNE(SrcVal, llvm::ConstantInt::get(SrcVal->getType(), 0));
			} else if (FromType->isFloat()) {
				V = Builder->CreateFCmpONE(SrcVal, llvm::ConstantFP::get(SrcVal->getType(), 0.0));
			} else {
				V = SrcVal;
			}
			break;
		}

		case SemaKind::TYPE_INTEGER: {
			SemaIntType *ToIntTy = static_cast<SemaIntType *>(ToType);
			llvm::Type *DestTy = intLLVMType(ToIntTy->getIntKind());
			if (FromType->getKind() == SemaKind::TYPE_BOOL) {
				// bool (i1 or i8) → integer: zero-extend
				V = Builder->CreateZExtOrTrunc(SrcVal, DestTy);
			} else if (FromType->isInteger()) {
				SemaIntType *FromIntTy = static_cast<SemaIntType *>(FromType);
				unsigned SrcBits = SrcVal->getType()->getIntegerBitWidth();
				unsigned DstBits = DestTy->getIntegerBitWidth();
				if (DstBits < SrcBits) {
					V = Builder->CreateTrunc(SrcVal, DestTy);
				} else if (DstBits > SrcBits) {
					V = FromIntTy->isSigned()
						? Builder->CreateSExt(SrcVal, DestTy)
						: Builder->CreateZExt(SrcVal, DestTy);
				} else {
					V = Builder->CreateBitCast(SrcVal, DestTy);
				}
			} else if (FromType->isFloat()) {
				V = ToIntTy->isSigned()
					? Builder->CreateFPToSI(SrcVal, DestTy)
					: Builder->CreateFPToUI(SrcVal, DestTy);
			} else {
				V = SrcVal;
			}
			break;
		}

		case SemaKind::TYPE_FLOAT: {
			SemaFloatType *ToFPTy = static_cast<SemaFloatType *>(ToType);
			llvm::Type *DestTy = (ToFPTy->getFloatKind() == SemaFloatTypeKind::TYPE_FLOAT)
				? CodeGen::FloatTy : CodeGen::DoubleTy;
			if (FromType->getKind() == SemaKind::TYPE_BOOL) {
				V = Builder->CreateUIToFP(SrcVal, DestTy);
			} else if (FromType->isInteger()) {
				SemaIntType *FromIntTy = static_cast<SemaIntType *>(FromType);
				V = FromIntTy->isSigned()
					? Builder->CreateSIToFP(SrcVal, DestTy)
					: Builder->CreateUIToFP(SrcVal, DestTy);
			} else if (FromType->isFloat()) {
				SemaFloatType *FromFPTy = static_cast<SemaFloatType *>(FromType);
				bool toDouble = (ToFPTy->getFloatKind() == SemaFloatTypeKind::TYPE_DOUBLE);
				bool fromDouble = (FromFPTy->getFloatKind() == SemaFloatTypeKind::TYPE_DOUBLE);
				if (!fromDouble && toDouble) {
					V = Builder->CreateFPExt(SrcVal, DestTy);
				} else if (fromDouble && !toDouble) {
					V = Builder->CreateFPTrunc(SrcVal, DestTy);
				} else {
					V = SrcVal;
				}
			} else {
				V = SrcVal;
			}
			break;
		}

		case SemaKind::TYPE_VOID:
		case SemaKind::TYPE_ERROR:
		case SemaKind::TYPE_ENUM:
		case SemaKind::TYPE_STRING:
		case SemaKind::TYPE_ARRAY:
		case SemaKind::TYPE_CLASS:
		default:
			// Unsupported cast: pass source value through unchanged
			V = SrcVal;
			break;
	}
}

void CodeGenExpr::GenExpr(SemaUnary *Sema) {
    FLY_DEBUG_SCOPE("CodeGenExpr", "GenUnary");
	ASTUnary &Unary = Sema->getAST();
    assert(Unary.getExprKind() == ASTExprKind::EXPR_UNARY && "Expected Unary Group Expr");
    assert(Sema->getExpr() && "Unary Expr empty");

	Sema->getExpr()->accept(*CGM);
    llvm::Value *OldVal = Sema->getExpr()->getCodeGen()->getValue();
	llvm::Value *NewVal = nullptr;

    switch (Unary.getOpKind()) {

        case ASTUnaryKind::OP_UNARY_PRE_INCR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CodeGen::Int32Ty, 1);
            NewVal = Builder->CreateNSWAdd(OldVal, RHS);
            V = NewVal;
        } break;
        case ASTUnaryKind::OP_UNARY_POST_INCR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CodeGen::Int32Ty, 1);
            NewVal = Builder->CreateNSWAdd(OldVal, RHS);
            V = OldVal;
        } break;
        case ASTUnaryKind::OP_UNARY_PRE_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CodeGen::Int32Ty, -1, true);
            NewVal = Builder->CreateNSWAdd(OldVal, RHS);
        	V = NewVal;
        } break;
        case ASTUnaryKind::OP_UNARY_POST_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CodeGen::Int32Ty, -1, true);
            NewVal = Builder->CreateNSWAdd(OldVal, RHS);
        	V = NewVal;
        } break;
        case ASTUnaryKind::OP_UNARY_NOT_LOG: {
	        OldVal = Builder->CreateTrunc(OldVal, CodeGen::BoolTy);
        	OldVal = Builder->CreateXor(OldVal, true);
        	V = Builder->CreateZExt(OldVal, CodeGen::Int8Ty);
        } break;
    }

	// Only mutation ops (pre/post incr/decr) set NewVal; read-only ops (!) do not.
	if (NewVal &&
		(Sema->getExpr()->getKind() == SemaKind::LOCAL_VAR ||
		 Sema->getExpr()->getKind() == SemaKind::PARAM_VAR ||
		 Sema->getExpr()->getKind() == SemaKind::ATTRIBUTE)) {
		static_cast<SemaVar *>(Sema->getExpr())->getCodeGen()->Store(NewVal);
	}
}

void CodeGenExpr::GenExpr(SemaBinary *Sema) {
	FLY_DEBUG_SCOPE("CodeGenExpr", "GenBinary");
	ASTBinary &Binary = Sema->getAST();
	assert(Binary.getExprKind() == ASTExprKind::EXPR_BINARY && "Expected Binary Group Expr");

	SemaExpr *Left = Sema->getLeft();
	SemaExpr *Right = Sema->getRight();
	ASTBinaryKind OpKind = Binary.getBinaryKind();

	// Generate Left and Right CodeGen Expressions
	Left->accept(*CGM);
	Right->accept(*CGM);

	// Generate Binary Operation
	if (Binary.isArith()) {
		V = GenBinaryArith(Left, OpKind, Right);
	} else if (Binary.isCompare()) {
		V = GenBinaryCompare(Left, OpKind, Right);
	} else if (Binary.isLogic()) {
		V = GenBinaryLogic(Left, OpKind, Right);
	} else if (Binary.isAssign()) {
		V = GenBinaryAssign(Left, Right);
	} else {
		assert(0 && "Unknown Binary Operation");
	}
}


void CodeGenExpr::GenExpr(SemaTernary *Sema) {
	ASTTernary &Ternary = Sema->getAST();

	llvm::BasicBlock *FromBB = Builder->GetInsertBlock();
	Sema->getCond()->accept(*CGM);
	llvm::Value *Cond = Sema->getCond()->getCodeGen()->getValue();

	// Create Blocks
	llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "terntrue", FromBB->getParent());
	llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternfalse", FromBB->getParent());
	llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternend", FromBB->getParent());

	// Create Condition
	Builder->CreateCondBr(Cond, TrueBB, FalseBB);

	// True Label
	Builder->SetInsertPoint(TrueBB);
	Sema->getTrueExpr()->accept(*CGM);
	llvm::Value *True = Sema->getTrueExpr()->getCodeGen()->getValue();
	llvm::Value *BoolTrue = ConvertToBool(True);
	Builder->CreateBr(EndBB);

	// False Label
	Builder->SetInsertPoint(FalseBB);
	Sema->getFalseExpr()->accept(*CGM);
	llvm::Value *False = Sema->getFalseExpr()->getCodeGen()->getValue();
	llvm::Value *BoolFalse = ConvertToBool(False);
	Builder->CreateBr(EndBB);

	// End Label
	Builder->SetInsertPoint(EndBB);
	llvm::PHINode *Phi = Builder->CreatePHI(CodeGen::BoolTy, 2);
	Phi->addIncoming(BoolTrue, TrueBB);
	Phi->addIncoming(BoolFalse, FalseBB);
	V = Phi;
}

llvm::Value *CodeGenExpr::GenStringConcat(SemaExpr *E1, SemaExpr *E2) {
	llvm::Value *V1 = E1->getCodeGen()->getValue();
	llvm::Value *V2 = E2->getCodeGen()->getValue();

	// Extract ptr (field 0) and size (field 1) from each string struct
	llvm::Value *Ptr1  = Builder->CreateExtractValue(V1, 0, "s1_ptr");
	llvm::Value *Size1 = Builder->CreateExtractValue(V1, 1, "s1_size");
	llvm::Value *Ptr2  = Builder->CreateExtractValue(V2, 0, "s2_ptr");
	llvm::Value *Size2 = Builder->CreateExtractValue(V2, 1, "s2_size");

	// Total size in IntTy; extend to size_t (IntPtrTy) for malloc/memcpy
	llvm::Value *Total    = Builder->CreateAdd(Size1, Size2, "str_total");
	llvm::Value *TotalExt = Builder->CreateZExt(Total, CodeGen::IntPtrTy, "str_total_ext");
	llvm::Value *Size1Ext = Builder->CreateZExt(Size1, CodeGen::IntPtrTy, "s1_size_ext");
	llvm::Value *Size2Ext = Builder->CreateZExt(Size2, CodeGen::IntPtrTy, "s2_size_ext");

	// Allocate buffer: malloc(total_size)
	llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
		"malloc",
		llvm::FunctionType::get(
			llvm::PointerType::getUnqual(CGM->LLVMCtx),
			{CodeGen::IntPtrTy}, false));
	llvm::Value *Buf = Builder->CreateCall(MallocFn, {TotalExt}, "str_buf");

	// memcpy(buf, ptr1, size1)
	Builder->CreateMemCpy(Buf, llvm::MaybeAlign(), Ptr1, llvm::MaybeAlign(), Size1Ext);

	// memcpy(buf + size1, ptr2, size2)
	llvm::Value *Buf2 = Builder->CreateGEP(CodeGen::Int8Ty, Buf, Size1Ext, "str_buf2");
	Builder->CreateMemCpy(Buf2, llvm::MaybeAlign(), Ptr2, llvm::MaybeAlign(), Size2Ext);

	// Build result string struct { buf, total }
	llvm::Value *Result = llvm::UndefValue::get(CodeGen::StringTy);
	Result = Builder->CreateInsertValue(Result, Buf, 0);
	Result = Builder->CreateInsertValue(Result, Total, 1);
	return Result;
}

llvm::Value *CodeGenExpr::GenStringHeapCopy(SemaStringValue *Sema) {
	llvm::StringRef Str = Sema->getValue();
	uint32_t Size = Str.size();
	llvm::Constant *GlobalPtr = Builder->CreateGlobalStringPtr(Str);
	llvm::Value *SizeVal = llvm::ConstantInt::get(CodeGen::Int32Ty, Size);
	llvm::Value *SizeExt = Builder->CreateZExt(SizeVal, CodeGen::IntPtrTy, "str_size_ext");

	llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
		"malloc",
		llvm::FunctionType::get(
			llvm::PointerType::getUnqual(CGM->LLVMCtx),
			{CodeGen::IntPtrTy}, false));
	llvm::Value *HeapPtr = Builder->CreateCall(MallocFn, {SizeExt}, "str_heap");
	Builder->CreateMemCpy(HeapPtr, llvm::MaybeAlign(), GlobalPtr, llvm::MaybeAlign(), SizeExt);

	llvm::Value *Result = llvm::UndefValue::get(CodeGen::StringTy);
	Result = Builder->CreateInsertValue(Result, HeapPtr, 0);
	Result = Builder->CreateInsertValue(Result, SizeVal, 1);
	return Result;
}

llvm::Value *CodeGenExpr::GenStringClone(llvm::Value *StrVal) {
	// Deep-copy an arbitrary runtime %string ({ptr, i32}) into a fresh malloc'd
	// buffer so the destination owns an independent heap buffer. Used when a string
	// lvalue (another variable/parameter/field) is bound into an owned string slot,
	// so each owner can free() its own buffer (no aliasing → no double free).
	llvm::Value *SrcPtr  = Builder->CreateExtractValue(StrVal, 0, "clone_src_ptr");
	llvm::Value *Size    = Builder->CreateExtractValue(StrVal, 1, "clone_size");
	llvm::Value *SizeExt = Builder->CreateZExt(Size, CodeGen::IntPtrTy, "clone_size_ext");

	llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
		"malloc",
		llvm::FunctionType::get(
			llvm::PointerType::getUnqual(CGM->LLVMCtx),
			{CodeGen::IntPtrTy}, false));
	llvm::Value *HeapPtr = Builder->CreateCall(MallocFn, {SizeExt}, "str_clone");
	Builder->CreateMemCpy(HeapPtr, llvm::MaybeAlign(), SrcPtr, llvm::MaybeAlign(), SizeExt);

	llvm::Value *Result = llvm::UndefValue::get(CodeGen::StringTy);
	Result = Builder->CreateInsertValue(Result, HeapPtr, 0);
	Result = Builder->CreateInsertValue(Result, Size, 1);
	return Result;
}

llvm::Value *CodeGenExpr::GenBinaryArith(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_SCOPE("CodeGenExpr", "GenBinaryArith");

	SemaType *T1 = E1->getType();
	SemaType *T2 = E2->getType();

	// String concatenation (+ only)
	if (T1->isString() && T2->isString())
		return GenStringConcat(E1, E2);

	SemaNumberType *Type1 = static_cast<SemaNumberType *>(T1);
	SemaNumberType *Type2 = static_cast<SemaNumberType *>(T2);

	// Get Values
    llvm::Value *V1 = E1->getCodeGen()->getValue();
    llvm::Value *V2 = E2->getCodeGen()->getValue();

	// Promotion rules: convert Type1 or Type2 to the max type
	SemaNumberType *EffectiveType = Type1;
	if (Type1->getRank() > Type2->getRank()) {
		bool Src2Signed = !Type2->isInteger() || static_cast<SemaIntType *>(Type2)->isSigned();
		V2 = ConvertNumber(V2, Type1, Src2Signed); // Implicit conversion
	} else if (Type1->getRank() < Type2->getRank()) {
		// Promote T1 to T2 Type
		bool Src1Signed = !Type1->isInteger() || static_cast<SemaIntType *>(Type1)->isSigned();
		V1 = ConvertNumber(V1, Type2, Src1Signed); // Implicit conversion
		EffectiveType = Type2;
	}

	// Choose float vs integer instructions based on effective type
	bool IsFloat = EffectiveType->isFloat();
	bool IsUnsignedInt = !IsFloat && EffectiveType->isInteger() &&
	    !static_cast<SemaIntType *>(EffectiveType)->isSigned();

    switch (OperatorKind) {

        case ASTBinaryKind::OP_BINARY_ARITH_ADD:
            return IsFloat ? Builder->CreateFAdd(V1, V2) : Builder->CreateAdd(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_SUB:
            return IsFloat ? Builder->CreateFSub(V1, V2) : Builder->CreateSub(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_MUL:
            return IsFloat ? Builder->CreateFMul(V1, V2) : Builder->CreateMul(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_DIV:
            return IsFloat ? Builder->CreateFDiv(V1, V2)
                 : IsUnsignedInt ? Builder->CreateUDiv(V1, V2)
                                 : Builder->CreateSDiv(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_MOD:
            return IsFloat ? Builder->CreateFRem(V1, V2)
                 : IsUnsignedInt ? Builder->CreateURem(V1, V2)
                                 : Builder->CreateSRem(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_AND:
            return Builder->CreateAnd(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_OR:
            return Builder->CreateOr(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_XOR:
            return Builder->CreateXor(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_SHIFT_L:
            return Builder->CreateShl(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_SHIFT_R:
            return IsUnsignedInt ? Builder->CreateLShr(V1, V2)
                                 : Builder->CreateAShr(V1, V2);
    }

	// Error
	CGM->Diag(diag::err_invalid_behavior);
	return nullptr;
}

llvm::Value *CodeGenExpr::GenBinaryCompare(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_SCOPE("CodeGenExpr", "GenBinaryComparison");
	SemaType *Type1 = E1->getType();
	SemaType *Type2 = E2->getType();

	// Get Values
    llvm::Value *V1 = E1->getCodeGen()->getValue();
    llvm::Value *V2 = E2->getCodeGen()->getValue();

	// String equality/inequality: compare size then memcmp
	if (Type1->isString() && Type2->isString()) {
		llvm::Value *Size1 = Builder->CreateExtractValue(V1, 1, "s1_size");
		llvm::Value *Size2 = Builder->CreateExtractValue(V2, 1, "s2_size");
		llvm::Value *SizesEq = Builder->CreateICmpEQ(Size1, Size2, "sizes_eq");

		llvm::Function *Fn = Builder->GetInsertBlock()->getParent();
		llvm::BasicBlock *DataCmpBB  = llvm::BasicBlock::Create(CGM->LLVMCtx, "str_data_cmp", Fn);
		llvm::BasicBlock *SizeDiffBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "str_size_diff", Fn);
		llvm::BasicBlock *EndBB      = llvm::BasicBlock::Create(CGM->LLVMCtx, "str_cmp_end", Fn);

		Builder->CreateCondBr(SizesEq, DataCmpBB, SizeDiffBB);

		// Branch: sizes differ → not equal
		Builder->SetInsertPoint(SizeDiffBB);
		Builder->CreateBr(EndBB);

		// Branch: compare raw data via memcmp
		Builder->SetInsertPoint(DataCmpBB);
		llvm::Value *Ptr1    = Builder->CreateExtractValue(V1, 0, "s1_ptr");
		llvm::Value *Ptr2    = Builder->CreateExtractValue(V2, 0, "s2_ptr");
		llvm::Value *SizeExt = Builder->CreateZExt(Size1, CodeGen::IntPtrTy, "size_ext");
		llvm::FunctionCallee MemCmpFn = CGM->Module->getOrInsertFunction(
			"memcmp",
			llvm::FunctionType::get(
				CodeGen::Int32Ty,
				{llvm::PointerType::getUnqual(CGM->LLVMCtx),
				 llvm::PointerType::getUnqual(CGM->LLVMCtx),
				 CodeGen::IntPtrTy}, false));
		llvm::Value *CmpResult = Builder->CreateCall(MemCmpFn, {Ptr1, Ptr2, SizeExt}, "memcmp_res");
		llvm::Value *DataEq = Builder->CreateICmpEQ(
			CmpResult, llvm::ConstantInt::get(CodeGen::Int32Ty, 0), "data_eq");
		Builder->CreateBr(EndBB);
		llvm::BasicBlock *AfterDataBB = Builder->GetInsertBlock();

		Builder->SetInsertPoint(EndBB);
		llvm::PHINode *Phi = Builder->CreatePHI(CodeGen::BoolTy, 2, "str_eq");
		Phi->addIncoming(llvm::ConstantInt::get(CodeGen::BoolTy, 0), SizeDiffBB);
		Phi->addIncoming(DataEq, AfterDataBB);

		switch (OperatorKind) {
			case ASTBinaryKind::OP_BINARY_COMPARE_EQ: return Phi;
			case ASTBinaryKind::OP_BINARY_COMPARE_NE: return Builder->CreateNot(Phi, "str_ne");
			default:
				CGM->Diag(diag::err_invalid_behavior);
				return nullptr;
		}
	}

    if (Type1->isBool() && Type2->isBool()) {
	    switch (OperatorKind) {
	    	case ASTBinaryKind::OP_BINARY_COMPARE_EQ:
	    		return Builder->CreateICmpEQ(V1, V2);
	    	case ASTBinaryKind::OP_BINARY_COMPARE_NE:
	    		return Builder->CreateICmpNE(V1, V2);
	    }
    }

	// Enums are represented as i32 (see CodeGenType::GenType(SemaEnumType)), so
	// compare their underlying integer values. Without this, enum comparisons
	// fall through and return nullptr, producing a `br <null>` and a broken
	// function in the verifier.
	if (Type1->isEnum() && Type2->isEnum()) {
		switch (OperatorKind) {
			case ASTBinaryKind::OP_BINARY_COMPARE_EQ:
				return Builder->CreateICmpEQ(V1, V2);
			case ASTBinaryKind::OP_BINARY_COMPARE_NE:
				return Builder->CreateICmpNE(V1, V2);
			case ASTBinaryKind::OP_BINARY_COMPARE_GT:
				return Builder->CreateICmpSGT(V1, V2);
			case ASTBinaryKind::OP_BINARY_COMPARE_GTE:
				return Builder->CreateICmpSGE(V1, V2);
			case ASTBinaryKind::OP_BINARY_COMPARE_LT:
				return Builder->CreateICmpSLT(V1, V2);
			case ASTBinaryKind::OP_BINARY_COMPARE_LTE:
				return Builder->CreateICmpSLE(V1, V2);
		}
	}

	if (Type1->isInteger() && Type2->isInteger()) {
		SemaIntType *IntType1 = static_cast<SemaIntType *>(E1->getType());
		SemaIntType *IntType2 = static_cast<SemaIntType *>(E2->getType());

		// Check if we need to promote one of the integers
		if (IntType1->getRank() > IntType2->getRank()) {
			V2 = ConvertToInteger(V2, IntType1); // Promote V2
		} else if (IntType1->getRank() < IntType2->getRank()) {
			V1 = ConvertToInteger(V1, IntType2); // Promote V1
		}

        bool Signed = IntType1->isSigned() || IntType2->isSigned();
        switch (OperatorKind) {

            case ASTBinaryKind::OP_BINARY_COMPARE_EQ:
                return Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_NE:
                return Builder->CreateICmpNE(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_GT:
                return Signed ? Builder->CreateICmpSGT(V1, V2) : Builder->CreateICmpUGT(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_GTE:
                return Signed ? Builder->CreateICmpSGE(V1, V2) : Builder->CreateICmpUGE(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_LT:
                return Signed ? Builder->CreateICmpSLT(V1, V2) : Builder->CreateICmpULT(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_LTE:
                return Signed ? Builder->CreateICmpSLE(V1, V2) : Builder->CreateICmpULE(V1, V2);
        }
    }

	if (Type1->isFloat() && Type2->isFloat()) {
		SemaFloatType *FloatType1 = static_cast<SemaFloatType *>(E1->getType());
		SemaFloatType *FloatType2 = static_cast<SemaFloatType *>(E2->getType());

		// Check if we need to promote one of the integers
		if (FloatType1->getRank() > FloatType2->getRank()) {
			V2 = ConvertToFloat(V2, FloatType1); // Promote V2
		} else if (FloatType1->getRank() < FloatType2->getRank()) {
			V1 = ConvertToFloat(V1, FloatType2); // Promote V1
		}

        switch (OperatorKind) {

            case ASTBinaryKind::OP_BINARY_COMPARE_EQ:
                return Builder->CreateFCmpOEQ(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_NE:
                return Builder->CreateFCmpONE(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_GT:
                return Builder->CreateFCmpOGT(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_GTE:
                return Builder->CreateFCmpOGE(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_LT:
                return Builder->CreateFCmpOLT(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_LTE:
                return Builder->CreateFCmpOLE(V1, V2);
        }
    }

	// Class/interface/struct compared with the `null` literal (or another reference):
	// compare as pointers. The `null` value lowers to a null pointer constant and class
	// values are pointers, so a pointer (in)equality is well-formed. Without this the
	// comparison fell through to nullptr → `br <null operand>` → broken function.
	if ((Type1 && Type1->isClass()) || (Type2 && Type2->isClass())) {
		if (V1 && V2 && V1->getType()->isPointerTy() && V2->getType()->isPointerTy()) {
			switch (OperatorKind) {
				case ASTBinaryKind::OP_BINARY_COMPARE_EQ:
					return Builder->CreateICmpEQ(V1, V2);
				case ASTBinaryKind::OP_BINARY_COMPARE_NE:
					return Builder->CreateICmpNE(V1, V2);
				default: break;
			}
		}
	}

	// Error
	CGM->Diag(diag::err_invalid_behavior);
	return nullptr;
}

llvm::Value *CodeGenExpr::GenBinaryLogic(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_SCOPE("CodeGenExpr", "GenBinaryLogic");

	// Get Values
	llvm::Value *V1 = E1->getCodeGen()->getValue();

    V1 = ConvertToBool(V1);
    llvm::BasicBlock *FromBB = Builder->GetInsertBlock();

    switch (OperatorKind) {

        case ASTBinaryKind::OP_BINARY_LOGIC_AND: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", FromBB->getParent());
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", FromBB->getParent());

            // From Branch
            Builder->CreateCondBr(V1, LeftBB, RightBB);

            // Left Branch
            Builder->SetInsertPoint(LeftBB);
            llvm::Value *V2 = ConvertToBool(E2->getCodeGen()->getValue());
            Builder->CreateBr(RightBB);

            // Right Branch
            Builder->SetInsertPoint(RightBB);
            llvm::PHINode *Phi = Builder->CreatePHI(CodeGen::BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CodeGen::BoolTy, false, false), FromBB);
            Phi->addIncoming(V2, LeftBB);
            return Phi;
        }
        case ASTBinaryKind::OP_BINARY_LOGIC_OR: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", FromBB->getParent());
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", FromBB->getParent());

            // From Branch
            Builder->CreateCondBr(V1, RightBB, LeftBB);

            // Left Branch
            Builder->SetInsertPoint(LeftBB);
            llvm::Value *V2 = ConvertToBool(E2->getCodeGen()->getValue());
            Builder->CreateBr(RightBB);

            // Right Branch
            Builder->SetInsertPoint(RightBB);
            llvm::PHINode *Phi = Builder->CreatePHI(CodeGen::BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CodeGen::BoolTy, true, false), FromBB);
            Phi->addIncoming(V2, LeftBB);
            return Phi;
        }
    }

	// Error
	CGM->Diag(diag::err_invalid_behavior);
	return nullptr;
}

llvm::Value * CodeGenExpr::GenBinaryAssign(SemaExpr *E1, SemaExpr *E2) {
	// Validate E1 and E2 are not null
	assert(E1 && "E1 is null");
	assert(E2 && "E2 is null");
	// Get CodeGen objects
	CodeGenExpr *E1CodeGen = E1->getCodeGen();
	CodeGenExpr *E2CodeGen = E2->getCodeGen();

	// Check if CodeGen was properly generated
	assert(E1CodeGen && "E1 CodeGen is null");
	assert(E2CodeGen && "E2 CodeGen is null");

	// Check if E2 is an array value - if so, use specialized store
	if (E1->getType()->isArray() && E2->getType()->isArray()) {
		CodeGenArrayValue *E2CGArray = static_cast<CodeGenArrayValue *>(E2CodeGen);
		// Use StoreArrayValue to store both the pointer and the elements
		return static_cast<SemaVar *>(E1)->getCodeGen()->StoreArrayValue(E2CGArray);
	}

	// Non-const string store: the destination must own an independent heap buffer
	// so it can free() it at scope exit without aliasing another owner's buffer.
	//  - RHS is a string literal (VALUE)        → heap-copy the global constant.
	//  - RHS is a string lvalue (var/param/field/member) → deep-clone its buffer.
	// RHS that produces a *fresh* buffer (CALL result, BINARY concat, …) is left
	// as-is: the destination takes ownership of that already-unique buffer. Because
	// `out = <lvalue>` returns flow through here too, every function that returns a
	// borrowed string (a param or field) returns a fresh clone — so its callers'
	// CALL results are always fresh and need no clone.
	if (E1->getType()->isString() && E2->getType() && E2->getType()->isString()) {
		SemaKind K1 = E1->getKind();
		bool IsVar1 = K1 == SemaKind::LOCAL_VAR || K1 == SemaKind::PARAM_VAR ||
		              K1 == SemaKind::ERROR_VAR  || K1 == SemaKind::ATTRIBUTE ||
		              K1 == SemaKind::INSTANCE_VAR;
		// A `this.field = ...` store is a MEMBER whose ref is an attribute/instance
		// var; the field owns its heap buffer and outlives the assigning scope, so it
		// needs the same clone-on-store as a direct var (otherwise it aliases the RHS
		// param/local, which is freed at scope exit → use-after-free).
		bool IsMember1 = K1 == SemaKind::MEMBER;
		bool Const1 = IsVar1 && static_cast<SemaVar *>(E1)->isConstant();
		if ((IsVar1 || IsMember1) && !Const1) {
			SemaKind K2 = E2->getKind();
			if (K2 == SemaKind::VALUE) {
				llvm::Value *HeapStr = GenStringHeapCopy(static_cast<SemaStringValue *>(E2));
				return static_cast<CodeGenVar *>(E1CodeGen)->Store(HeapStr);
			}
			bool IsLValue2 = K2 == SemaKind::LOCAL_VAR || K2 == SemaKind::PARAM_VAR ||
			                 K2 == SemaKind::ERROR_VAR  || K2 == SemaKind::ATTRIBUTE ||
			                 K2 == SemaKind::INSTANCE_VAR || K2 == SemaKind::MEMBER;
			if (IsLValue2) {
				llvm::Value *Clone = GenStringClone(E2CodeGen->getValue());
				return static_cast<CodeGenVar *>(E1CodeGen)->Store(Clone);
			}
		}
	}

	llvm::Value *V2 = E2CodeGen->getValue();

	if (E1->getType()->isNumber() && E2->getType()->isNumber()) {
		SemaNumberType *Type1 = static_cast<SemaNumberType *>(E1->getType());
		SemaNumberType *Type2 = static_cast<SemaNumberType *>(E2->getType());

		// Promotion rules: convert Type1 or Type2 to the max type
		if (Type1->getRank() > Type2->getRank()) {
			bool Src2Signed = !Type2->isInteger() || static_cast<SemaIntType *>(Type2)->isSigned();
			V2 = ConvertNumber(V2, Type1, Src2Signed); // Implicit conversion
		}
	}
	// Bridge: when storing a CLang instance into a variable, propagate the
	// InstancePtr → lib mapping to alloca → lib so call() can look it up.
	if (E2->getKind() == SemaKind::CALL) {
		auto LibIt = CGM->CLangLibMap.find(V2);
		if (LibIt != CGM->CLangLibMap.end()) {
			SemaKind K1 = E1->getKind();
			if (K1 == SemaKind::LOCAL_VAR || K1 == SemaKind::PARAM_VAR ||
			    K1 == SemaKind::ATTRIBUTE || K1 == SemaKind::INSTANCE_VAR) {
				llvm::Value *DestAlloca = static_cast<CodeGenVar *>(E1CodeGen)->getPointer();
				CGM->CLangLibMap[DestAlloca] = LibIt->second;
				// Also register by SemaVar* so call() can find the lib even when
				// the GEP pointer changes between the assignment and the call site.
				CGM->CLangLibMapBySema[static_cast<SemaVar *>(E1)] = LibIt->second;
			}
		}
	}

	// Upcast a derived class to a base/interface lvalue: store the base-subobject
	// pointer so the variable's later dispatch uses the right vtable.
	V2 = adjustToBaseSubobject(V2, E2->getType(), E1->getType());

	return static_cast<CodeGenVar *>(E1CodeGen)->Store(V2);
}

llvm::Value *CodeGenExpr::adjustToBaseSubobject(llvm::Value *V, SemaType *FromType, SemaType *ToType) {
	if (!V || !V->getType()->isPointerTy()) return V;
	if (!FromType || !ToType || !FromType->isClass() || !ToType->isClass()) return V;
	SemaClassType *From = static_cast<SemaClassType *>(FromType);
	SemaClassType *To   = static_cast<SemaClassType *>(ToType);
	// Adjust an upcast (derived → base CLASS or INTERFACE) to the base subobject pointer, so a
	// base-typed lvalue points at its own subobject. This preserves the invariant every consumer
	// relies on — that a T-typed value points at the T subobject — so field access and
	// inherited/virtual dispatch through the base-typed value resolve against the correct
	// subobject and vtable (getBaseInstance navigates from the *static* type). Without this,
	// `Base x = new Derived()` stored the full-object pointer and reads through x were wrong
	// (bug #8). The per-base vtable thunks restore `this` to the most-derived object for
	// overrides, so free()/identity through the base still reach the right method.
	// Same type, or no codegen, or To is not a base of From → nothing to adjust.
	if (From->isEquals(To) || !From->getCodeGen()) return V;
	// getBaseInstance navigates From's base subobjects (recursively) to To and returns
	// the adjusted pointer, or null when To is not a base of From.
	llvm::Value *Adj = From->getCodeGen()->getBaseInstance(V, To);
	return Adj ? Adj : V;
}

void CodeGenExpr::addArgs(SemaCall *Sema, llvm::SmallVector<llvm::Value *, 8> &Args) {
	// Add Call arguments using resolved Sema expressions
	auto &Params = Sema->getFunction()->getParams();
	auto &ArgExprs = Sema->getArgs();

	for (size_t i = 0; i < ArgExprs.size(); i++) {
		SemaExpr *ArgExpr = ArgExprs[i];

		// An upcast of a derived class to a base CLASS or INTERFACE param is a value-pass for
		// polymorphic dispatch (not a writable reference): it must NOT take the output-param
		// fast-path below, and the pointer needs adjusting to the base subobject so the callee
		// reads the right fields/vtable (bug #8).
		bool IsUpcast = i < Params.size() && ArgExpr->getType() && Params[i]->getType() &&
			ArgExpr->getType()->isClass() && Params[i]->getType()->isClass() &&
			!ArgExpr->getType()->isEquals(Params[i]->getType()) &&
			static_cast<SemaClassType *>(ArgExpr->getType())->isDerived(static_cast<SemaClassType *>(Params[i]->getType()));

		// For output (non-const) params: pass the original alloca pointer directly and
		// invalidate the variable's load cache so subsequent reads see the updated value.
		if (i < Params.size() && !Params[i]->isConstant() && !IsUpcast) {
			SemaKind K = ArgExpr->getKind();
			if (K == SemaKind::LOCAL_VAR || K == SemaKind::PARAM_VAR ||
			    K == SemaKind::ERROR_VAR || K == SemaKind::ATTRIBUTE ||
			    K == SemaKind::INSTANCE_VAR) {
				// For non-static class attributes: ensure the GEP pointer is computed
				// before we hand the address to the callee (the GenBody loop pre-sets the
				// wrong InstancePtr alloca; visit(SemaClassAttribute) fixes const args but
				// is not called on this output-param path).
				if (K == SemaKind::ATTRIBUTE) {
					ArgExpr->accept(*CGM);  // triggers visit(SemaClassAttribute) → GEP setup
				}
				CodeGenVar *CGV = static_cast<SemaVar *>(ArgExpr)->getCodeGen();
				// For struct-type hidden out-params (__out_N): the callee convention does
				// `load ptr, ptr %arg` to retrieve a heap File*.  Pre-allocate the struct
				// on the heap so the callee has a valid target to write into.
				if (ArgExpr == Sema->getOutVar()) {
					llvm::Type *OVTy = CGV->getType();
					if (OVTy->isStructTy() && OVTy != CodeGen::StringTy && OVTy != CodeGen::ArrayTy) {
						SemaType *SemaTy = static_cast<SemaVar *>(ArgExpr)->getType();
						bool isStructKind = SemaTy && SemaTy->getKind() == SemaKind::TYPE_CLASS &&
							static_cast<SemaClassType *>(SemaTy)->getClassKind() == SemaClassKind::STRUCT;
						if (!isStructKind) {
							// CLASS/INTERFACE: heap-allocate so the callee has a valid target.
							llvm::Type *PSITy = CGM->Module->getDataLayout().getIntPtrType(CGM->LLVMCtx);
							llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
								"malloc",
								llvm::FunctionType::get(
									llvm::PointerType::getUnqual(CGM->LLVMCtx), {PSITy}, false));
							llvm::Value *HeapPtr = Builder->CreateCall(MallocFn, {
								Builder->CreateIntCast(
									llvm::ConstantExpr::getSizeOf(OVTy), PSITy, false)});
							Builder->CreateStore(HeapPtr, CGV->getPointer());
						}
						// STRUCT: ptr_slot already points to stack data from CodeGenVar::Alloca()
					}
				}
				Args.push_back(CGV->getPointer());
				CGV->resetLoad();  // force fresh load on next use
				continue;
			}
		}

		ArgExpr->accept(*CGM);
		llvm::Value *V = ArgExpr->getCodeGen()->getValue();

		// Upcast a derived class argument to a base/interface param: adjust the pointer
		// to the base subobject so the callee's dispatch reads the right vtable.
		if (IsUpcast) {
			V = adjustToBaseSubobject(V, ArgExpr->getType(), Params[i]->getType());
			// A non-const param is passed by reference (the callee loads a pointer from
			// the slot), so hand it a temp slot holding the adjusted pointer. A const
			// param takes the pointer by value, so pass it directly.
			if (!Params[i]->isConstant()) {
				llvm::AllocaInst *Tmp = Builder->CreateAlloca(V->getType());
				Builder->CreateStore(V, Tmp);
				V = Tmp;
			}
			Args.push_back(V);
			continue;
		}

		// Allow class ↔ long parameter interchange: ptrtoint / inttoptr.
		if (i < Params.size()) {
			Params[i]->getType()->accept(*CGM);
			llvm::Type *ExpLLVMTy = Params[i]->getType()->getCodeGen()->getType();
			if (V->getType()->isPointerTy() && ExpLLVMTy->isIntegerTy(64)) {
				// class → long: convert class pointer to i64
				V = Builder->CreatePtrToInt(V, CodeGen::Int64Ty);
			} else if (V->getType()->isIntegerTy(64) && ExpLLVMTy->isPointerTy()) {
				// long → class: convert i64 to opaque pointer
				V = Builder->CreateIntToPtr(V, llvm::PointerType::getUnqual(CGM->LLVMCtx));
			}
		}

		// If value is not a pointer but the function expects a pointer (by-reference params),
		// create a temp alloca, convert the value to match the param type, store, and pass pointer
		if (!V->getType()->isPointerTy() && i < Params.size()) {
			SemaParam *Param = Params[i];
			Param->getType()->accept(*CGM);
			llvm::Type *ExpectedType = Param->getType()->getCodeGen()->getType();

			// Convert value to expected type if needed
			if (V->getType() != ExpectedType) {
				if (V->getType()->isIntegerTy() && ExpectedType->isIntegerTy()) {
					unsigned SrcBits = V->getType()->getIntegerBitWidth();
					unsigned DstBits = ExpectedType->getIntegerBitWidth();
					if (SrcBits < DstBits) {
						// Check if source type is signed for proper extension
						SemaType *ArgType = ArgExpr->getType();
						bool isSigned = ArgType->isNumber() &&
							static_cast<SemaIntType *>(ArgType)->isSigned();
						V = isSigned ? Builder->CreateSExt(V, ExpectedType)
						             : Builder->CreateZExt(V, ExpectedType);
					} else if (SrcBits > DstBits) {
						V = Builder->CreateTrunc(V, ExpectedType);
					}
				} else if (V->getType()->isFloatingPointTy() && ExpectedType->isFloatingPointTy()) {
					V = Builder->CreateFPCast(V, ExpectedType);
				}
			}

			llvm::AllocaInst *TmpAlloca = Builder->CreateAlloca(ExpectedType);
			Builder->CreateStore(V, TmpAlloca);
			V = TmpAlloca;
		}

		Args.push_back(V);
	}
}

llvm::Value *CodeGenExpr::ConvertToBool(llvm::Value *V) {
	FLY_DEBUG_SCOPE_MSG("CodeGenExpr", "Convert", "FromVal=" << V << " to Bool Type=");

	if (V->getType()->isIntegerTy()) {
		if (V->getType()->getIntegerBitWidth() > 8) {
			llvm::Value *ZERO = llvm::ConstantInt::get(V->getType(), 0);
			return Builder->CreateICmpNE(V, ZERO);
		}
		return Builder->CreateTrunc(V, CodeGen::BoolTy);
	}
	if (V->getType()->isFloatingPointTy()) {
		llvm::Value *ZERO = llvm::ConstantFP::get(V->getType(), 0);
		return Builder->CreateFCmpUNE(V, ZERO);
	}
	if (V->getType()->isArrayTy()) {
		// TODO
		return nullptr;
	}
	if (V->getType()->isStructTy()) {
		// TODO
		return nullptr;
	}

	// default 0
	return llvm::ConstantInt::get(CodeGen::BoolTy, 0, false);
}

llvm::Value *CodeGenExpr::ConvertNumber(llvm::Value *V, SemaNumberType *Ty, bool IsSigned) {
	// Promote V to Type Ty
	if (Ty->isInteger()) {
		V  = ConvertToInteger(V, static_cast<SemaIntType *>(Ty)); // Implicit conversion
	} else if (Ty->isFloat()) {
		V = ConvertToFloat(V, static_cast<SemaFloatType *>(Ty), IsSigned); // Implicit conversion
	}
	return V;
}

llvm::Value *CodeGenExpr::ConvertToInteger(llvm::Value *V, SemaIntType *Ty) {
	if (V->getType()->isIntegerTy()) {
		switch (Ty->getIntKind()) {

			case SemaIntTypeKind::TYPE_BYTE:
				return Builder->CreateTrunc(V, CodeGen::Int8Ty);

			case SemaIntTypeKind::TYPE_USHORT:
			case SemaIntTypeKind::TYPE_SHORT:
				if (V->getType() == CodeGen::Int8Ty) {
					return Builder->CreateZExt(V, CodeGen::Int16Ty);
				}
				return Builder->CreateTrunc(V, CodeGen::Int16Ty);

			case SemaIntTypeKind::TYPE_UINT:
			case SemaIntTypeKind::TYPE_INT:
				if (V->getType() == CodeGen::Int8Ty || V->getType() == CodeGen::Int16Ty) {
					return Ty->isSigned() ? Builder->CreateSExt(V, CodeGen::Int32Ty) :
						   Builder->CreateZExt(V, CodeGen::Int32Ty);
				}
				return Builder->CreateTrunc(V, CodeGen::Int32Ty);

			case SemaIntTypeKind::TYPE_ULONG:
			case SemaIntTypeKind::TYPE_LONG:
				if (V->getType() == CodeGen::Int8Ty || V->getType() == CodeGen::Int16Ty || V->getType() == CodeGen::Int32Ty) {
					return Ty->isSigned() ? Builder->CreateSExt(V, CodeGen::Int64Ty) :
						   Builder->CreateZExt(V, CodeGen::Int64Ty);
				}
				// V is already i64 — no conversion needed (covers long→ulong and ulong→long)
				return V;
		}
	}

	if (V->getType()->isFloatingPointTy()) {
		switch (Ty->getIntKind()) {

			case SemaIntTypeKind::TYPE_BYTE:
				return Builder->CreateFPToUI(V, CodeGen::Int8Ty);

			case SemaIntTypeKind::TYPE_USHORT:
			case SemaIntTypeKind::TYPE_SHORT:
				return Ty->isSigned() ? Builder->CreateFPToSI(V, CodeGen::Int16Ty) :
					   Builder->CreateFPToUI(V, CodeGen::Int16Ty);

			case SemaIntTypeKind::TYPE_UINT:
			case SemaIntTypeKind::TYPE_INT:
				return Ty->isSigned() ? Builder->CreateFPToSI(V, CodeGen::Int32Ty) :
					   Builder->CreateFPToUI(V, CodeGen::Int32Ty);

			case SemaIntTypeKind::TYPE_ULONG:
			case SemaIntTypeKind::TYPE_LONG:
				return Ty->isSigned() ? Builder->CreateFPToSI(V, CodeGen::Int64Ty) :
					   Builder->CreateFPToUI(V, CodeGen::Int64Ty);
		}
	}

	// V is already the target type or an unhandled type — return as-is
	return V;
}

llvm::Value *CodeGenExpr::ConvertToFloat(llvm::Value *V, SemaFloatType *Ty, bool IsSigned) {
	if (V->getType()->isFloatingPointTy()) {
		switch (Ty->getFloatKind()) {

			case SemaFloatTypeKind::TYPE_FLOAT:
				return Builder->CreateFPCast(V, CodeGen::FloatTy);

			case SemaFloatTypeKind::TYPE_DOUBLE:
				return Builder->CreateFPCast(V, CodeGen::DoubleTy);
		}
	}
	if (V->getType()->isIntegerTy()) {
		switch (Ty->getFloatKind()) {

			case SemaFloatTypeKind::TYPE_FLOAT:
				return IsSigned ? Builder->CreateSIToFP(V, CodeGen::FloatTy)
				                : Builder->CreateUIToFP(V, CodeGen::FloatTy);

			case SemaFloatTypeKind::TYPE_DOUBLE:
				return IsSigned ? Builder->CreateSIToFP(V, CodeGen::DoubleTy)
				                : Builder->CreateUIToFP(V, CodeGen::DoubleTy);
		}
	}
	return V;
}

