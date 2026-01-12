//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGExpr.cpp - Code Generator Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenExpr.h"

#include "AST/ASTCast.h"
#include "AST/ASTExpr.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTOp.h"
#include "Basic/Debug.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaCast.h"

#include "llvm/ADT/SmallVector.h"

#include <AST/ASTArg.h>
#include <AST/ASTCall.h>
#include <CodeGen/CodeGenClass.h>
#include <CodeGen/CodeGenVarBase.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaClassMethod.h>
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaFunctionBase.h>
#include <Sema/SemaMemberVar.h>
#include <Sema/SemaType.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaVar.h>

using namespace fly;

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM) : CGM(CGM) {
    FLY_DEBUG_START("CodeGenExpr", "CodeGenExpr");
}

llvm::Value *CodeGenExpr::GenExpr(SemaExpr *Sema) {
	switch (Sema->getKind()) {

		case SemaKind::VAR:
			return GenExpr(static_cast<SemaVar *>(Sema));

		case SemaKind::CALL:
			return GenExpr(static_cast<SemaCall *>(Sema));

		case SemaKind::VALUE:
			return GenExpr(static_cast<SemaValue *>(Sema));

		case SemaKind::UNARY:
			return GenExpr(static_cast<SemaUnary *>(Sema));

		case SemaKind::BINARY:
			return GenExpr(static_cast<SemaBinary *>(Sema));

		case SemaKind::TERNARY:
			return GenExpr(static_cast<SemaTernary *>(Sema));

		case SemaKind::CAST:
			return GenExpr(static_cast<SemaCast *>(Sema));

		default:
			assert("Unknown Sema Expr Kind");
			return nullptr;
	}
}

/**
 * Generate a LLVM Constant Value
 * @param Type is the parsed SemaType
 * @param Sema need to be correctly configured or you need to call GenDefaultValue()
 * @return
 */
llvm::Value *CodeGenExpr::GenExpr(SemaValue *Sema) {
    FLY_DEBUG_START("CodeGenModule", "GenValue");
    assert(Sema && "Value has to be not empty");
	assert(Sema->getType() && "Type has to be not empty");

	SemaType *Type = Sema->getType();

	switch (Type->getTypeKind()) {

		// Boolean Value
		case SemaTypeKind::TYPE_BOOL:
			return static_cast<SemaBoolValue *>(Sema)->getValue() ?
					llvm::ConstantInt::getTrue(CGM->LLVMCtx) : llvm::ConstantInt::getFalse(CGM->LLVMCtx);

		// Integer Value
		case SemaTypeKind::TYPE_INTEGER: {
			SemaIntValue * IntValue = static_cast<SemaIntValue *>(Sema);
			SemaIntTypeKind IntKind = static_cast<SemaIntType *>(Type)->getIntKind();
			switch (IntKind) {
				case SemaIntTypeKind::TYPE_BYTE:
					return llvm::ConstantInt::get(CGM->LLVMCtx, IntValue->getValue().trunc(8));
				case SemaIntTypeKind::TYPE_USHORT:
				case SemaIntTypeKind::TYPE_SHORT:
					return llvm::ConstantInt::get(CGM->LLVMCtx, IntValue->getValue().trunc(16));
				case SemaIntTypeKind::TYPE_UINT:
				case SemaIntTypeKind::TYPE_INT:
					return llvm::ConstantInt::get(CGM->LLVMCtx, IntValue->getValue().trunc(32));
				case SemaIntTypeKind::TYPE_ULONG:
				case SemaIntTypeKind::TYPE_LONG:
					return llvm::ConstantInt::get(CGM->LLVMCtx, IntValue->getValue().trunc(64));
			}
		} break;

		// Floating Point Value
		case SemaTypeKind::TYPE_FLOATING_POINT: {
			SemaFloatValue *FloatValue = static_cast<SemaFloatValue *>(Sema);
			SemaFloatTypeKind FPKind = static_cast<SemaFloatType *>(Type)->getFPKind();
			switch (FPKind) {
				case SemaFloatTypeKind::TYPE_FLOAT:
					return llvm::ConstantFP::get(CGM->FloatTy, FloatValue->getValue());
				case SemaFloatTypeKind::TYPE_DOUBLE:
					return llvm::ConstantFP::get(CGM->DoubleTy, FloatValue->getValue());
			}
		} break;

		// Strig Value
		case SemaTypeKind::TYPE_STRING: {
			return CGM->Builder->CreateGlobalStringPtr(static_cast<SemaStringValue *>(Sema)->getValue());
		}

		// Array Value
		case SemaTypeKind::TYPE_ARRAY: {
			SemaArrayValue *ArrayValue = static_cast<SemaArrayValue *>(Sema);

			llvm::PointerType *AllocType = CGM->GenArrayType((SemaArrayType *) Type);
			std::vector<llvm::Value *> Values;
			for (SemaValue *Value : ArrayValue->getValues()) {
				llvm::Value * V = GenExpr(Value);
				Values.push_back(V);
			}

			// Calculate Space
			llvm::Value* AllocSize = llvm::ConstantInt::get(CGM->IntPtrTy, 0);
			if (Values.size() > 0) {
				llvm::Value* NumElements = llvm::ConstantInt::get(CGM->IntPtrTy, Values.size());
				llvm::TypeSize SizeInBytes = CGM->Target.getDataLayout().getTypeAllocSize(Values[0]->getType());
				llvm::Value* ElementSize = llvm::ConstantInt::get(CGM->IntPtrTy, SizeInBytes); // sizeof(int32)
				AllocSize = CGM->Builder->CreateMul(NumElements, ElementSize);
			}

			// @malloc data type struct
			llvm::Instruction *I = llvm::CallInst::CreateMalloc(CGM->Builder->GetInsertBlock(), CGM->IntPtrTy,
														  AllocType, AllocSize, nullptr, nullptr);
			llvm::Value * Instance = CGM->Builder->Insert(I);

			return Instance;
		} break;

		case SemaTypeKind::TYPE_CLASS:
			SemaStructValue *StructValue = static_cast<SemaStructValue *>(Sema);
            break;
	}

    assert(0 && "Unknown Type");
	return nullptr;
}

llvm::Value *CodeGenExpr::GenExpr(SemaVar *Sema) {

	// Class Instance
	if (Sema->getVarKind() == SemaVarKind::CLASS_INSTANCE) {
		SemaClassInstance *Instance = static_cast<SemaClassInstance *>(Sema);

		// Check if the Instance belong to a base class, need to set the pointer to the main instance
		if (Instance->getParent()) {
			Instance->getCodeGen()->setPointer(Instance->getParent()->getCodeGen()->getPointer());
		}
	}

	// Member Variable
	else if (Sema->getVarKind() == SemaVarKind::MEMBER_VAR) {
		SemaMemberVar *MemberVar = static_cast<SemaMemberVar *>(Sema);

		llvm::Value *Pointer = GenExpr(MemberVar->getParent());
		llvm::Type *Ty = CGM->GenType(MemberVar->getType());
		size_t Index = MemberVar->getClassAttribute()->getCodeGen()->getIndex();
		CodeGenVar *CGV = new CodeGenVar(CGM, Sema, Ty, Index);
		CGV->setPointer(Pointer);
		MemberVar->setCodeGen(CGV);
	}

	// Class Attribute
	else if (Sema->getVarKind() == SemaVarKind::CLASS_ATTRIBUTE) {
		SemaClassAttribute * ClassAttribute = static_cast<SemaClassAttribute *>(Sema);

		// Check if the ClassAttribute is a static attribute
		if (!ClassAttribute->isStatic()) {
			llvm::Value *ParentPointer = ClassAttribute->getClass().getThis()->getCodeGen()->getValue();
			ClassAttribute->getCodeGen()->setPointer(ParentPointer);
		}
	}

	// Enum Entry
	else if (Sema->getVarKind() == SemaVarKind::ENUM_ENTRY) {
		// TODO
	}

	// No Parent
	else {
		// Generate CodeGenVar if not already generated
		if (Sema->getCodeGen() == nullptr) {
			llvm::Type *Ty = CGM->GenType(Sema->getType());
			Sema->setCodeGen(new CodeGenVar(CGM, Sema, Ty));
		}
	}

	return Sema->getCodeGen()->getValue();
}


llvm::Value *CodeGenExpr::GenExpr(SemaCall *Sema) {

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
        if (Method->getClass()->getClassKind() != SemaClassKind::STRUCT)
            Args.push_back(Sema->getErrorHandler()->getCodeGen()->getValue()); // Error is a Pointer

    	// Get Class CodeGen
    	CodeGenClass * CGClass = Method->getClass()->getCodeGen();

		// Call class constructor
    	if (Method->isConstructor()) {

    		// Allocate memory for the new instance in InstancePtr
    		if (Sema->getAST().getCallKind() == ASTCallKind::CALL_NEW) {
    			InstancePtr = CGClass->NewInstance();
    			CGM->Builder->Insert(InstancePtr);

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
    			CGM->Builder->CreateCall(Method->getCodeGen()->getFunction(), Args);

    		return InstancePtr;
    	}

    	// Call is not a constructor, so it must be a method call
    	if (Sema->getParent()) {

    		SemaClassType * ParentClass = static_cast<SemaClassType *>(Sema->getParent()->getType());
    		if (Method->getClass()->isBase(ParentClass)) {
    			// Instance of base class method call
    			InstancePtr = GenExpr(Sema->getParent());

    			// Get the base class instance pointer
    			InstancePtr = ParentClass->getCodeGen()->getBaseInstance(InstancePtr, Method->getClass());
    		} else {
    			// Instance of class method call
    			InstancePtr = GenExpr(Sema->getParent());
    		}

    	} else {
    		// Direct method call inside a class
    		InstancePtr = Method->getClass()->getThis()->getCodeGen()->getValue();
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
    		llvm::Value * VTablePtrPtr = CGM->Builder->CreateStructGEP(CGClass->getType(), InstancePtr, 0);
    		llvm::LoadInst * VTablePtr = CGM->Builder->CreateLoad(VTablePtrPtr);

    		// TODO
    		// Calculate the offset of the method in the VTable

    		// Get the Method index in the VTable
    		// %fn1_ptr = getelementptr i8*, i8** %vptr1, i64 1
    		// %fn1_i8 = load i8*, i8** %fn1_ptr
    		// %fn1 = bitcast i8* %fn1_i8 to void (%class.Base1*)*
    		llvm::Value * FuncPtrPtr = CGM->Builder->CreateGEP(CGM->Int8PtrTy, VTablePtr, llvm::ConstantInt::get(CGM->Int64Ty, Method->getCodeGen()->getIndex()));
    		FuncPtr = CGM->Builder->CreateLoad(FuncPtrPtr);
			FuncPtr = CGM->Builder->CreateBitCast(FuncPtr, Method->getCodeGen()->getFunction()->getType());

    	} else {
    		FuncPtr = Method->getCodeGen()->getFunction();
    	}

    	// Create the function call
    	return CGM->Builder->CreateCall(Method->getCodeGen()->getFunction()->getFunctionType(), FuncPtr, Args);

    } else {

    	// Add Error parameter
        Args.push_back(Sema->getErrorHandler()->getCodeGen()->getValue()); // Error is a Pointer
    	addArgs(Sema, Args);

    	CodeGenFunctionBase *CGF = Sema->getFunction()->getCodeGen();
    	return CGM->Builder->CreateCall(CGF->getFunction(), Args);
    }
}


llvm::Value * CodeGenExpr::GenExpr(SemaCast *Sema) {
	ASTCast &AST = Sema->getAST();
	SemaType *ToType = AST.getToType()->getSema();
	llvm::Value *V = GenExpr(Sema);
	switch (AST.getType()->getTypeKind()) {

		case SemaTypeKind::TYPE_VOID:
		case SemaTypeKind::TYPE_ERROR:
		case SemaTypeKind::TYPE_ENUM:
			// TODO: Void, Error, Enum cast is not supported
			break;
		case SemaTypeKind::TYPE_BOOL:
			// TODO
				break;
		case SemaTypeKind::TYPE_INTEGER:
			// TODO
				break;
		case SemaTypeKind::TYPE_FLOATING_POINT:
			// TODO
				break;
		case SemaTypeKind::TYPE_STRING:
			// TODO
				break;
		case SemaTypeKind::TYPE_ARRAY:
			// TODO
				break;
		case SemaTypeKind::TYPE_CLASS:
			// TODO
				break;
	}

	return V;
}

llvm::Value *CodeGenExpr::GenExpr(SemaUnary *Sema) {
    FLY_DEBUG_START("CodeGenExpr", "GenUnary");
	ASTUnaryOp &Unary = Sema->getAST();
    assert(Unary.getExprKind() == ASTExprKind::EXPR_UNARY && "Expected Unary Group Expr");
    assert(Unary.getExpr() && "Unary Expr empty");

    llvm::Value *OldVal = GenExpr(Unary.getExpr()->getSema());
	llvm::Value *NewVal = nullptr;
	llvm::Value *Result = nullptr;

    switch (Unary.getOpKind()) {

        case ASTUnaryOpKind::OP_UNARY_PRE_INCR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
            NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            Result = NewVal;
        }
        case ASTUnaryOpKind::OP_UNARY_POST_INCR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
            NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            Result = OldVal;
        }
        case ASTUnaryOpKind::OP_UNARY_PRE_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
            NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
        	Result = NewVal;
        }
        case ASTUnaryOpKind::OP_UNARY_POST_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
            NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
        	Result = NewVal;
        }
        case ASTUnaryOpKind::OP_UNARY_NOT_LOG:
            OldVal = CGM->Builder->CreateTrunc(OldVal, CGM->BoolTy);
            OldVal = CGM->Builder->CreateXor(OldVal, true);
            return CGM->Builder->CreateZExt(OldVal, CGM->Int8Ty);
    }

	// Set Var with NewVal
	if (Unary.getExpr()->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
		ASTIdentifier *Identifier = static_cast<ASTIdentifier *>(Unary.getExpr());
		Identifier->getSema()->getCodeGen()->Store(NewVal);
	}

    return Result;
}

llvm::Value *CodeGenExpr::GenExpr(SemaBinary *Sema) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinary");
	ASTBinaryOp &Binary = Sema->getAST();
    assert(Binary.getExprKind() == ASTExprKind::EXPR_BINARY && "Expected Binary Group Expr");
    assert(Binary.getLeftExpr() && "First Expr is empty");
    assert(Binary.getRightExpr() && "Second Expr is empty");

    SemaExpr *Left = Binary.getLeftExpr()->getSema();
    SemaExpr *Right = Binary.getRightExpr()->getSema();
    ASTBinaryOpKind OpKind = Binary.getOpKind();

    switch (Binary.getBinaryKind()) {
        // Arithmetic operations
        case ASTBinaryKind::OP_BINARY_ARITH:
            return GenBinaryArith(Left, OpKind, Right);

        // Comparison operations
        case ASTBinaryKind::OP_BINARY_COMPARE:
            return GenBinaryComparison(Left, OpKind, Right);

        // Logical operations
        case ASTBinaryKind::OP_BINARY_LOGIC:
            return GenBinaryLogic(Left, OpKind, Right);

        // Assignment operations - not yet implemented
        case ASTBinaryKind::OP_BINARY_ASSIGN:
            return GenBinaryAssign(Left, OpKind, Right);
    }

    assert(0 && "Unknown Binary Operation");
    return nullptr;
}

llvm::Value *CodeGenExpr::GenBinaryArith(SemaExpr *E1, ASTBinaryOpKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryArith");
    llvm::Value *V1 = GenExpr(E1);
    llvm::Value *V2 = GenExpr(E2);

    // Convert E2 to E1 Type
    V2 = CGM->Convert(V2, E2->getType(), E1->getType()); // Implicit conversion

    switch (OperatorKind) {

        case ASTBinaryOpKind::OP_BINARY_ARITH_ADD:
            return CGM->Builder->CreateAdd(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_ARITH_SUB:
            return CGM->Builder->CreateSub(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_ARITH_MUL:
            return CGM->Builder->CreateMul(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_ARITH_DIV:
            return CGM->Builder->CreateSDiv(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_ARITH_MOD:
            return CGM->Builder->CreateSRem(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_ARITH_AND:
            return CGM->Builder->CreateAnd(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_ARITH_OR:
            return CGM->Builder->CreateOr(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_ARITH_XOR:
            return CGM->Builder->CreateXor(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_ARITH_SHIFT_L:
            return CGM->Builder->CreateShl(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_ARITH_SHIFT_R:
            return CGM->Builder->CreateAShr(V1, V2);
    }
    assert(0 && "Unknown Arith Operation");
}

llvm::Value *CodeGenExpr::GenBinaryComparison(SemaExpr *E1, ASTBinaryOpKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryComparison");
    llvm::Value *V1 = GenExpr(E1);
    llvm::Value *V2 = GenExpr(E2);
	SemaType *V1Type = E1->getType();
    SemaType *V2Type = E2->getType();

    if (E1->getType()->isBool() && E2->getType()->isBool()) {
        switch (OperatorKind) {
            case ASTBinaryOpKind::OP_BINARY_COMPARE_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
        }
    } else if (E1->getType()->isInteger() && E2->getType()->isInteger()) {
        bool Signed = static_cast<SemaIntType *>(E1->getType())->isSigned() ||
        	static_cast<SemaIntType *>(E2->getType())->isSigned();
        switch (OperatorKind) {

            case ASTBinaryOpKind::OP_BINARY_COMPARE_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_GT:
                return Signed ? CGM->Builder->CreateICmpSGT(V1, V2) : CGM->Builder->CreateICmpUGT(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_GTE:
                return Signed ? CGM->Builder->CreateICmpSGE(V1, V2) : CGM->Builder->CreateICmpUGE(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_LT:
                return Signed ? CGM->Builder->CreateICmpSLT(V1, V2) : CGM->Builder->CreateICmpULT(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_LTE:
                return Signed ? CGM->Builder->CreateICmpSLE(V1, V2) : CGM->Builder->CreateICmpULE(V1, V2);
        }
    } else {
        // Convert values to Float if one of them is Float
        if ( (V1->getType()->isFloatTy() || V1->getType()->isDoubleTy()) &&
             (V2->getType()->isIntegerTy() || V2->getType()->isIntegerTy()) ) {
            V2 = CGM->Convert(V2, V2Type, V1Type); // Explicit conversion
        } else if ( (V1->getType()->isIntegerTy() || V1->getType()->isIntegerTy()) &&
                    (V2->getType()->isFloatTy() || V2->getType()->isDoubleTy()) ) {
            V1 = CGM->Convert(V1, V2Type, V1Type); // Explicit conversion
        }
        switch (OperatorKind) {

            case ASTBinaryOpKind::OP_BINARY_COMPARE_EQ:
                return CGM->Builder->CreateFCmpOEQ(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_NE:
                return CGM->Builder->CreateFCmpONE(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_GT:
                return CGM->Builder->CreateFCmpOGT(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_GTE:
                return CGM->Builder->CreateFCmpOGE(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_LT:
                return CGM->Builder->CreateFCmpOLT(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_COMPARE_LTE:
                return CGM->Builder->CreateFCmpOLE(V1, V2);
        }
    }

    assert(0 && "Invalid Comparator Operator");
}

llvm::Value *CodeGenExpr::GenBinaryLogic(SemaExpr *E1, ASTBinaryOpKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryLogic");
    llvm::Value *V1 = GenExpr(E1);
    V1 = CGM->ConvertToBool(V1);
    V1 = CGM->Convert(V1, E1->getType(), SemaBuiltin::getBoolType());
    llvm::BasicBlock *FromBB = CGM->Builder->GetInsertBlock();

    switch (OperatorKind) {

        case ASTBinaryOpKind::OP_BINARY_LOGIC_AND: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", FromBB->getParent());
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", FromBB->getParent());

            // From Branch
            CGM->Builder->CreateCondBr(V1, LeftBB, RightBB);

            // Left Branch
            CGM->Builder->SetInsertPoint(LeftBB);
            llvm::Value *V2 = GenExpr(E2);
            V2 = CGM->ConvertToBool(V2);
            llvm::Value *V2Trunc = CGM->Builder->CreateTrunc(V2, CGM->BoolTy);
            CGM->Builder->CreateBr(RightBB);

            // Right Branch
            CGM->Builder->SetInsertPoint(RightBB);
            llvm::PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->BoolTy, false, false), FromBB);
            Phi->addIncoming(V2Trunc, LeftBB);
            return Phi;
        }
        case ASTBinaryOpKind::OP_BINARY_LOGIC_OR: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", FromBB->getParent());
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", FromBB->getParent());

            // From Branch
            CGM->Builder->CreateCondBr(V1, RightBB, LeftBB);

            // Left Branch
            CGM->Builder->SetInsertPoint(LeftBB);
            llvm::Value *V2 = GenExpr(E2);
            llvm::Value *V2Trunc = CGM->Builder->CreateTrunc(V2, CGM->BoolTy);
            CGM->Builder->CreateBr(RightBB);

            // Right Branch
            CGM->Builder->SetInsertPoint(RightBB);
            llvm::PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->BoolTy, true, false), FromBB);
            Phi->addIncoming(V2Trunc, LeftBB);
            return Phi;
        }
    }
    assert(0 && "Invalid Logic Operator");
}

llvm::Value * CodeGenExpr::GenBinaryAssign(SemaExpr *E1, ASTBinaryOpKind OperatorKind, SemaExpr *E2) {
	return static_cast<SemaVar *>(E1)->getCodeGen()->Store(GenExpr(E2));
}

llvm::Value *CodeGenExpr::GenExpr(SemaTernary *Sema) {
	ASTTernaryOp &Ternary = Sema->getAST();
    assert(Ternary.getConditionExpr() && "First Expr is empty");
    assert(Ternary.getTrueExpr() && "Second Expr is empty");
    assert(Ternary.getFalseExpr() && "Third Expr is empty");

    llvm::BasicBlock *FromBB = CGM->Builder->GetInsertBlock();
    llvm::Value *Cond = GenExpr(Ternary.getConditionExpr()->getSema());

    // Create Blocks
    llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "terntrue", FromBB->getParent());
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternfalse", FromBB->getParent());
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternend", FromBB->getParent());

    // Create Condition
    CGM->Builder->CreateCondBr(Cond, TrueBB, FalseBB);

    // True Label
    CGM->Builder->SetInsertPoint(TrueBB);
    llvm::Value *True = GenExpr(Ternary.getTrueExpr()->getSema());
    llvm::Value *BoolTrue = CGM->ConvertToBool(True);
    CGM->Builder->CreateBr(EndBB);

    // False Label
    CGM->Builder->SetInsertPoint(FalseBB);
    llvm::Value *False = GenExpr(Ternary.getFalseExpr()->getSema());
    llvm::Value *BoolFalse = CGM->ConvertToBool(False);
    CGM->Builder->CreateBr(EndBB);

    // End Label
    CGM->Builder->SetInsertPoint(EndBB);
    llvm::PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
    Phi->addIncoming(BoolTrue, TrueBB);
    Phi->addIncoming(BoolFalse, FalseBB);
    return Phi;
}

void CodeGenExpr::addArgs(SemaCall *Sema, llvm::SmallVector<llvm::Value *, 8> &Args) {
	// Add Call arguments to Function args
	for (ASTArg *Arg : Sema->getAST().getArgs()) {
		llvm::Value *V = GenExpr(Arg->getExpr()->getSema());
		Args.push_back(V);
	}
}