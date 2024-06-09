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
#include "AST/ASTGroupExpr.h"
#include "AST/ASTOperatorExpr.h"
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
        case ASTExprKind::EXPR_GROUP:
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_GROUP");
            return GenGroup((ASTGroupExpr *) Expr);

        case ASTExprKind::EXPR_EMPTY:
            return nullptr; // FIXME
    }

    assert("Unknown Expr Kind");
    return nullptr;
}

llvm::Value *CodeGenExpr::getValue() const {
    return Val;
}

/**
 * Generate the Value by generating expression recursively
 * @param Group
 * @return
 */
llvm::Value *CodeGenExpr::GenGroup(ASTGroupExpr *Group) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "GenGroup", "GroupKind=" + std::to_string((int) Group->getGroupKind()));

    llvm::Value *V = nullptr;
    switch (Group->getGroupKind()) {
        case ASTExprGroupKind::GROUP_UNARY:
            V = GenUnary((ASTUnaryGroupExpr *) Group);
            break;
        case ASTExprGroupKind::GROUP_BINARY:
            V = GenBinary((ASTBinaryGroupExpr *) Group);
            break;
        case ASTExprGroupKind::GROUP_TERNARY:
            V = GenTernary((ASTTernaryGroupExpr *) Group);
            break;
    }

    return V;
}

llvm::Value *CodeGenExpr::GenUnary(ASTUnaryGroupExpr *Expr) {
    FLY_DEBUG("CodeGenExpr", "GenUnary");
    assert(Expr->getGroupKind() == ASTExprGroupKind::GROUP_UNARY  && "Expected Unary Group Expr");
    assert(Expr->getFirst() && "Unary Expr empty");

    CodeGenVarBase *CGVal = Expr->getFirst()->getVarRef()->getDef()->getCodeGen();
    llvm::Value *OldVal = CGVal->getValue();

    switch (Expr->getOperator()->getOperatorKind()) {

        case ASTUnaryOperatorKind::UNARY_ARITH_PRE_INCR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
            llvm::Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            CGVal->Store(NewVal);
            return NewVal;
        }
        case ASTUnaryOperatorKind::UNARY_ARITH_POST_INCR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
            Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            CGVal->Store(NewVal);
            return OldVal;
        }
        case ASTUnaryOperatorKind::UNARY_ARITH_PRE_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
            Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            CGVal->Store(NewVal);
            return NewVal;
        }
        case ASTUnaryOperatorKind::UNARY_ARITH_POST_DECR: {
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
            Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
            CGVal->Store(NewVal);
            return OldVal;
        }
        case ASTUnaryOperatorKind::UNARY_LOGIC_NOT:
            OldVal = CGM->Builder->CreateTrunc(OldVal, CGM->BoolTy);
            OldVal = CGM->Builder->CreateXor(OldVal, true);
            return CGM->Builder->CreateZExt(OldVal, CGM->Int8Ty);
    }

    assert(0 && "Invalid Unary Operation");
}

llvm::Value *CodeGenExpr::GenBinary(ASTBinaryGroupExpr *Expr) {
    FLY_DEBUG("CodeGenExpr", "GenBinary");
    assert(Expr->getGroupKind() == ASTExprGroupKind::GROUP_BINARY && "Expected Binary Group Expr");
    assert(Expr->getFirst() && "First Expr is empty");
    assert(Expr->getSecond() && "Second Expr is empty");

    switch (Expr->getOperator()->getOptionKind()) {

        case ASTBinaryOptionKind::BINARY_ARITH:
            return GenBinaryArith(Expr->getFirst(), Expr->getOperator()->getOperatorKind(), Expr->getSecond());
        case ASTBinaryOptionKind::BINARY_COMPARISON:
            return GenBinaryComparison(Expr->getFirst(), Expr->getOperator()->getOperatorKind(), Expr->getSecond());
        case ASTBinaryOptionKind::BINARY_LOGIC:
            return GenBinaryLogic(Expr->getFirst(), Expr->getOperator()->getOperatorKind(), Expr->getSecond());
    }

    assert(0 && "Unknown Operation");
}

llvm::Value *CodeGenExpr::GenBinaryArith(const ASTExpr *E1, ASTBinaryOperatorKind OperatorKind, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryArith");
    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);

    // Convert E2 to E1 Type
    V2 = CGM->Convert(V2, E2->getType(), E1->getType()); // Implicit conversion

    switch (OperatorKind) {

        case ASTBinaryOperatorKind::BINARY_ARITH_ADD:
            return CGM->Builder->CreateAdd(V1, V2);
        case ASTBinaryOperatorKind::BINARY_ARITH_SUB:
            return CGM->Builder->CreateSub(V1, V2);
        case ASTBinaryOperatorKind::BINARY_ARITH_MUL:
            return CGM->Builder->CreateMul(V1, V2);
        case ASTBinaryOperatorKind::BINARY_ARITH_DIV:
            return CGM->Builder->CreateSDiv(V1, V2);
        case ASTBinaryOperatorKind::BINARY_ARITH_MOD:
            return CGM->Builder->CreateSRem(V1, V2);
        case ASTBinaryOperatorKind::BINARY_ARITH_AND:
            return CGM->Builder->CreateAnd(V1, V2);
        case ASTBinaryOperatorKind::BINARY_ARITH_OR:
            return CGM->Builder->CreateOr(V1, V2);
        case ASTBinaryOperatorKind::BINARY_ARITH_XOR:
            return CGM->Builder->CreateXor(V1, V2);
        case ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_L:
            return CGM->Builder->CreateShl(V1, V2);
        case ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_R:
            return CGM->Builder->CreateAShr(V1, V2);
    }
    assert(0 && "Unknown Arith Operation");
}

llvm::Value *CodeGenExpr::GenBinaryComparison(const ASTExpr *E1, ASTBinaryOperatorKind OperatorKind, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryComparison");
    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);
    ASTType *V2Type = E2->getType();

    if (E1->getType()->isBool() && E2->getType()->isBool()) {
        switch (OperatorKind) {
            case ASTBinaryOperatorKind::BINARY_COMP_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
        }
    } else if (E1->getType()->isInteger() && E2->getType()->isInteger()) {
        bool Signed = ((ASTIntegerType *) E1->getType())->isSigned() || ((ASTIntegerType *) E2->getType())->isSigned();
        switch (OperatorKind) {

            case ASTBinaryOperatorKind::BINARY_COMP_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_GT:
                return Signed ? CGM->Builder->CreateICmpSGT(V1, V2) : CGM->Builder->CreateICmpUGT(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_GTE:
                return Signed ? CGM->Builder->CreateICmpSGE(V1, V2) : CGM->Builder->CreateICmpUGE(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_LT:
                return Signed ? CGM->Builder->CreateICmpSLT(V1, V2) : CGM->Builder->CreateICmpULT(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_LTE:
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

            case ASTBinaryOperatorKind::BINARY_COMP_EQ:
                return CGM->Builder->CreateFCmpOEQ(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_NE:
                return CGM->Builder->CreateFCmpONE(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_GT:
                return CGM->Builder->CreateFCmpOGT(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_GTE:
                return CGM->Builder->CreateFCmpOGE(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_LT:
                return CGM->Builder->CreateFCmpOLT(V1, V2);
            case ASTBinaryOperatorKind::BINARY_COMP_LTE:
                return CGM->Builder->CreateFCmpOLE(V1, V2);
        }
    }

    assert(0 && "Invalid Comparator Operator");
}

llvm::Value *CodeGenExpr::GenBinaryLogic(const ASTExpr *E1, ASTBinaryOperatorKind OperatorKind, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryLogic");
    llvm::Value *V1 = GenValue(E1);
    V1 = CGM->ConvertToBool(V1);
//    V1 = Convert(V1, E1->getType(), BoolType); //FIXME
    llvm::BasicBlock *FromBB = CGM->Builder->GetInsertBlock();

    switch (OperatorKind) {

        case ASTBinaryOperatorKind::BINARY_LOGIC_AND: {
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
        case ASTBinaryOperatorKind::BINARY_LOGIC_OR: {
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

llvm::Value *CodeGenExpr::GenTernary(ASTTernaryGroupExpr *Expr) {
    assert(Expr->getGroupKind() == ASTExprGroupKind::GROUP_TERNARY && "Expected Ternary Group Expr");
    assert(Expr->getFirst() && "First Expr is empty");
    assert(Expr->getSecond() && "Second Expr is empty");
    assert(Expr->getThird() && "Third Expr is empty");

    llvm::BasicBlock *FromBB = CGM->Builder->GetInsertBlock();
    llvm::Value *Cond = GenValue(Expr->getFirst());

    // Create Blocks
    llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "terntrue", FromBB->getParent());
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternfalse", FromBB->getParent());
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternend", FromBB->getParent());

    // Create Condition
    CGM->Builder->CreateCondBr(Cond, TrueBB, FalseBB);

    // True Label
    CGM->Builder->SetInsertPoint(TrueBB);
    llvm::Value *True = GenValue(Expr->getSecond());
    llvm::Value *BoolTrue = CGM->ConvertToBool(True);
    CGM->Builder->CreateBr(EndBB);

    // False Label
    CGM->Builder->SetInsertPoint(FalseBB);
    llvm::Value *False = GenValue(Expr->getThird());
    llvm::Value *BoolFalse = CGM->ConvertToBool(False);
    CGM->Builder->CreateBr(EndBB);

    // End Label
    CGM->Builder->SetInsertPoint(EndBB);
    PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
    Phi->addIncoming(BoolTrue, TrueBB);
    Phi->addIncoming(BoolFalse, FalseBB);
    return Phi;
}