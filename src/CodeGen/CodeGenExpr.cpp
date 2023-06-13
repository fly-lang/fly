//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGExpr.cpp - Code Generator Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenExpr.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTExpr.h"
#include "AST/ASTVarRef.h"
#include "Basic/Debug.h"
#include "llvm/IR/Value.h"

using namespace fly;

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM, llvm::Function *Fn, ASTExpr *Expr, const ASTType *ToType) :
        CGM(CGM), Fn(Fn) {
    FLY_DEBUG("CodeGenExpr", "CodeGenExpr");
    ASTType *FromType = Expr->getType();
    llvm::Value *TheVal = GenValue(Expr);
    Val = Convert(TheVal, FromType, ToType);
}

bool CodeGenExpr::isNoStore() const {
    return NoStore;
}

llvm::Value *CodeGenExpr::getValue() const {
    return Val;
}

llvm::Value *CodeGenExpr::Convert(llvm::Value *FromVal, const ASTType *FromType, const ASTType *ToType) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "Convert",
                      "Value=" << FromVal << " to ASTType=" << ToType->str());
    assert(ToType && "Invalid conversion type");

    llvm::Type *FromLLVMType = FromVal->getType();
    switch (ToType->getKind()) {

            // to INT 1
        case ASTTypeKind::TYPE_BOOL:
            switch (FromType->getKind()) {
                case ASTTypeKind::TYPE_BOOL: {
                    return CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                }
                case ASTTypeKind::TYPE_BYTE:
                case ASTTypeKind::TYPE_USHORT:
                case ASTTypeKind::TYPE_SHORT:
                case ASTTypeKind::TYPE_UINT:
                case ASTTypeKind::TYPE_INT:
                case ASTTypeKind::TYPE_ULONG:
                case ASTTypeKind::TYPE_LONG: {
                    llvm::Value *ZERO = llvm::ConstantInt::get(FromLLVMType, 0, false);
                    return CGM->Builder->CreateICmpNE(FromVal, ZERO);
                }
                case ASTTypeKind::TYPE_FLOAT:
                case ASTTypeKind::TYPE_DOUBLE:
                    llvm::Value *ZERO = llvm::ConstantInt::get(FromLLVMType, 0, false);
                    return CGM->Builder->CreateFCmpUNE(FromVal, ZERO);
            }

            // to INT 8
        case ASTTypeKind::TYPE_BYTE:
            switch (FromType->getKind()) {
                case ASTTypeKind::TYPE_BOOL: {
                    llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                    return CGM->Builder->CreateZExt(ToVal, CGM->Int8Ty);
                }
                case ASTTypeKind::TYPE_BYTE:
                    return FromVal;
                case ASTTypeKind::TYPE_USHORT:
                case ASTTypeKind::TYPE_SHORT:
                case ASTTypeKind::TYPE_UINT:
                case ASTTypeKind::TYPE_INT:
                case ASTTypeKind::TYPE_ULONG:
                case ASTTypeKind::TYPE_LONG:
                    return CGM->Builder->CreateTrunc(FromVal, CGM->Int8Ty);
                case ASTTypeKind::TYPE_FLOAT:
                case ASTTypeKind::TYPE_DOUBLE:
                    return CGM->Builder->CreateFPToUI(FromVal, CGM->Int8Ty);
            }

            // to Unsigned INT 16
        case ASTTypeKind::TYPE_USHORT:
            switch (FromType->getKind()) {
                case ASTTypeKind::TYPE_BOOL: {
                    llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                    return CGM->Builder->CreateZExt(ToVal, CGM->Int16Ty);
                }
                case ASTTypeKind::TYPE_BYTE:
                    return CGM->Builder->CreateZExt(FromVal, CGM->Int16Ty);
                case ASTTypeKind::TYPE_USHORT:
                case ASTTypeKind::TYPE_SHORT:
                    return FromVal;
                case ASTTypeKind::TYPE_UINT:
                case ASTTypeKind::TYPE_INT:
                case ASTTypeKind::TYPE_ULONG:
                case ASTTypeKind::TYPE_LONG:
                    return CGM->Builder->CreateTrunc(FromVal, CGM->Int16Ty);
                case ASTTypeKind::TYPE_FLOAT:
                case ASTTypeKind::TYPE_DOUBLE:
                    return CGM->Builder->CreateFPToUI(FromVal, CGM->Int16Ty);
            }

            // to Signed INT 16
        case ASTTypeKind::TYPE_SHORT:
            switch (FromType->getKind()) {
                case ASTTypeKind::TYPE_BOOL: {
                    llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                    return CGM->Builder->CreateZExt(ToVal, CGM->Int16Ty);
                }
                case ASTTypeKind::TYPE_BYTE:
                    return CGM->Builder->CreateZExt(FromVal, CGM->Int16Ty);
                case ASTTypeKind::TYPE_USHORT:
                case ASTTypeKind::TYPE_SHORT:
                    return FromVal;
                case ASTTypeKind::TYPE_UINT:
                case ASTTypeKind::TYPE_INT:
                case ASTTypeKind::TYPE_ULONG:
                case ASTTypeKind::TYPE_LONG:
                    return CGM->Builder->CreateTrunc(FromVal, CGM->Int16Ty);
                case ASTTypeKind::TYPE_FLOAT:
                case ASTTypeKind::TYPE_DOUBLE:
                    return CGM->Builder->CreateFPToSI(FromVal, CGM->Int16Ty);
            }

            // to Unsigned INT 32
        case ASTTypeKind::TYPE_UINT:
            switch (FromType->getKind()) {
                case ASTTypeKind::TYPE_BOOL: {
                    llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                    return CGM->Builder->CreateZExt(ToVal, CGM->Int32Ty);
                }
                case ASTTypeKind::TYPE_BYTE:
                case ASTTypeKind::TYPE_USHORT:
                    return CGM->Builder->CreateZExt(FromVal, CGM->Int32Ty);
                case ASTTypeKind::TYPE_SHORT:
                    return CGM->Builder->CreateSExt(FromVal, CGM->Int32Ty);
                case ASTTypeKind::TYPE_UINT:
                case ASTTypeKind::TYPE_INT:
                    return FromVal;
                case ASTTypeKind::TYPE_ULONG:
                case ASTTypeKind::TYPE_LONG:
                    return CGM->Builder->CreateTrunc(FromVal, CGM->Int32Ty);
                case ASTTypeKind::TYPE_FLOAT:
                case ASTTypeKind::TYPE_DOUBLE:
                    return CGM->Builder->CreateFPToUI(FromVal, CGM->Int32Ty);;
            }

            // to Signed INT 32
        case ASTTypeKind::TYPE_INT:
            switch (FromType->getKind()) {
                case ASTTypeKind::TYPE_BOOL: {
                    llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                    return CGM->Builder->CreateZExt(ToVal, CGM->Int32Ty);
                }
                case ASTTypeKind::TYPE_BYTE:
                case ASTTypeKind::TYPE_USHORT:
                    return CGM->Builder->CreateZExt(FromVal, CGM->Int32Ty);
                case ASTTypeKind::TYPE_SHORT:
                    return CGM->Builder->CreateSExt(FromVal, CGM->Int32Ty);
                case ASTTypeKind::TYPE_UINT:
                case ASTTypeKind::TYPE_INT:
                    return FromVal;
                case ASTTypeKind::TYPE_ULONG:
                case ASTTypeKind::TYPE_LONG:
                    return CGM->Builder->CreateTrunc(FromVal, CGM->Int32Ty);
                case ASTTypeKind::TYPE_FLOAT:
                case ASTTypeKind::TYPE_DOUBLE:
                    return CGM->Builder->CreateFPToSI(FromVal, CGM->Int32Ty);
            }

            // to Unsigned INT 64
        case ASTTypeKind::TYPE_ULONG:
            switch (FromType->getKind()) {
                case ASTTypeKind::TYPE_BOOL: {
                    llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                    return CGM->Builder->CreateZExt(ToVal, CGM->Int64Ty);
                }
                case ASTTypeKind::TYPE_BYTE:
                case ASTTypeKind::TYPE_USHORT:
                case ASTTypeKind::TYPE_UINT:
                    return CGM->Builder->CreateZExt(FromVal, CGM->Int64Ty);
                case ASTTypeKind::TYPE_SHORT:
                case ASTTypeKind::TYPE_INT:
                    return CGM->Builder->CreateSExt(FromVal, CGM->Int64Ty);
                case ASTTypeKind::TYPE_ULONG:
                case ASTTypeKind::TYPE_LONG:
                    return FromVal;
                case ASTTypeKind::TYPE_FLOAT:
                case ASTTypeKind::TYPE_DOUBLE:
                    return CGM->Builder->CreateFPToUI(FromVal, CGM->Int64Ty);
            }

            // to Signed INT 64
        case ASTTypeKind::TYPE_LONG:
            switch (FromType->getKind()) {
                case ASTTypeKind::TYPE_BOOL: {
                    llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                    return CGM->Builder->CreateZExt(ToVal, CGM->Int64Ty);
                }
                case ASTTypeKind::TYPE_BYTE:
                case ASTTypeKind::TYPE_USHORT:
                case ASTTypeKind::TYPE_UINT:
                    return CGM->Builder->CreateZExt(FromVal, CGM->Int64Ty);
                case ASTTypeKind::TYPE_SHORT:
                case ASTTypeKind::TYPE_INT:
                    return CGM->Builder->CreateSExt(FromVal, CGM->Int64Ty);
                case ASTTypeKind::TYPE_ULONG:
                case ASTTypeKind::TYPE_LONG:
                    return FromVal;
                case ASTTypeKind::TYPE_FLOAT:
                case ASTTypeKind::TYPE_DOUBLE:
                    return CGM->Builder->CreateFPToSI(FromVal, CGM->Int64Ty);
            }

            // to FLOAT 32
        case ASTTypeKind::TYPE_FLOAT:
            if (FromLLVMType->isIntegerTy()) { // INT to FLOAT
                if (FromType->getKind() == ASTTypeKind::TYPE_BOOL) {
                    FromVal = CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                }
                return isSigned(FromType) ?
                       CGM->Builder->CreateSIToFP(FromVal, CGM->FloatTy) :
                       CGM->Builder->CreateUIToFP(FromVal, CGM->FloatTy);
            } else if (FromType->getKind() == ASTTypeKind::TYPE_FLOAT) { // FLOAT to FLOAT
                return FromVal;
            } else if (FromType->getKind() == ASTTypeKind::TYPE_DOUBLE) { // DOUBLE to FLOAT
                return CGM->Builder->CreateFPTrunc(FromVal, CGM->FloatTy);
            }

            // to DOUBLE 64
        case ASTTypeKind::TYPE_DOUBLE: {
            if (FromLLVMType->isIntegerTy()) { // INT to DOUBLE
                if (FromType->getKind() == ASTTypeKind::TYPE_BOOL) {
                    FromVal = CGM->Builder->CreateTrunc(FromVal, CGM->BoolTy);
                }
                return isSigned(FromType) ?
                       CGM->Builder->CreateSIToFP(FromVal, CGM->DoubleTy) :
                       CGM->Builder->CreateUIToFP(FromVal, CGM->DoubleTy);
            } else if (FromType->getKind() == ASTTypeKind::TYPE_FLOAT) { // FLOAT to DOUBLE
                return CGM->Builder->CreateFPExt(FromVal, CGM->DoubleTy);
            } else if (FromType->getKind() == ASTTypeKind::TYPE_DOUBLE) { // DOUBLE to DOUBLE
                return FromVal;
            }
        }

            // to Class
        case ASTTypeKind::TYPE_CLASS:
            return FromVal;
    }
    assert(0 && "Conversion failed");
}

llvm::Value *CodeGenExpr::GenValue(const ASTExpr *Expr, llvm::Value *Pointer) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "Expr=" << Expr->str());
    switch (Expr->getExprKind()) {

        case ASTExprKind::EXPR_VALUE: {
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_VALUE");
            return CGM->GenValue(Expr->getType(), &((ASTValueExpr *)Expr)->getValue());
        }
        case ASTExprKind::EXPR_VAR_REF: {
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_REF_VAR");
            ASTVarRefExpr *VarRefExpr = (ASTVarRefExpr *) Expr;
            assert(VarRefExpr->getVarRef() && "Missing Ref");
            CGM->GenVarRef(VarRefExpr->getVarRef());
            return VarRefExpr->getVarRef()->getDef()->getCodeGen()->getValue();
        }
        case ASTExprKind::EXPR_CALL: {
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_REF_FUNC");
            ASTCallExpr *CallExpr = (ASTCallExpr *)Expr;
            return CGM->GenCall(Fn, CallExpr->getCall(), NoStore);
        }
        case ASTExprKind::EXPR_GROUP:
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_GROUP");
            return GenGroup((ASTGroupExpr *) Expr);
    }

    assert("Unknown Expr Kind");
    return nullptr;
}

/**
 * Generate the Value by generating expression recursively
 * @param Origin
 * @param New
 * @param Idx
 * @param E1
 * @param OP1
 * @return
 */
llvm::Value *CodeGenExpr::GenGroup(ASTGroupExpr *Group) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "GenGroup", "GroupKind=" + std::to_string((int) Group->getGroupKind()));

    switch (Group->getGroupKind()) {
        case ASTExprGroupKind::GROUP_UNARY:
            return GenUnary((ASTUnaryGroupExpr *) Group);
        case ASTExprGroupKind::GROUP_BINARY:
            return GenBinary((ASTBinaryGroupExpr *) Group);
        case ASTExprGroupKind::GROUP_TERNARY:
            return GenTernary((ASTTernaryGroupExpr *) Group);
    }

    assert("Unknown Group Kind");
}

llvm::Value *CodeGenExpr::GenUnary(ASTUnaryGroupExpr *Expr) {
    FLY_DEBUG("CodeGenExpr", "GenUnary");
    assert(Expr->getGroupKind() == ASTExprGroupKind::GROUP_UNARY  && "Expected Unary Group Expr");
    assert(Expr->getFirst() && "Unary Expr empty");

    CodeGenVarBase *CGVal = Expr->getFirst()->getVarRef()->getDef()->getCodeGen();
    llvm::Value *OldVal = CGVal->getValue();

    // PRE or POST INCREMENT/DECREMENT
    if (Expr->getOperatorKind() == ASTUnaryOperatorKind::ARITH_INCR) {
        llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
        Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
        CGVal->Store(NewVal);
        if (Expr->getOptionKind() == ASTUnaryOptionKind::UNARY_PRE) { // PRE INCREMENT ++a
            return NewVal;
        } else if (Expr->getOptionKind() == ASTUnaryOptionKind::UNARY_POST) { // POST INCREMENT a++
            return OldVal;
        } else {
            assert(0 && "Invalid Unary Option Kind");
        }
    }

    if (Expr->getOperatorKind() == ASTUnaryOperatorKind::ARITH_DECR) {
        llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
        Value *NewVal = CGM->Builder->CreateNSWAdd(OldVal, RHS);
        CGVal->Store(NewVal);
        if (Expr->getOptionKind() == ASTUnaryOptionKind::UNARY_PRE) { // PRE DECREMENT --a
            return NewVal;
        } else if (Expr->getOptionKind() == ASTUnaryOptionKind::UNARY_POST) { // POST DECREMENT a--
            return OldVal;
        } else {
            assert(0 && "Invalid Unary Option Kind");
        }
    }

    // NOT Operator '!'
    if (Expr->getOperatorKind() == ASTUnaryOperatorKind::LOGIC_NOT) {
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

    switch (Expr->getOptionKind()) {

        case ASTBinaryOptionKind::BINARY_ARITH:
            return GenBinaryArith(Expr->getFirst(), Expr->getOperatorKind(), Expr->getSecond());
        case ASTBinaryOptionKind::BINARY_COMPARISON:
            return GenBinaryComparison(Expr->getFirst(), Expr->getOperatorKind(), Expr->getSecond());
        case ASTBinaryOptionKind::BINARY_LOGIC:
            return GenBinaryLogic(Expr->getFirst(), Expr->getOperatorKind(), Expr->getSecond());
    }

    assert(0 && "Unknown Operation");
}

Value *CodeGenExpr::GenBinaryArith(const ASTExpr *E1, ASTBinaryOperatorKind Op, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryArith");
    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);

    // Convert E2 to E1 Type
    V2 = Convert(V2, E2->getType(), E1->getType()); // Implicit conversion

    switch (Op) {

        case ASTBinaryOperatorKind::ARITH_ADD:
            return CGM->Builder->CreateAdd(V1, V2);
        case ASTBinaryOperatorKind::ARITH_SUB:
            return CGM->Builder->CreateSub(V1, V2);
        case ASTBinaryOperatorKind::ARITH_MUL:
            return CGM->Builder->CreateMul(V1, V2);
        case ASTBinaryOperatorKind::ARITH_DIV:
            return CGM->Builder->CreateSDiv(V1, V2);
        case ASTBinaryOperatorKind::ARITH_MOD:
            return CGM->Builder->CreateSRem(V1, V2);
        case ASTBinaryOperatorKind::ARITH_AND:
            return CGM->Builder->CreateAnd(V1, V2);
        case ASTBinaryOperatorKind::ARITH_OR:
            return CGM->Builder->CreateOr(V1, V2);
        case ASTBinaryOperatorKind::ARITH_XOR:
            return CGM->Builder->CreateXor(V1, V2);
        case ASTBinaryOperatorKind::ARITH_SHIFT_L:
            return CGM->Builder->CreateShl(V1, V2);
        case ASTBinaryOperatorKind::ARITH_SHIFT_R:
            return CGM->Builder->CreateAShr(V1, V2);
    }
    assert(0 && "Unknown Arith Operation");
}

bool CodeGenExpr::isSigned(const ASTType * T1) {
    return T1->getKind() == ASTTypeKind::TYPE_SHORT ||
           T1->getKind() == ASTTypeKind::TYPE_INT ||
           T1->getKind() == ASTTypeKind::TYPE_LONG;
}

Value *CodeGenExpr::GenBinaryComparison(const ASTExpr *E1, ASTBinaryOperatorKind Op, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryComparison");
    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);
    ASTType *V2Type = E2->getType();

    if (V1->getType()->isIntegerTy() && V2->getType()->isIntegerTy()) {
        bool Signed = isSigned(E1->getType()) || isSigned(E2->getType());
        switch (Op) {

            case ASTBinaryOperatorKind::COMP_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOperatorKind::COMP_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
            case ASTBinaryOperatorKind::COMP_GT:
                return Signed ? CGM->Builder->CreateICmpSGT(V1, V2) : CGM->Builder->CreateICmpUGT(V1, V2);
            case ASTBinaryOperatorKind::COMP_GTE:
                return Signed ? CGM->Builder->CreateICmpSGE(V1, V2) : CGM->Builder->CreateICmpUGE(V1, V2);
            case ASTBinaryOperatorKind::COMP_LT:
                return Signed ? CGM->Builder->CreateICmpSLT(V1, V2) : CGM->Builder->CreateICmpULT(V1, V2);
            case ASTBinaryOperatorKind::COMP_LTE:
                return Signed ? CGM->Builder->CreateICmpSLE(V1, V2) : CGM->Builder->CreateICmpULE(V1, V2);
        }
    } else {
        // Convert values to Float if one of them is Float
        if ( (V1->getType()->isFloatTy() || V1->getType()->isDoubleTy()) &&
             (V2->getType()->isIntegerTy() || V2->getType()->isIntegerTy()) ) {
            V2 = Convert(V2, V2Type, E1->getType()); // Explicit conversion
        } else if ( (V1->getType()->isIntegerTy() || V1->getType()->isIntegerTy()) &&
                    (V2->getType()->isFloatTy() || V2->getType()->isDoubleTy()) ) {
            V1 = Convert(V1, V2Type, E1->getType()); // Explicit conversion
        }
        switch (Op) {

            case ASTBinaryOperatorKind::COMP_EQ:
                return CGM->Builder->CreateFCmpOEQ(V1, V2);
            case ASTBinaryOperatorKind::COMP_NE:
                return CGM->Builder->CreateFCmpONE(V1, V2);
            case ASTBinaryOperatorKind::COMP_GT:
                return CGM->Builder->CreateFCmpOGT(V1, V2);
            case ASTBinaryOperatorKind::COMP_GTE:
                return CGM->Builder->CreateFCmpOGE(V1, V2);
            case ASTBinaryOperatorKind::COMP_LT:
                return CGM->Builder->CreateFCmpOLT(V1, V2);
            case ASTBinaryOperatorKind::COMP_LTE:
                return CGM->Builder->CreateFCmpOLE(V1, V2);
        }
    }

    assert(0 && "Invalid Comparator Operator");
}

Value *CodeGenExpr::GenBinaryLogic(const ASTExpr *E1, ASTBinaryOperatorKind Op, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryLogic");
    llvm::Value *V1 = GenValue(E1);
    ASTBoolType *BoolType = SemaBuilder::CreateBoolType(SourceLocation());
    V1 = Convert(V1, E1->getType(), BoolType);
    BasicBlock *FromBB = CGM->Builder->GetInsertBlock();

    switch (Op) {

        case ASTBinaryOperatorKind::LOGIC_AND: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", Fn);
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", Fn);

            // From Branch
            CGM->Builder->CreateCondBr(V1, LeftBB, RightBB);

            // Left Branch
            CGM->Builder->SetInsertPoint(LeftBB);
            llvm::Value *PtrV2 = nullptr;
            llvm::Value *V2 = GenValue(E2, PtrV2);
            llvm::Value *V2Trunc = CGM->Builder->CreateTrunc(V2, CGM->BoolTy);
            CGM->Builder->CreateBr(RightBB);

            // Right Branch
            CGM->Builder->SetInsertPoint(RightBB);
            PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->BoolTy, false, false), FromBB);
            Phi->addIncoming(V2Trunc, LeftBB);
            return Phi;
        }
        case ASTBinaryOperatorKind::LOGIC_OR: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", Fn);
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or", Fn);

            // From Branch
            CGM->Builder->CreateCondBr(V1, RightBB, LeftBB);

            // Left Branch
            CGM->Builder->SetInsertPoint(LeftBB);
            llvm::Value *PtrV2 = nullptr;
            llvm::Value *V2 = GenValue(E2, PtrV2);
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

    ASTBoolType * BoolType = SemaBuilder::CreateBoolType(SourceLocation());
    llvm::Value *Cond = GenValue(Expr->getFirst());

    // Create Blocks
    llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "terntrue", Fn);
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternfalse", Fn);
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternend", Fn);

    // Create Condition
    CGM->Builder->CreateCondBr(Cond, TrueBB, FalseBB);

    // True Label
    CGM->Builder->SetInsertPoint(TrueBB);
    llvm::Value *True = GenValue(Expr->getSecond());
    llvm::Value *BoolTrue = Convert(True, Expr->getSecond()->getType(), BoolType);
    CGM->Builder->CreateBr(EndBB);

    // False Label
    CGM->Builder->SetInsertPoint(FalseBB);
    llvm::Value *False = GenValue(Expr->getThird());
    llvm::Value *BoolFalse = Convert(False, Expr->getThird()->getType(), BoolType);
    CGM->Builder->CreateBr(EndBB);

    // End Label
    CGM->Builder->SetInsertPoint(EndBB);
    PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
    Phi->addIncoming(BoolTrue, TrueBB);
    Phi->addIncoming(BoolFalse, FalseBB);
    return Phi;
}