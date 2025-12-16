//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGExpr.cpp - Code Generator Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenExpr.h"
#include "AST/ASTExpr.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTOp.h"
#include "Basic/Debug.h"
#include "llvm/IR/Value.h"

#include <AST/ASTCall.h>
#include <AST/ASTCast.h>
#include <AST/ASTType.h>
#include <AST/ASTValue.h>
#include <CodeGen/CodeGenVarBase.h>
#include <Sema/SemaType.h>
#include <Sema/SemaVar.h>
#include <Sema/SemaValue.h>

using namespace fly;

llvm::Value *CodeGenExpr::Generate(CodeGenModule *CGM, ASTExpr *Expr) {
    FLY_DEBUG_START("CodeGenExpr", "Generate");
    CodeGenExpr CGE(CGM);
    return CGE.GenExpr(Expr);
}

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM) : CGM(CGM) {
    FLY_DEBUG_START("CodeGenExpr", "CodeGenExpr");
}

llvm::Value *CodeGenExpr::GenExpr(ASTExpr *Expr) {
    FLY_DEBUG_START_MSG("CodeGenExpr", "GenExpr", "Expr=" << Expr->str());
    switch (Expr->getExprKind()) {

        case ASTExprKind::EXPR_VALUE: {
            FLY_DEBUG_START_MSG("CodeGenExpr", "GenValue", "EXPR_VALUE");
        	ASTValue *Value = static_cast<ASTValue *>(Expr);
        	assert(Value && "Missing Value");
            return GenValue(Expr->getType(), Value->getSema());
        }

        case ASTExprKind::EXPR_IDENTIFIER: {
            FLY_DEBUG_START_MSG("CodeGenExpr", "GenValue", "EXPR_VAR_REF");
            ASTIdentifier *Identifier = static_cast<ASTIdentifier *>(Expr);
            assert(Identifier && "Missing Ref");
        	return CGM->GenVar(Identifier->getSema())->getValue();
        }

        case ASTExprKind::EXPR_CALL: {
            FLY_DEBUG_START_MSG("CodeGenExpr", "GenValue", "EXPR_CALL");
            ASTCall *Call = static_cast<ASTCall *>(Expr);
        	assert(Call && "Missing Call");
            return CGM->GenCall(Call->getSema());
        }

    	case ASTExprKind::EXPR_CAST: {
        	ASTCast * CastExpr = static_cast<ASTCast *>(Expr);
        	return CGM->GenCast(CastExpr->getExpr(), CastExpr->getCast()->getSema());
    	}

    	case ASTExprKind::EXPR_UNARY:
    		return GenUnary(static_cast<ASTUnaryOp *>(Expr));

    	case ASTExprKind::EXPR_BINARY:
    		return GenBinary(static_cast<ASTBinaryOp *>(Expr));

    	case ASTExprKind::EXPR_TERNARY:
    		return GenTernary(static_cast<ASTTernaryOp *>(Expr));
    }

    assert("Unknown Expr Kind");
    return nullptr;
}

/**
 * Generate a LLVM Constant Value
 * @param Type is the parsed SemaType
 * @param Val need to be correctly configured or you need to call GenDefaultValue()
 * @return
 */
llvm::Value *CodeGenExpr::GenValue(SemaType *Type, SemaValue *Val) {
    FLY_DEBUG_START("CodeGenModule", "GenValue");
    assert(Type && "Type has to be not empty");
    assert(Val && "Value has to be not empty");

    //TODO value conversion from Val->getType() to TypeBase (if are different)

	switch (Val->getType()->getTypeKind()) {

		// Boolean Value
		case SemaTypeKind::TYPE_BOOL:
			return static_cast<SemaBoolValue *>(Val) ?
					llvm::ConstantInt::getTrue(CGM->LLVMCtx) : llvm::ConstantInt::getFalse(CGM->LLVMCtx);

		// Integer Value
		case SemaTypeKind::TYPE_INTEGER: {
			SemaIntValue * IntValue = static_cast<SemaIntValue *>(Val);
			switch (Type->getTypeKind()) {

				case SemaTypeKind::TYPE_INTEGER:
					switch (static_cast<SemaIntType *>(Type)->getIntKind()) {
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
					break;
				case SemaTypeKind::TYPE_FLOATING_POINT:

					break;
				case SemaTypeKind::TYPE_BOOL:
					break;
				case SemaTypeKind::TYPE_STRING:
					break;
				case SemaTypeKind::TYPE_ERROR:
					break;
				case SemaTypeKind::TYPE_ARRAY:
					break;
				case SemaTypeKind::TYPE_CLASS:
					break;
				case SemaTypeKind::TYPE_ENUM:
					break;
				case SemaTypeKind::TYPE_VOID:
					break;
			}
		} break;

		// Floating Point Value
		case SemaTypeKind::TYPE_FLOATING_POINT: {
			SemaFloatValue *FloatValue = static_cast<SemaFloatValue *>(Val);

			switch (Type->getTypeKind()) {

				case SemaTypeKind::TYPE_VOID:
					break;
				case SemaTypeKind::TYPE_BOOL:
					break;
				case SemaTypeKind::TYPE_INTEGER:
					break;
				case SemaTypeKind::TYPE_FLOATING_POINT: {
					SemaFloatType *FPType = static_cast<SemaFloatType *>(Type);
					switch (FPType->getFPKind()) {
						case SemaFloatTypeKind::TYPE_FLOAT:
							return llvm::ConstantFP::get(CGM->FloatTy, FloatValue->getValue());
						case SemaFloatTypeKind::TYPE_DOUBLE:
							return llvm::ConstantFP::get(CGM->DoubleTy, FloatValue->getValue());
					}
				}	break;
				case SemaTypeKind::TYPE_STRING:
					break;
				case SemaTypeKind::TYPE_ERROR:
					break;
				case SemaTypeKind::TYPE_ARRAY:
					break;
				case SemaTypeKind::TYPE_CLASS:
					break;
				case SemaTypeKind::TYPE_ENUM:
					break;
			}
		} break;

		// Strig Value
		case SemaTypeKind::TYPE_STRING: {
			return CGM->Builder->CreateGlobalStringPtr(static_cast<SemaStringValue *>(Val)->getValue());
		}

		// Array Value
		case SemaTypeKind::TYPE_ARRAY: {
			SemaArrayValue *ArrayValue = static_cast<SemaArrayValue *>(Val);

			llvm::PointerType *AllocType = CGM->GenArrayType((SemaArrayType *) Type);
			std::vector<llvm::Value *> Values;
			for (SemaValue *Value : ArrayValue->getValues()) {
				llvm::Value * V = GenValue(((SemaArrayType *) Type)->getType(), Value);
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
			SemaStructValue *StructValue = static_cast<SemaStructValue *>(Val);
            break;
	}

    assert(0 && "Unknown Type");
	return nullptr;
}

llvm::Value *CodeGenExpr::GenUnary(ASTUnaryOp *Unary) {
    FLY_DEBUG_START("CodeGenExpr", "GenUnary");
    assert(Unary->getExprKind() == ASTExprKind::EXPR_UNARY && "Expected Unary Group Expr");
    assert(Unary->getExpr() && "Unary Expr empty");

    llvm::Value *OldVal = GenExpr(Unary->getExpr());
	llvm::Value *NewVal = nullptr;
	llvm::Value *Result = nullptr;

    switch (Unary->getOpKind()) {

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
	if (Unary->getExpr()->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
		ASTIdentifier *Identifier = static_cast<ASTIdentifier *>(Unary->getExpr());
		Identifier->getSema()->getCodeGen()->Store(NewVal);
	}

    return Result;
}

llvm::Value *CodeGenExpr::GenBinary(ASTBinaryOp *Binary) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinary");
    assert(Binary->getExprKind() == ASTExprKind::EXPR_BINARY && "Expected Binary Group Expr");
    assert(Binary->getLeftExpr() && "First Expr is empty");
    assert(Binary->getRightExpr() && "Second Expr is empty");

    switch (Binary->getTypeKind()) {

        case ASTBinaryOpTypeExprKind::OP_BINARY_ARITH:
            return GenBinaryArith(Binary->getLeftExpr(), Binary->getOpKind(), Binary->getRightExpr());
        case ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON:
            return GenBinaryComparison(Binary->getLeftExpr(), Binary->getOpKind(), Binary->getRightExpr());
        case ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC:
            return GenBinaryLogic(Binary->getLeftExpr(), Binary->getOpKind(), Binary->getRightExpr());
    }

    assert(0 && "Unknown Operation");
}

llvm::Value *CodeGenExpr::GenBinaryArith(ASTExpr *E1, ASTBinaryOpKind OperatorKind, ASTExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryArith");
    llvm::Value *V1 = GenExpr(E1);
    llvm::Value *V2 = GenExpr(E2);

    // Convert E2 to E1 Type
    V2 = CGM->Convert(V2, E2->getType(), E1->getType()); // Implicit conversion

    switch (OperatorKind) {

        case ASTBinaryOpKind::OP_BINARY_ADD:
            return CGM->Builder->CreateAdd(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_SUB:
            return CGM->Builder->CreateSub(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_MUL:
            return CGM->Builder->CreateMul(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_DIV:
            return CGM->Builder->CreateSDiv(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_MOD:
            return CGM->Builder->CreateSRem(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_AND:
            return CGM->Builder->CreateAnd(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_OR:
            return CGM->Builder->CreateOr(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_XOR:
            return CGM->Builder->CreateXor(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_SHIFT_L:
            return CGM->Builder->CreateShl(V1, V2);
        case ASTBinaryOpKind::OP_BINARY_SHIFT_R:
            return CGM->Builder->CreateAShr(V1, V2);
    }
    assert(0 && "Unknown Arith Operation");
}

llvm::Value *CodeGenExpr::GenBinaryComparison(ASTExpr *E1, ASTBinaryOpKind OperatorKind, ASTExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryComparison");
    llvm::Value *V1 = GenExpr(E1);
    llvm::Value *V2 = GenExpr(E2);
	SemaType *V1Type = E1->getType();
    SemaType *V2Type = E2->getType();

    if (E1->getType()->isBool() && E2->getType()->isBool()) {
        switch (OperatorKind) {
            case ASTBinaryOpKind::OP_BINARY_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
        }
    } else if (E1->getType()->isInteger() && E2->getType()->isInteger()) {
        bool Signed = static_cast<SemaIntType *>(E1->getType())->isSigned() ||
        	static_cast<SemaIntType *>(E2->getType())->isSigned();
        switch (OperatorKind) {

            case ASTBinaryOpKind::OP_BINARY_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_GT:
                return Signed ? CGM->Builder->CreateICmpSGT(V1, V2) : CGM->Builder->CreateICmpUGT(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_GTE:
                return Signed ? CGM->Builder->CreateICmpSGE(V1, V2) : CGM->Builder->CreateICmpUGE(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_LT:
                return Signed ? CGM->Builder->CreateICmpSLT(V1, V2) : CGM->Builder->CreateICmpULT(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_LTE:
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

            case ASTBinaryOpKind::OP_BINARY_EQ:
                return CGM->Builder->CreateFCmpOEQ(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_NE:
                return CGM->Builder->CreateFCmpONE(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_GT:
                return CGM->Builder->CreateFCmpOGT(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_GTE:
                return CGM->Builder->CreateFCmpOGE(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_LT:
                return CGM->Builder->CreateFCmpOLT(V1, V2);
            case ASTBinaryOpKind::OP_BINARY_LTE:
                return CGM->Builder->CreateFCmpOLE(V1, V2);
        }
    }

    assert(0 && "Invalid Comparator Operator");
}

llvm::Value *CodeGenExpr::GenBinaryLogic(ASTExpr *E1, ASTBinaryOpKind OperatorKind, ASTExpr *E2) {
    FLY_DEBUG_START("CodeGenExpr", "GenBinaryLogic");
    llvm::Value *V1 = GenExpr(E1);
    V1 = CGM->ConvertToBool(V1);
//    V1 = Convert(V1, E1->getType(), BoolType); //FIXME
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

llvm::Value *CodeGenExpr::GenTernary(ASTTernaryOp *Ternary) {
    assert(Ternary->getConditionExpr() && "First Expr is empty");
    assert(Ternary->getTrueExpr() && "Second Expr is empty");
    assert(Ternary->getFalseExpr() && "Third Expr is empty");

    llvm::BasicBlock *FromBB = CGM->Builder->GetInsertBlock();
    llvm::Value *Cond = GenExpr(Ternary->getConditionExpr());

    // Create Blocks
    llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "terntrue", FromBB->getParent());
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternfalse", FromBB->getParent());
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternend", FromBB->getParent());

    // Create Condition
    CGM->Builder->CreateCondBr(Cond, TrueBB, FalseBB);

    // True Label
    CGM->Builder->SetInsertPoint(TrueBB);
    llvm::Value *True = GenExpr(Ternary->getTrueExpr());
    llvm::Value *BoolTrue = CGM->ConvertToBool(True);
    CGM->Builder->CreateBr(EndBB);

    // False Label
    CGM->Builder->SetInsertPoint(FalseBB);
    llvm::Value *False = GenExpr(Ternary->getFalseExpr());
    llvm::Value *BoolFalse = CGM->ConvertToBool(False);
    CGM->Builder->CreateBr(EndBB);

    // End Label
    CGM->Builder->SetInsertPoint(EndBB);
    llvm::PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
    Phi->addIncoming(BoolTrue, TrueBB);
    Phi->addIncoming(BoolFalse, FalseBB);
    return Phi;
}