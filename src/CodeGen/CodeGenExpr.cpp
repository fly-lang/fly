//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGExpr.cpp - Code Generator Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenExpr.h"

#include "AST/ASTBinary.h"
#include "AST/ASTCast.h"
#include "AST/ASTExpr.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTTernary.h"
#include "AST/ASTUnary.h"
#include "Basic/Debug.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaBinary.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaCast.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaEnumEntry.h"
#include "Sema/SemaTernary.h"

#include "llvm/ADT/SmallVector.h"

#include <AST/ASTArg.h>
#include <AST/ASTCall.h>
#include <CodeGen/CodeGenClass.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaClassMethod.h>
#include <Sema/SemaError.h>
#include <Sema/SemaFunctionBase.h>
#include <Sema/SemaMember.h>
#include <Sema/SemaType.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaVar.h>

using namespace fly;

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM) : CodeGenBase(), CGM(CGM), Builder(CGM->getBuilder()) {
	FLY_DEBUG_START("CodeGenExpr", "CodeGenExpr");
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

void CodeGenExpr::GenExpr(SemaStringValue *Sema) {
	V = Builder->CreateGlobalStringPtr(Sema->getValue());
}

void CodeGenExpr::GenExpr(SemaStructValue *Sema) {
	// TODO: Implement StructValue CodeGen
}

void CodeGenExpr::GenExpr(SemaNullValue *Sema) {
	// TODO: Implement NullValue CodeGen
}

void CodeGenExpr::GenExpr(SemaUnsetValue *Sema) {
	// Generate a constant integer 0 for unset enum value
	V = llvm::ConstantInt::get(CodeGen::Int32Ty, 0);
}

void CodeGenExpr::GenExpr(SemaEnumEntry *Sema) {
	// TODO: Implement EnumEntry CodeGen
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
			llvm::Value *ParentPointer = ClassAttribute->getClass().getThis()->getCodeGen()->getValue();
			ClassAttribute->getCodeGen()->setPointer(ParentPointer);
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
        if (Method->getClass()->getClassKind() != SemaClassKind::STRUCT)
            Args.push_back(Sema->getErrorHandler()->getCodeGen()->getValue()); // Error is a Pointer

    	// Get Class CodeGen
    	CodeGenClass * CGClass = Method->getClass()->getCodeGen();

		// Call class constructor
    	if (Method->isConstructor()) {

    		// Allocate memory for the new instance in InstancePtr
    		if (Sema->getAST().getCallKind() == ASTCallKind::CALL_NEW) {
    			InstancePtr = CGClass->NewInstance();
    			Builder->Insert(InstancePtr);

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
    		return;
    	}

    	// Call is not a constructor, so it must be a method call
    	if (Sema->getParent()) {

    		SemaClassType * ParentClass = static_cast<SemaClassType *>(Sema->getParent()->getType());
    		if (Method->getClass()->isBase(ParentClass)) {
    			// Instance of base class method call
    			Sema->getParent()->accept(*CGM);
    			InstancePtr = Sema->getParent()->getCodeGen()->getValue();

    			// Get the base class instance pointer
    			InstancePtr = ParentClass->getCodeGen()->getBaseInstance(InstancePtr, Method->getClass());
    		} else {
    			// Instance of class method call
    			Sema->getParent()->accept(*CGM);
    			InstancePtr = Sema->getParent()->getCodeGen()->getValue();
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
    		llvm::Value * VTablePtrPtr = Builder->CreateStructGEP(CGClass->getType(), InstancePtr, 0);
    		llvm::LoadInst * VTablePtr = Builder->CreateLoad(VTablePtrPtr);

    		// TODO
    		// Calculate the offset of the method in the VTable

    		// Get the Method index in the VTable
    		// %fn1_ptr = getelementptr i8*, i8** %vptr1, i64 1
    		// %fn1_i8 = load i8*, i8** %fn1_ptr
    		// %fn1 = bitcast i8* %fn1_i8 to void (%class.Base1*)*
    		llvm::Value * FuncPtrPtr = Builder->CreateGEP(CodeGen::Int8PtrTy, VTablePtr, llvm::ConstantInt::get(CodeGen::Int64Ty, Method->getCodeGen()->getIndex()));
    		FuncPtr = Builder->CreateLoad(FuncPtrPtr);
			FuncPtr = Builder->CreateBitCast(FuncPtr, Method->getCodeGen()->getFunction()->getType());

    	} else {
    		FuncPtr = Method->getCodeGen()->getFunction();
    	}

    	// Create the function call
    	V = Builder->CreateCall(Method->getCodeGen()->getFunction()->getFunctionType(), FuncPtr, Args);

    } else {

    	// Add Error parameter
        Args.push_back(CGM->CurrentErrorHandler->getValue()); // Error is a Pointer
    	addArgs(Sema, Args);

    	llvm::Function *Fn = Sema->getFunction()->getCodeGen()->getFunction();
    	V = Builder->CreateCall(Fn, Args);
    }
}

void CodeGenExpr::GenExpr(SemaMember *Sema) {
	// Generate Parent
	Sema->getParent()->accept(*CGM);

	// Get Pointer to Parent
	llvm::Value *Pointer = Sema->getParent()->getCodeGen()->getValue();

	Sema->getType()->accept(*CGM);
	llvm::Type *Ty = Sema->getType()->getCodeGen()->getType();
	SemaExpr *Ref = Sema->getRef();
	if (Ref->getKind() == SemaKind::ATTRIBUTE) {
		SemaClassAttribute *Attr = static_cast<SemaClassAttribute *>(Ref);
		size_t Index = Attr->getCodeGen()->getIndex();
		CodeGenVar *CGV = new CodeGenVar(CGM, Attr, Ty, Index);
		CGV->setPointer(Pointer);
		Sema->setCodeGen(CGV);
		V = CGV->getValue();
	} else if (Ref->getKind() == SemaKind::ENUM_ENTRY) {
		Ref->accept(*CGM);
		V = Ref->getCodeGen()->getValue();
	} else {
		CGM->Diag(diag::err_invalid_behavior);
	}
}

void CodeGenExpr::GenExpr(SemaCast *Sema) {
	ASTCast &AST = Sema->getAST();
	SemaType *ToType = AST.getToType()->getSema();
	switch (AST.getType()->getKind()) {

		case SemaKind::TYPE_VOID:
		case SemaKind::TYPE_ERROR:
		case SemaKind::TYPE_ENUM:
			// TODO: Void, Error, Enum cast is not supported
			break;
		case SemaKind::TYPE_BOOL:
			// TODO
				break;
		case SemaKind::TYPE_INTEGER:
			// TODO
				break;
		case SemaKind::TYPE_FLOAT:
			// TODO
				break;
		case SemaKind::TYPE_STRING:
			// TODO
				break;
		case SemaKind::TYPE_ARRAY:
			// TODO
				break;
		case SemaKind::TYPE_CLASS:
			// TODO
				break;
	}
}

void CodeGenExpr::GenExpr(SemaUnary *Sema) {
    FLY_DEBUG_START("CodeGenExpr", "GenUnary");
	ASTUnary &Unary = Sema->getAST();
    assert(Unary.getExprKind() == ASTExprKind::EXPR_UNARY && "Expected Unary Group Expr");
    assert(Unary.getExpr() && "Unary Expr empty");

	Unary.getExpr()->getSema()->accept(*CGM);
    llvm::Value *OldVal = Unary.getExpr()->getSema()->getCodeGen()->getValue();
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

	// Set Var with NewVal
	if (Unary.getExpr()->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
		ASTIdentifier *Identifier = static_cast<ASTIdentifier *>(Unary.getExpr());
		Identifier->getSema()->getCodeGen()->Store(NewVal);
	}
}

void CodeGenExpr::GenExpr(SemaBinary *Sema) {
	FLY_DEBUG_START("CodeGenExpr", "GenBinary");
	ASTBinary &Binary = Sema->getAST();
	assert(Binary.getExprKind() == ASTExprKind::EXPR_BINARY && "Expected Binary Group Expr");
	assert(Binary.getLeftExpr() && "First Expr is empty");
	assert(Binary.getRightExpr() && "Second Expr is empty");

	SemaExpr *Left = Binary.getLeftExpr()->getSema();
	SemaExpr *Right = Binary.getRightExpr()->getSema();
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
	assert(Ternary.getConditionExpr() && "First Expr is empty");
	assert(Ternary.getTrueExpr() && "Second Expr is empty");
	assert(Ternary.getFalseExpr() && "Third Expr is empty");

	llvm::BasicBlock *FromBB = Builder->GetInsertBlock();
	Ternary.getConditionExpr()->getSema()->accept(*CGM);
	llvm::Value *Cond = Ternary.getConditionExpr()->getSema()->getCodeGen()->getValue();

	// Create Blocks
	llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "terntrue", FromBB->getParent());
	llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternfalse", FromBB->getParent());
	llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternend", FromBB->getParent());

	// Create Condition
	Builder->CreateCondBr(Cond, TrueBB, FalseBB);

	// True Label
	Builder->SetInsertPoint(TrueBB);
	Ternary.getTrueExpr()->getSema()->accept(*CGM);
	llvm::Value *True = Ternary.getTrueExpr()->getSema()->getCodeGen()->getValue();
	llvm::Value *BoolTrue = ConvertToBool(True);
	Builder->CreateBr(EndBB);

	// False Label
	Builder->SetInsertPoint(FalseBB);
	Ternary.getFalseExpr()->getSema()->accept(*CGM);
	llvm::Value *False = Ternary.getFalseExpr()->getSema()->getCodeGen()->getValue();
	llvm::Value *BoolFalse = ConvertToBool(False);
	Builder->CreateBr(EndBB);

	// End Label
	Builder->SetInsertPoint(EndBB);
	llvm::PHINode *Phi = Builder->CreatePHI(CodeGen::BoolTy, 2);
	Phi->addIncoming(BoolTrue, TrueBB);
	Phi->addIncoming(BoolFalse, FalseBB);
	V = Phi;
}

llvm::Value *CodeGenExpr::GenBinaryArith(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryArith");
	SemaNumberType *Type1 = static_cast<SemaNumberType *>(E1->getType());
	SemaNumberType *Type2 = static_cast<SemaNumberType *>(E2->getType());

	// Get Values
    llvm::Value *V1 = E1->getCodeGen()->getValue();
    llvm::Value *V2 = E2->getCodeGen()->getValue();

	// Promotion rules: convert Type1 or Type2 to the max type
	if (Type1->getRank() > Type2->getRank()) {
		V2 = ConvertNumber(V2, Type1); // Implicit conversion
	} else if (Type1->getRank() < Type2->getRank()) {
		// Promote T1 to T2 Type
		V1 = ConvertNumber(V1, Type2); // Implicit conversion
	}

    switch (OperatorKind) {

        case ASTBinaryKind::OP_BINARY_ARITH_ADD:
            return Builder->CreateAdd(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_SUB:
            return Builder->CreateSub(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_MUL:
            return Builder->CreateMul(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_DIV:
            return Builder->CreateSDiv(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_MOD:
            return Builder->CreateSRem(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_AND:
            return Builder->CreateAnd(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_OR:
            return Builder->CreateOr(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_XOR:
            return Builder->CreateXor(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_SHIFT_L:
            return Builder->CreateShl(V1, V2);
        case ASTBinaryKind::OP_BINARY_ARITH_SHIFT_R:
            return Builder->CreateAShr(V1, V2);
    }

	// Error
	CGM->Diag(diag::err_invalid_behavior);
	return nullptr;
}

llvm::Value *CodeGenExpr::GenBinaryCompare(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryComparison");
	SemaType *Type1 = E1->getType();
	SemaType *Type2 = E2->getType();

	// Get Values
    llvm::Value *V1 = E1->getCodeGen()->getValue();
    llvm::Value *V2 = E2->getCodeGen()->getValue();

    if (Type1->isBool() && Type2->isBool()) {
	    switch (OperatorKind) {
	    	case ASTBinaryKind::OP_BINARY_COMPARE_EQ:
	    		return Builder->CreateICmpEQ(V1, V2);
	    	case ASTBinaryKind::OP_BINARY_COMPARE_NE:
	    		return Builder->CreateICmpNE(V1, V2);
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

	// Error
	CGM->Diag(diag::err_invalid_behavior);
	return nullptr;
}

llvm::Value *CodeGenExpr::GenBinaryLogic(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryLogic");

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

	llvm::Value *V2 = E2CodeGen->getValue();
	if (E1->getType()->isNumber() && E2->getType()->isNumber()) {
		SemaNumberType *Type1 = static_cast<SemaNumberType *>(E1->getType());
		SemaNumberType *Type2 = static_cast<SemaNumberType *>(E2->getType());

		// Promotion rules: convert Type1 or Type2 to the max type
		if (Type1->getRank() > Type2->getRank()) {

			V2 = ConvertNumber(V2, Type1); // Implicit conversion
		}
	}

	return static_cast<SemaVar *>(E1)->getCodeGen()->Store(V2);
}

void CodeGenExpr::addArgs(SemaCall *Sema, llvm::SmallVector<llvm::Value *, 8> &Args) {
	// Add Call arguments to Function args
	for (ASTArg *Arg : Sema->getAST().getArgs()) {
		Arg->getExpr()->getSema()->accept(*CGM);
		llvm::Value *V = Arg->getExpr()->getSema()->getCodeGen()->getValue();
		Args.push_back(V);
	}
}

llvm::Value *CodeGenExpr::ConvertToBool(llvm::Value *V) {
	FLY_DEBUG_START_MSG("CodeGenExpr", "Convert", "FromVal=" << V << " to Bool Type=");

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

llvm::Value *CodeGenExpr::ConvertNumber(llvm::Value *V, SemaNumberType *Ty) {
	// Promote V to Type Ty
	if (Ty->isInteger()) {
		V  = ConvertToInteger(V, static_cast<SemaIntType *>(Ty)); // Implicit conversion
	} else if (Ty->isFloat()) {
		V = ConvertToFloat(V, static_cast<SemaFloatType *>(Ty)); // Implicit conversion
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
				break;
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
}

llvm::Value *CodeGenExpr::ConvertToFloat(llvm::Value *V, SemaFloatType *Ty) {
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

		}
	}
}
