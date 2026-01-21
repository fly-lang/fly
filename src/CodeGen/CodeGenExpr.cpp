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
#include "Sema/SemaEnumValue.h"
#include "Sema/SemaTernary.h"
#include "Sema/SemaUnary.h"

#include "llvm/ADT/SmallVector.h"

#include <AST/ASTArg.h>
#include <AST/ASTCall.h>
#include <CodeGen/CodeGenClass.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassInstance.h>
#include <Sema/SemaClassMethod.h>
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaFunctionBase.h>
#include <Sema/SemaMember.h>
#include <Sema/SemaType.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaVar.h>

using namespace fly;

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM) : CodeGenBase(), CGM(CGM), Builder(Builder) {
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
	SemaFloatTypeKind FPKind = static_cast<SemaFloatType *>(Type)->getFPKind();
	switch (FPKind) {
		case SemaFloatTypeKind::TYPE_FLOAT:
			V = llvm::ConstantFP::get(CGM->FloatTy, Sema->getValue());
			break;
		case SemaFloatTypeKind::TYPE_DOUBLE:
			V = llvm::ConstantFP::get(CGM->DoubleTy, Sema->getValue());
			break;
	}
}

void CodeGenExpr::GenExpr(SemaStringValue *Sema) {
	V = Builder->CreateGlobalStringPtr(Sema->getValue());
}

void CodeGenExpr::GenExpr(SemaArrayValue *Sema) {
	Sema->getType()->accept(*CGM);
	llvm::Type *ElementType = Sema->getType()->getCodeGen()->getType();
	std::vector<llvm::Value *> Values;
	for (SemaValue *Value : Sema->getValues()) {
		Value->accept(*CGM);
		llvm::Value *V = Value->getCodeGen()->getValue();
		Values.push_back(V);
	}

	// Calculate Space
	llvm::Value* AllocSize = llvm::ConstantInt::get(CGM->IntPtrTy, 0);
	if (Values.size() > 0) {
		llvm::Value* NumElements = llvm::ConstantInt::get(CGM->IntPtrTy, Values.size());
		llvm::TypeSize SizeInBytes = CGM->Target.getDataLayout().getTypeAllocSize(Values[0]->getType());
		llvm::Value* ElementSize = llvm::ConstantInt::get(CGM->IntPtrTy, SizeInBytes); // sizeof(int32)
		AllocSize = Builder->CreateMul(NumElements, ElementSize);
	}

	// @malloc data type struct
	llvm::Instruction *I = llvm::CallInst::CreateMalloc(Builder->GetInsertBlock(), CGM->IntPtrTy,
												  ElementType, AllocSize, nullptr, nullptr);
	V = Builder->Insert(I);
}

void CodeGenExpr::GenExpr(SemaStructValue *Sema) {
	
}

void CodeGenExpr::GenExpr(SemaNullValue *Sema) {

}

void CodeGenExpr::GenExpr(SemaEnumValue *Sema) {

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
    		llvm::Value * FuncPtrPtr = Builder->CreateGEP(CGM->Int8PtrTy, VTablePtr, llvm::ConstantInt::get(CGM->Int64Ty, Method->getCodeGen()->getIndex()));
    		FuncPtr = Builder->CreateLoad(FuncPtrPtr);
			FuncPtr = Builder->CreateBitCast(FuncPtr, Method->getCodeGen()->getFunction()->getType());

    	} else {
    		FuncPtr = Method->getCodeGen()->getFunction();
    	}

    	// Create the function call
    	V = Builder->CreateCall(Method->getCodeGen()->getFunction()->getFunctionType(), FuncPtr, Args);

    } else {

    	// Add Error parameter
        Args.push_back(Sema->getErrorHandler()->getCodeGen()->getValue()); // Error is a Pointer
    	addArgs(Sema, Args);

    	CodeGenFunctionBase *CGF = Sema->getFunction()->getCodeGen();
    	V = Builder->CreateCall(CGF->getFunction(), Args);
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
	} else if (Ref->getKind() == SemaKind::ENUM_VALUE) {
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
		case SemaKind::TYPE_FLOATING_POINT:
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
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
            NewVal = Builder->CreateNSWAdd(OldVal, RHS);
            V = NewVal;
        } break;
        case ASTUnaryKind::OP_UNARY_POST_INCR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
            NewVal = Builder->CreateNSWAdd(OldVal, RHS);
            V = OldVal;
        } break;
        case ASTUnaryKind::OP_UNARY_PRE_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
            NewVal = Builder->CreateNSWAdd(OldVal, RHS);
        	V = NewVal;
        } break;
        case ASTUnaryKind::OP_UNARY_POST_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
            NewVal = Builder->CreateNSWAdd(OldVal, RHS);
        	V = NewVal;
        } break;
        case ASTUnaryKind::OP_UNARY_NOT_LOG: {
	        OldVal = Builder->CreateTrunc(OldVal, CGM->BoolTy);
        	OldVal = Builder->CreateXor(OldVal, true);
        	V = Builder->CreateZExt(OldVal, CGM->Int8Ty);
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
	ASTBinaryKind OpKind = Binary.getOpKind();

	if (Binary.isArith()) {
		V = GenBinaryArith(Left, OpKind, Right);
	} else if (Binary.isCompare()) {
		V = GenBinaryComparison(Left, OpKind, Right);
	} else if (Binary.isLogic()) {
		V = GenBinaryLogic(Left, OpKind, Right);
	} else if (Binary.isAssign()) {
		V = GenBinaryAssign(Left, OpKind, Right);
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
	llvm::PHINode *Phi = Builder->CreatePHI(CGM->BoolTy, 2);
	Phi->addIncoming(BoolTrue, TrueBB);
	Phi->addIncoming(BoolFalse, FalseBB);
	V = Phi;
}

llvm::Value *CodeGenExpr::GenBinaryArith(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryArith");
	E1->accept(*CGM);
	E2->accept(*CGM);
    llvm::Value *V1 = E1->getCodeGen()->getValue();
    llvm::Value *V2 = E2->getCodeGen()->getValue();

    // Convert E2 to E1 Type
    V2 = Convert(V2, E2->getType(), E1->getType()); // Implicit conversion

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
    assert(0 && "Unknown Arith Operation");
}

llvm::Value *CodeGenExpr::GenBinaryComparison(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryComparison");
	E1->accept(*CGM);
	E2->accept(*CGM);
    llvm::Value *V1 = E1->getCodeGen()->getValue();;
    llvm::Value *V2 = E2->getCodeGen()->getValue();;
	SemaType *V1Type = E1->getType();
    SemaType *V2Type = E2->getType();

    if (E1->getType()->isBool() && E2->getType()->isBool()) {
        switch (OperatorKind) {
            case ASTBinaryKind::OP_BINARY_COMPARE_EQ:
                return Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryKind::OP_BINARY_COMPARE_NE:
                return Builder->CreateICmpNE(V1, V2);
        }
    } else if (E1->getType()->isInteger() && E2->getType()->isInteger()) {
        bool Signed = static_cast<SemaIntType *>(E1->getType())->isSigned() ||
        	static_cast<SemaIntType *>(E2->getType())->isSigned();
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
    } else {
        // Convert values to Float if one of them is Float
        if ( (V1->getType()->isFloatTy() || V1->getType()->isDoubleTy()) &&
             (V2->getType()->isIntegerTy() || V2->getType()->isIntegerTy()) ) {
            V2 = Convert(V2, V2Type, V1Type); // Explicit conversion
        } else if ( (V1->getType()->isIntegerTy() || V1->getType()->isIntegerTy()) &&
                    (V2->getType()->isFloatTy() || V2->getType()->isDoubleTy()) ) {
            V1 = Convert(V1, V2Type, V1Type); // Explicit conversion
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

    assert(0 && "Invalid Comparator Operator");
}

llvm::Value *CodeGenExpr::GenBinaryLogic(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryLogic");
	E1->accept(*CGM);
    llvm::Value *V1 = E1->getCodeGen()->getValue();
    V1 = ConvertToBool(V1);
    V1 = Convert(V1, E1->getType(), SemaBuiltin::getBoolType());
    llvm::BasicBlock *FromBB = Builder->GetInsertBlock();

    switch (OperatorKind) {

        case ASTBinaryKind::OP_BINARY_LOGIC_AND: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", FromBB->getParent());
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", FromBB->getParent());

            // From Branch
            Builder->CreateCondBr(V1, LeftBB, RightBB);

            // Left Branch
            Builder->SetInsertPoint(LeftBB);
        	E2->accept(*CGM);
            llvm::Value *V2 = E2->getCodeGen()->getValue();
            V2 = ConvertToBool(V2);
            llvm::Value *V2Trunc = Builder->CreateTrunc(V2, CGM->BoolTy);
            Builder->CreateBr(RightBB);

            // Right Branch
            Builder->SetInsertPoint(RightBB);
            llvm::PHINode *Phi = Builder->CreatePHI(CGM->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->BoolTy, false, false), FromBB);
            Phi->addIncoming(V2Trunc, LeftBB);
            return Phi;
        }
        case ASTBinaryKind::OP_BINARY_LOGIC_OR: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", FromBB->getParent());
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", FromBB->getParent());

            // From Branch
            Builder->CreateCondBr(V1, RightBB, LeftBB);

            // Left Branch
            Builder->SetInsertPoint(LeftBB);
        	E2->accept(*CGM);
            llvm::Value *V2 = E2->getCodeGen()->getValue();
            llvm::Value *V2Trunc = Builder->CreateTrunc(V2, CGM->BoolTy);
            Builder->CreateBr(RightBB);

            // Right Branch
            Builder->SetInsertPoint(RightBB);
            llvm::PHINode *Phi = Builder->CreatePHI(CGM->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->BoolTy, true, false), FromBB);
            Phi->addIncoming(V2Trunc, LeftBB);
            return Phi;
        }
    }
    assert(0 && "Invalid Logic Operator");
}

llvm::Value * CodeGenExpr::GenBinaryAssign(SemaExpr *E1, ASTBinaryKind OperatorKind, SemaExpr *E2) {
	E2->accept(*CGM);
	llvm::Value *V2 = E2->getCodeGen()->getValue();
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
	FLY_DEBUG_START_MSG("CodeGenExpr", "Convert",
					  "FromVal=" << V << " to Bool Type=");
	if (V->getType()->isIntegerTy()) {
		if (V->getType()->getIntegerBitWidth() > 8) {
			llvm::Value *ZERO = llvm::ConstantInt::get(V->getType(), 0);
			return Builder->CreateICmpNE(V, ZERO);
		} else {
			return Builder->CreateTrunc(V, CGM->BoolTy);
		}
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

	assert(false && "Unhandled Value Type");
}

llvm::Value *CodeGenExpr::Convert(llvm::Value *FromVal, SemaType *FromType, SemaType *ToType) {
    // FLY_DEBUG_START("CodeGenExpr", "Convert",
    //                   "Value=" << FromVal << " to ASTType=" << ToType->str());
    assert(ToType && "Invalid conversion type");

    llvm::Type *FromLLVMType = V->getType();
    switch (ToType->getKind()) {

        // to BOOL
        case SemaKind::TYPE_BOOL: {

            // from BOOL
            if (FromType->isBool()) {
                return Builder->CreateTrunc(FromVal, CGM->BoolTy);
            }

            // from Integer
            if (FromType->isInteger()) {
                llvm::Value *ZERO = llvm::ConstantInt::get(FromLLVMType, 0, ((SemaIntType *) FromType)->isSigned());
                return Builder->CreateICmpNE(FromVal, ZERO);
            }

            // from FLOATING POINT
            if (FromLLVMType->isFloatTy()) {
                llvm::Value *ZERO = llvm::ConstantFP::get(FromLLVMType, 0);
                return Builder->CreateFCmpUNE(FromVal, ZERO);
            }

            // default 0
            return llvm::ConstantInt::get(CGM->BoolTy, 0, false);
        }

            // to INTEGER
        case SemaKind::TYPE_INTEGER: {
            SemaIntType *IntegerType = (SemaIntType *) ToType;
            switch(IntegerType->getIntKind()) {

                // to INT 8
                case SemaIntTypeKind::TYPE_BYTE: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = Builder->CreateTrunc(FromVal, CGM->BoolTy);
                        return Builder->CreateZExt(ToVal, CGM->Int8Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == CGM->Int8Ty) {
                            return FromVal;
                        } else {
                            return Builder->CreateTrunc(FromVal, CGM->Int8Ty);
                        }
                    }

                    // from FLOATING POINT
                    if (FromLLVMType->isFloatingPointTy()) {
                        return Builder->CreateFPToUI(FromVal, CGM->Int8Ty);
                    }
                }

                    // to INT 16
                case SemaIntTypeKind::TYPE_SHORT:
                case SemaIntTypeKind::TYPE_USHORT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = Builder->CreateTrunc(FromVal, CGM->BoolTy);
                        return Builder->CreateZExt(ToVal, CGM->Int16Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == CGM->Int8Ty) {
                            return Builder->CreateZExt(FromVal, CGM->Int16Ty);
                        } else if (FromLLVMType == CGM->Int16Ty) {
                            return FromVal;
                        } else {
                            return Builder->CreateTrunc(FromVal, CGM->Int16Ty);
                        }
                    }

                    // from FLOATING POINT
                    if (FromLLVMType->isFloatingPointTy()) {
                        return IntegerType->isSigned() ? Builder->CreateFPToSI(FromVal, CGM->Int16Ty) :
                               Builder->CreateFPToUI(FromVal, CGM->Int16Ty);
                    }
                }

                    // to INT 32
                case SemaIntTypeKind::TYPE_INT:
                case SemaIntTypeKind::TYPE_UINT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = Builder->CreateTrunc(FromVal, CGM->BoolTy);
                        return Builder->CreateZExt(ToVal, CGM->Int32Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == CGM->Int8Ty || FromLLVMType == CGM->Int16Ty) {
                            return IntegerType->isSigned() ? Builder->CreateSExt(FromVal, CGM->Int32Ty) :
                                   Builder->CreateZExt(FromVal, CGM->Int32Ty);
                        } else if (FromLLVMType == CGM->Int32Ty) {
                            return FromVal;
                        } else {
                            return Builder->CreateTrunc(FromVal, CGM->Int32Ty);
                        }
                    }

                    // from FLOATING POINT
                    if (FromLLVMType->isFloatingPointTy()) {
                        return IntegerType->isSigned() ? Builder->CreateFPToSI(FromVal, CGM->Int32Ty) :
                               Builder->CreateFPToUI(FromVal, CGM->Int32Ty);
                    }
                }

                    // to INT 64
                case SemaIntTypeKind::TYPE_LONG:
                case SemaIntTypeKind::TYPE_ULONG: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = Builder->CreateTrunc(FromVal, CGM->BoolTy);
                        return Builder->CreateZExt(ToVal, CGM->Int64Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == CGM->Int8Ty || FromLLVMType == CGM->Int16Ty ||
                            FromLLVMType == CGM->Int32Ty) {
                            return IntegerType->isSigned() ? Builder->CreateSExt(FromVal, CGM->Int64Ty) :
                                   Builder->CreateZExt(FromVal, CGM->Int64Ty);
                        } else {
                            return FromVal;
                        }
                    }

                    // from FLOATING POINT
                    if (FromType->isFloatingPoint()) {
                        return IntegerType->isSigned() ? Builder->CreateFPToSI(FromVal, CGM->Int64Ty) :
                               Builder->CreateFPToUI(FromVal, CGM->Int64Ty);
                    }
                }
            }
        }

            // to FLOATING POINT
        case SemaKind::TYPE_FLOATING_POINT: {
            switch(((SemaFloatType *) ToType)->getFPKind()) {

                // to FLOAT 32
                case SemaFloatTypeKind::TYPE_FLOAT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        return Builder->CreateTrunc(FromVal, CGM->BoolTy);
                    }

                    // from INT
                    if (FromType->isInteger()) {
                        return ((SemaIntType *) FromType)->isSigned() ?
                               Builder->CreateSIToFP(FromVal, CGM->FloatTy) :
                               Builder->CreateUIToFP(FromVal, CGM->FloatTy);
                    }

                    // from FLOAT
                    if (FromType->isFloatingPoint()) {
                        switch (((SemaFloatType *) FromType)->getFPKind()) {

                            case SemaFloatTypeKind::TYPE_FLOAT:
                                return FromVal;
                            case SemaFloatTypeKind::TYPE_DOUBLE:
                                return Builder->CreateFPTrunc(FromVal, CGM->FloatTy);
                        }
                    }
                }

                    // to DOUBLE 64
                case SemaFloatTypeKind::TYPE_DOUBLE: {

                    // from BOOL
                    if (FromType->isBool()) {
                        return Builder->CreateTrunc(FromVal, CGM->BoolTy);
                    }

                    // from INT
                    if (FromType->isInteger()) {
                        return ((SemaIntType *) FromType)->isSigned() ?
                               Builder->CreateSIToFP(FromVal, CGM->DoubleTy) :
                               Builder->CreateUIToFP(FromVal, CGM->DoubleTy);
                    }

                    // from FLOAT
                    if (FromType->isFloatingPoint()) {
                        switch (((SemaFloatType *) FromType)->getFPKind()) {

                            case SemaFloatTypeKind::TYPE_FLOAT:
                                return Builder->CreateFPExt(FromVal, CGM->DoubleTy);
                            case SemaFloatTypeKind::TYPE_DOUBLE:
                                return FromVal;
                        }
                    }
                }
            }
        }

            // to Identity
        case SemaKind::TYPE_CLASS:
            return FromVal; // TODO implement class cast
    }
    assert(0 && "Conversion failed");
}