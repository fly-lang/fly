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
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_REF_VAR");
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

llvm::Value *CodeGenExpr::Convert(llvm::Value *FromVal, const ASTType *FromType, const ASTType *ToType) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "Convert",
                      "Value=" << FromVal << " to ASTType=" << ToType->str());
    assert(ToType && "Invalid conversion type");

    llvm::Type *FromLLVMType = FromVal->getType();
    switch (ToType->getKind()) {

        // to BOOL
        case ASTTypeKind::TYPE_BOOL: {

            // from BOOL
            if (FromType->isBool()) {
                return CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->BoolTy);
            }

            // from Integer
            if (FromType->isInteger()) {
                llvm::Value *ZERO = llvm::ConstantInt::get(FromLLVMType, 0, ((ASTIntegerType *) FromType)->isSigned());
                return CGM->Builder->CreateICmpNE(FromVal, ZERO);
            }

            // from FLOATING POINT
            if (FromLLVMType->isFloatTy()) {
                llvm::Value *ZERO = llvm::ConstantFP::get(FromLLVMType, 0);
                return CGM->Builder->CreateFCmpUNE(FromVal, ZERO);
            }

            // default 0
            return llvm::ConstantInt::get(CGM->getCodeGen()->BoolTy, 0, false);
        }

        // to INTEGER
        case ASTTypeKind::TYPE_INTEGER: {
            ASTIntegerType *IntegerType = (ASTIntegerType *) ToType;
            switch(IntegerType->getIntegerKind()) {

                // to INT 8
                case ASTIntegerTypeKind::TYPE_BYTE: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->BoolTy);
                        return CGM->Builder->CreateZExt(ToVal, CGM->getCodeGen()->Int8Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == CGM->getCodeGen()->Int8Ty) {
                            return FromVal;
                        } else {
                            return CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->Int8Ty);
                        }
                    }

                    // from FLOATING POINT
                    if (FromLLVMType->isFloatingPointTy()) {
                        return CGM->Builder->CreateFPToUI(FromVal, CGM->getCodeGen()->Int8Ty);
                    }
                }

                // to INT 16
                case ASTIntegerTypeKind::TYPE_SHORT:
                case ASTIntegerTypeKind::TYPE_USHORT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->BoolTy);
                        return CGM->Builder->CreateZExt(ToVal, CGM->getCodeGen()->Int16Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == CGM->getCodeGen()->Int8Ty) {
                            return CGM->Builder->CreateZExt(FromVal, CGM->getCodeGen()->Int16Ty);
                        } else if (FromLLVMType == CGM->getCodeGen()->Int16Ty) {
                            return FromVal;
                        } else {
                            return CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->Int16Ty);
                        }
                    }

                    // from FLOATING POINT
                    if (FromLLVMType->isFloatingPointTy()) {
                        return IntegerType->isSigned() ? CGM->Builder->CreateFPToSI(FromVal, CGM->getCodeGen()->Int16Ty) :
                               CGM->Builder->CreateFPToUI(FromVal, CGM->getCodeGen()->Int16Ty);
                    }
                }

                // to INT 32
                case ASTIntegerTypeKind::TYPE_INT:
                case ASTIntegerTypeKind::TYPE_UINT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->BoolTy);
                        return CGM->Builder->CreateZExt(ToVal, CGM->getCodeGen()->Int32Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == CGM->getCodeGen()->Int8Ty || FromLLVMType == CGM->getCodeGen()->Int16Ty) {
                            return IntegerType->isSigned() ? CGM->Builder->CreateSExt(FromVal, CGM->getCodeGen()->Int32Ty) :
                                CGM->Builder->CreateZExt(FromVal, CGM->getCodeGen()->Int32Ty);
                        } else if (FromLLVMType == CGM->getCodeGen()->Int32Ty) {
                            return FromVal;
                        } else {
                            return CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->Int32Ty);
                        }
                    }

                    // from FLOATING POINT
                    if (FromLLVMType->isFloatingPointTy()) {
                        return IntegerType->isSigned() ? CGM->Builder->CreateFPToSI(FromVal, CGM->getCodeGen()->Int32Ty) :
                               CGM->Builder->CreateFPToUI(FromVal, CGM->getCodeGen()->Int32Ty);
                    }
                }

                // to INT 64
                case ASTIntegerTypeKind::TYPE_LONG:
                case ASTIntegerTypeKind::TYPE_ULONG: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->BoolTy);
                        return CGM->Builder->CreateZExt(ToVal, CGM->getCodeGen()->Int64Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == CGM->getCodeGen()->Int8Ty || FromLLVMType == CGM->getCodeGen()->Int16Ty ||
                            FromLLVMType == CGM->getCodeGen()->Int32Ty) {
                            return IntegerType->isSigned() ? CGM->Builder->CreateSExt(FromVal, CGM->getCodeGen()->Int64Ty) :
                                   CGM->Builder->CreateZExt(FromVal, CGM->getCodeGen()->Int64Ty);
                        } else {
                            return FromVal;
                        }
                    }

                    // from FLOATING POINT
                    if (FromType->isFloatingPoint()) {
                        return IntegerType->isSigned() ? CGM->Builder->CreateFPToSI(FromVal, CGM->getCodeGen()->Int64Ty) :
                               CGM->Builder->CreateFPToUI(FromVal, CGM->getCodeGen()->Int64Ty);
                    }
                }
            }
        }

        // to FLOATING POINT
        case ASTTypeKind::TYPE_FLOATING_POINT: {
            switch(((ASTFloatingPointType *) ToType)->getFloatingPointKind()) {

                // to FLOAT 32
                case ASTFloatingPointTypeKind::TYPE_FLOAT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        return CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->BoolTy);
                    }

                    // from INT
                    if (FromType->isInteger()) {
                        return ((ASTIntegerType *) FromType)->isSigned() ?
                               CGM->Builder->CreateSIToFP(FromVal, CGM->getCodeGen()->FloatTy) :
                               CGM->Builder->CreateUIToFP(FromVal, CGM->getCodeGen()->FloatTy);
                    }

                    // from FLOAT
                    if (FromType->isFloatingPoint()) {
                        switch (((ASTFloatingPointType *) FromType)->getFloatingPointKind()) {

                            case ASTFloatingPointTypeKind::TYPE_FLOAT:
                                return FromVal;
                            case ASTFloatingPointTypeKind::TYPE_DOUBLE:
                                return CGM->Builder->CreateFPTrunc(FromVal, CGM->getCodeGen()->FloatTy);
                        }
                    }
                }

                // to DOUBLE 64
                case ASTFloatingPointTypeKind::TYPE_DOUBLE: {

                    // from BOOL
                    if (FromType->isBool()) {
                        return CGM->Builder->CreateTrunc(FromVal, CGM->getCodeGen()->BoolTy);
                    }

                    // from INT
                    if (FromType->isInteger()) {
                        return ((ASTIntegerType *) FromType)->isSigned() ?
                               CGM->Builder->CreateSIToFP(FromVal, CGM->getCodeGen()->DoubleTy) :
                               CGM->Builder->CreateUIToFP(FromVal, CGM->getCodeGen()->DoubleTy);
                    }

                    // from FLOAT
                    if (FromType->isFloatingPoint()) {
                        switch (((ASTFloatingPointType *) FromType)->getFloatingPointKind()) {

                            case ASTFloatingPointTypeKind::TYPE_FLOAT:
                                return CGM->Builder->CreateFPExt(FromVal, CGM->getCodeGen()->DoubleTy);
                            case ASTFloatingPointTypeKind::TYPE_DOUBLE:
                                return FromVal;
                        }
                    }
                }
            }
        }

        // to Identity
        case ASTTypeKind::TYPE_IDENTITY:
            return FromVal; // TODO implement class cast
    }
    assert(0 && "Conversion failed");
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
        case ASTExprGroupKind::GROUP_BINARY:
            V = GenBinary((ASTBinaryGroupExpr *) Group);
        case ASTExprGroupKind::GROUP_TERNARY:
            V = GenTernary((ASTTernaryGroupExpr *) Group);
    }

    return V;
}

llvm::Value *CodeGenExpr::GenUnary(ASTUnaryGroupExpr *Expr) {
    FLY_DEBUG("CodeGenExpr", "GenUnary");
    assert(Expr->getGroupKind() == ASTExprGroupKind::GROUP_UNARY  && "Expected Unary Group Expr");
    assert(Expr->getFirst() && "Unary Expr empty");

    CodeGenVarBase *CGVal = Expr->getFirst()->getVarRef()->getDef()->getCodeGen();
    llvm::Value *OldVal = CGVal->getValue();

    // PRE or POST INCREMENT/DECREMENT
    if (Expr->getOperatorKind() == ASTUnaryOperatorKind::ARITH_INCR) {
        llvm::Value *RHS = llvm::ConstantInt::get(CGM->getCodeGen()->Int32Ty, 1);
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
        llvm::Value *RHS = llvm::ConstantInt::get(CGM->getCodeGen()->Int32Ty, -1, true);
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
        OldVal = CGM->Builder->CreateTrunc(OldVal, CGM->getCodeGen()->BoolTy);
        OldVal = CGM->Builder->CreateXor(OldVal, true);
        return CGM->Builder->CreateZExt(OldVal, CGM->getCodeGen()->Int8Ty);
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

Value *CodeGenExpr::GenBinaryComparison(const ASTExpr *E1, ASTBinaryOperatorKind Op, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryComparison");
    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);
    ASTType *V2Type = E2->getType();

    if (E1->getType()->isBool() && E2->getType()->isBool()) {
        switch (Op) {
            case ASTBinaryOperatorKind::COMP_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case ASTBinaryOperatorKind::COMP_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
        }
    } else if (E1->getType()->isInteger() && E2->getType()->isInteger()) {
        bool Signed = ((ASTIntegerType *) E1->getType())->isSigned() || ((ASTIntegerType *) E2->getType())->isSigned();
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
//    V1 = Convert(V1, E1->getType(), BoolType); //FIXME
    BasicBlock *FromBB = CGM->Builder->GetInsertBlock();

    switch (Op) {

        case ASTBinaryOperatorKind::LOGIC_AND: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and");
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and");

            // From Branch
            CGM->Builder->CreateCondBr(V1, LeftBB, RightBB);

            // Left Branch
            CGM->Builder->SetInsertPoint(LeftBB);
            llvm::Value *V2 = GenValue(E2);
            llvm::Value *V2Trunc = CGM->Builder->CreateTrunc(V2, CGM->getCodeGen()->BoolTy);
            CGM->Builder->CreateBr(RightBB);

            // Right Branch
            CGM->Builder->SetInsertPoint(RightBB);
            PHINode *Phi = CGM->Builder->CreatePHI(CGM->getCodeGen()->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->getCodeGen()->BoolTy, false, false), FromBB);
            Phi->addIncoming(V2Trunc, LeftBB);
            return Phi;
        }
        case ASTBinaryOperatorKind::LOGIC_OR: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or");
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "or");

            // From Branch
            CGM->Builder->CreateCondBr(V1, RightBB, LeftBB);

            // Left Branch
            CGM->Builder->SetInsertPoint(LeftBB);
            llvm::Value *V2 = GenValue(E2);
            llvm::Value *V2Trunc = CGM->Builder->CreateTrunc(V2, CGM->getCodeGen()->BoolTy);
            CGM->Builder->CreateBr(RightBB);

            // Right Branch
            CGM->Builder->SetInsertPoint(RightBB);
            PHINode *Phi = CGM->Builder->CreatePHI(CGM->getCodeGen()->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->getCodeGen()->BoolTy, true, false), FromBB);
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

    llvm::Value *Cond = GenValue(Expr->getFirst());

    // Create Blocks
    llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "terntrue");
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternfalse");
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "ternend");

    // Create Condition
    CGM->Builder->CreateCondBr(Cond, TrueBB, FalseBB);

    // True Label
    CGM->Builder->SetInsertPoint(TrueBB);
    llvm::Value *True = GenValue(Expr->getSecond());
//    llvm::Value *BoolTrue = Convert(True, Expr->getSecond()->getType(), BoolType); FIXME
    CGM->Builder->CreateBr(EndBB);

    // False Label
    CGM->Builder->SetInsertPoint(FalseBB);
    llvm::Value *False = GenValue(Expr->getThird());
//    llvm::Value *BoolFalse = Convert(False, Expr->getThird()->getType(), BoolType); FIXME
    CGM->Builder->CreateBr(EndBB);

    // End Label
    CGM->Builder->SetInsertPoint(EndBB);
    PHINode *Phi = CGM->Builder->CreatePHI(CGM->getCodeGen()->BoolTy, 2);
    Phi->addIncoming(True, TrueBB);
    Phi->addIncoming(False, FalseBB);
    return Phi;
}