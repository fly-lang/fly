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
#include "CodeGen/CodeGenFunctionBase.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTExpr.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTOpExpr.h"
#include "Basic/Debug.h"
#include "llvm/IR/Value.h"

using namespace fly;

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM, ASTExpr *Expr) :
        CGM(CGM) {
    FLY_DEBUG("CodeGenExpr", "CodeGenExpr");
    Val = GenValue(Expr);
}

llvm::Value *CodeGenExpr::GenValue(const ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "Expr=" << Expr->str());
    switch (Expr->getExprKind()) {

        case ASTExprKind::EXPR_VALUE: {
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_VALUE");
            return CGM->GenValue(Expr->getType(), ((ASTValueExpr *)Expr)->getValue());
        }
        case ASTExprKind::EXPR_VAR_REF: {
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_VAR_REF");
            ASTVarRef *VarRef = ((ASTVarRefExpr *) Expr)->getVarRef();
            assert(VarRef && "Missing Ref");
            return CGM->GenVarRef(VarRef);
        }
        case ASTExprKind::EXPR_CALL: {
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_REF_FUNC");
            ASTCallExpr *CallExpr = (ASTCallExpr *)Expr;
            return CGM->GenCall(CallExpr->getCall());
        }
        case ASTExprKind::EXPR_OP:
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_GROUP");
            return GenOp((ASTOpExpr *) Expr);
    }

    assert("Unknown Expr Kind");
    return nullptr;
}

llvm::Value *CodeGenExpr::getValue() const {
    return Val;
}

/**
 * Generate the Value by generating expression recursively
 * @param Expr
 * @return
 */
llvm::Value *CodeGenExpr::GenOp(ASTOpExpr *Expr) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "GenOp", "GroupKind=" + std::to_string((int) Expr->getOpExprKind()));

    llvm::Value *V = nullptr;
    switch (Expr->getOpExprKind()) {
        case ASTOpExprKind::OP_UNARY:
            V = GenUnary((ASTUnaryOpExpr *) Expr);
            break;
        case ASTOpExprKind::OP_BINARY:
            V = GenBinary((ASTBinaryOpExpr *) Expr);
            break;
        case ASTOpExprKind::OP_TERNARY:
            V = GenTernary((ASTTernaryOpExpr *) Expr);
            break;
    }

    return V;
}

llvm::Value *CodeGenExpr::GenUnary(ASTUnaryOpExpr *Expr) {
    FLY_DEBUG("CodeGenExpr", "GenUnary");
    assert(Expr->getOpExprKind() == ASTOpExprKind::OP_UNARY && "Expected Unary Group Expr");
    assert(Expr->getExpr() && "Unary Expr empty");

    // FIXME check ASTVarRefExpr
    CodeGenVarBase *CGVal = ((ASTVarRefExpr *) Expr->getExpr())->getVarRef()->getDef()->getCodeGen();
    llvm::Value *OldVal = CGVal->getValue();

    switch (Expr->getOpKind()) {

        case ASTUnaryOpExprKind::OP_UNARY_PRE_INCR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
            llvm::Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            CGVal->Store(NewVal);
            return NewVal;
        }
        case ASTUnaryOpExprKind::OP_UNARY_POST_INCR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
            Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            CGVal->Store(NewVal);
            return OldVal;
        }
        case ASTUnaryOpExprKind::OP_UNARY_PRE_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
            Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            CGVal->Store(NewVal);
            return NewVal;
        }
        case ASTUnaryOpExprKind::OP_UNARY_POST_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
            Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            CGVal->Store(NewVal);
            return OldVal;
        }
        case ASTUnaryOpExprKind::OP_UNARY_NOT_LOG:
            OldVal = CGM->Builder->CreateTrunc(OldVal, CGM->BoolTy);
            OldVal = CGM->Builder->CreateXor(OldVal, true);
            return CGM->Builder->CreateZExt(OldVal, CGM->Int8Ty);
    }

    assert(0 && "Invalid Unary Operation");
}

llvm::Value *CodeGenExpr::GenBinary(ASTBinaryOpExpr *Expr) {
    FLY_DEBUG("CodeGenExpr", "GenBinary");
    assert(Expr->getOpExprKind() == ASTOpExprKind::OP_BINARY && "Expected Binary Group Expr");
    assert(Expr->getLeftExpr() && "First Expr is empty");
    assert(Expr->getRightExpr() && "Second Expr is empty");

    switch (Expr->getTypeKind()) {

        case ASTBinaryOpTypeExprKind::OP_BINARY_ARITH:
            return GenBinaryArith(Expr->getLeftExpr(), Expr->getOpKind(), Expr->getRightExpr());
        case ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON:
            return GenBinaryComparison(Expr->getLeftExpr(), Expr->getOpKind(), Expr->getRightExpr());
        case ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC:
            return GenBinaryLogic(Expr->getLeftExpr(), Expr->getOpKind(), Expr->getRightExpr());
    }

    assert(0 && "Unknown Operation");
}

llvm::Value *CodeGenExpr::GenBinaryArith(const ASTExpr *E1, ASTBinaryOpExprKind OperatorKind, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryArith");
    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);

    // Convert E2 to E1 Type
    V2 = CGM->Convert(V2, E2->getType(), E1->getType()); // Implicit conversion

    switch (OperatorKind) {

        case ASTBinaryOpExprKind::OP_BINARY_ADD:
            return CGM->Builder->CreateAdd(V1, V2);
        case ASTBinaryOpExprKind::OP_BINARY_SUB:
            return CGM->Builder->CreateSub(V1, V2);
        case ASTBinaryOpExprKind::OP_BINARY_MUL:
            return CGM->Builder->CreateMul(V1, V2);
        case ASTBinaryOpExprKind::OP_BINARY_DIV:
            return CGM->Builder->CreateSDiv(V1, V2);
        case ASTBinaryOpExprKind::OP_BINARY_MOD:
            return CGM->Builder->CreateSRem(V1, V2);
        case ASTBinaryOpExprKind::OP_BINARY_AND:
            return CGM->Builder->CreateAnd(V1, V2);
        case ASTBinaryOpExprKind::OP_BINARY_OR:
            return CGM->Builder->CreateOr(V1, V2);
        case ASTBinaryOpExprKind::OP_BINARY_XOR:
            return CGM->Builder->CreateXor(V1, V2);
        case ASTBinaryOpExprKind::OP_BINARY_SHIFT_L:
            return CGM->Builder->CreateShl(V1, V2);
        case ASTBinaryOpExprKind::OP_BINARY_SHIFT_R:
            return CGM->Builder->CreateAShr(V1, V2);
    }
    assert(0 && "Unknown Arith Operation");
}

llvm::Value *CodeGenExpr::GenBinaryComparison(const ASTExpr *E1, ASTBinaryOpExprKind OperatorKind, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryComparison");
    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);
    ASTType *V2Type = E2->getType();

    if (E1->getType()->isBool() && E2->getType()->isBool()) {
        switch (OperatorKind) {
            case ASTBinaryOpExprKind::OP_BINARY_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
        }
    } else if (E1->getType()->isInteger() && E2->getType()->isInteger()) {
        bool Signed = ((ASTIntegerType *) E1->getType())->isSigned() || ((ASTIntegerType *) E2->getType())->isSigned();
        switch (OperatorKind) {

            case ASTBinaryOpExprKind::OP_BINARY_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_GT:
                return Signed ? CGM->Builder->CreateICmpSGT(V1, V2) : CGM->Builder->CreateICmpUGT(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_GTE:
                return Signed ? CGM->Builder->CreateICmpSGE(V1, V2) : CGM->Builder->CreateICmpUGE(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_LT:
                return Signed ? CGM->Builder->CreateICmpSLT(V1, V2) : CGM->Builder->CreateICmpULT(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_LTE:
                return Signed ? CGM->Builder->CreateICmpSLE(V1, V2) : CGM->Builder->CreateICmpULE(V1, V2);
        }
    } else {
        // Convert values to Float if one of them is Float
        if ( (V1->getType()->isFloatTy() || V1->getType()->isDoubleTy()) &&
             (V2->getType()->isIntegerTy() || V2->getType()->isIntegerTy()) ) {
            V2 = CGM->Convert(V2, V2Type, E1->getType()); // Explicit conversion
        } else if ( (V1->getType()->isIntegerTy() || V1->getType()->isIntegerTy()) &&
                    (V2->getType()->isFloatTy() || V2->getType()->isDoubleTy()) ) {
            V1 = CGM->Convert(V1, V2Type, E1->getType()); // Explicit conversion
        }
        switch (OperatorKind) {

            case ASTBinaryOpExprKind::OP_BINARY_EQ:
                return CGM->Builder->CreateFCmpOEQ(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_NE:
                return CGM->Builder->CreateFCmpONE(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_GT:
                return CGM->Builder->CreateFCmpOGT(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_GTE:
                return CGM->Builder->CreateFCmpOGE(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_LT:
                return CGM->Builder->CreateFCmpOLT(V1, V2);
            case ASTBinaryOpExprKind::OP_BINARY_LTE:
                return CGM->Builder->CreateFCmpOLE(V1, V2);
        }
    }

    assert(0 && "Invalid Comparator Operator");
}

llvm::Value *CodeGenExpr::GenBinaryLogic(const ASTExpr *E1, ASTBinaryOpExprKind OperatorKind, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryLogic");
    llvm::Value *V1 = GenValue(E1);
    V1 = CGM->ConvertToBool(V1);
//    V1 = Convert(V1, E1->getType(), BoolType); //FIXME
    llvm::BasicBlock *FromBB = CGM->Builder->GetInsertBlock();

    switch (OperatorKind) {

        case ASTBinaryOpExprKind::OP_BINARY_LOGIC_AND: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", FromBB->getParent());
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", FromBB->getParent());

            // From Branch
            CGM->Builder->CreateCondBr(V1, LeftBB, RightBB);

            // Left Branch
            CGM->Builder->SetInsertPoint(LeftBB);
            llvm::Value *V2 = GenValue(E2);
            V2 = CGM->ConvertToBool(V2);
            llvm::Value *V2Trunc = CGM->Builder->CreateTrunc(V2, CGM->BoolTy);
            CGM->Builder->CreateBr(RightBB);

            // Right Branch
            CGM->Builder->SetInsertPoint(RightBB);
            PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->BoolTy, false, false), FromBB);
            Phi->addIncoming(V2Trunc, LeftBB);
            return Phi;
        }
        case ASTBinaryOpExprKind::OP_BINARY_LOGIC_OR: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", FromBB->getParent());
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", FromBB->getParent());

            // From Branch
            CGM->Builder->CreateCondBr(V1, RightBB, LeftBB);

            // Left Branch
            CGM->Builder->SetInsertPoint(LeftBB);
            llvm::Value *V2 = GenValue(E2);
            llvm::Value *V2Trunc = CGM->Builder->CreateTrunc(V2, CGM->BoolTy);
            CGM->Builder->CreateBr(RightBB);

            // Right Branch
            CGM->Builder->SetInsertPoint(RightBB);
            PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->BoolTy, true, false), FromBB);
            Phi->addIncoming(V2Trunc, LeftBB);
            return Phi;
        }
    }
    assert(0 && "Invalid Logic Operator");
}

llvm::Value *CodeGenExpr::GenTernary(ASTTernaryOpExpr *Expr) {
    assert(Expr->getConditionExpr() && "First Expr is empty");
    assert(Expr->getTrueExpr() && "Second Expr is empty");
    assert(Expr->getFalseExpr() && "Third Expr is empty");

    llvm::BasicBlock *FromBB = CGM->Builder->GetInsertBlock();
    llvm::Value *Cond = GenValue(Expr->getConditionExpr());

    // Create Blocks
    llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "terntrue", FromBB->getParent());
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternfalse", FromBB->getParent());
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternend", FromBB->getParent());

    // Create Condition
    CGM->Builder->CreateCondBr(Cond, TrueBB, FalseBB);

    // True Label
    CGM->Builder->SetInsertPoint(TrueBB);
    llvm::Value *True = GenValue(Expr->getTrueExpr());
    llvm::Value *BoolTrue = CGM->ConvertToBool(True);
    CGM->Builder->CreateBr(EndBB);

    // False Label
    CGM->Builder->SetInsertPoint(FalseBB);
    llvm::Value *False = GenValue(Expr->getFalseExpr());
    llvm::Value *BoolFalse = CGM->ConvertToBool(False);
    CGM->Builder->CreateBr(EndBB);

    // End Label
    CGM->Builder->SetInsertPoint(EndBB);
    PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
    Phi->addIncoming(BoolTrue, TrueBB);
    Phi->addIncoming(BoolFalse, FalseBB);
    return Phi;
}