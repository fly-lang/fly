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
#include "AST/ASTGlobalVar.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTExpr.h"
#include "Basic/Debug.h"
#include "llvm/IR/Value.h"

using namespace fly;

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM, llvm::Function *Fn, ASTExpr *Expr, const ASTType *ToType) :
        CGM(CGM), Fn(Fn) {
    FLY_DEBUG("CodeGenExpr", "CodeGenExpr");
    llvm::Value *Val = GenValue(Expr);
    for (auto &Value : PostValues) {
        CGM->Builder->Insert(Value);
    }
    Val = Convert(Val, (Expr)->getType(), ToType);
}

llvm::Value *CodeGenExpr::getValue() const {
    return Val;
}

llvm::Value *CodeGenExpr::Convert(llvm::Value *V, const ASTType *FromType, const ASTType *ToType) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "Convert",
                      "Value=" << V << " to ASTType=" << ToType->str());
    assert(ToType && "Invalid conversion type");
    bool SignedInt = FromType->getKind() == TYPE_SHORT || FromType->getKind() == TYPE_INT ||
            FromType->getKind() == TYPE_LONG;

    switch (ToType->getKind()) {
        case TYPE_BOOL:
            return Convert(V, CGM->BoolTy, SignedInt); // to INT 1
        case TYPE_BYTE:
            return Convert(V, CGM->Int8Ty, SignedInt); // to INT 8
        case TYPE_USHORT:
            return Convert(V, CGM->Int16Ty, SignedInt); // to Unsigned INT 16
        case TYPE_SHORT:
            return Convert(V, CGM->Int16Ty, SignedInt); // to Signed INT 16
        case TYPE_UINT:
            return Convert(V, CGM->Int32Ty, SignedInt); // to Unsigned INT 32
        case TYPE_INT:
            return Convert(V, CGM->Int32Ty, SignedInt); // to Signed INT 32
        case TYPE_ULONG:
            return Convert(V, CGM->Int64Ty, SignedInt); // to Unsigned INT 64
        case TYPE_LONG:
            return Convert(V, CGM->Int64Ty, SignedInt); // to Signed INT 64
        case TYPE_FLOAT:
            return Convert(V, CGM->FloatTy, SignedInt); // to FLOAT 32
        case TYPE_DOUBLE:
            return Convert(V, CGM->DoubleTy, SignedInt); // to DOUBLE 64

        case TYPE_CLASS:
            return nullptr;
    }
    assert(0 && "Conversion failed");
}

llvm::Value *CodeGenExpr::Convert(llvm::Value *V, llvm::Type *ToType, bool SignedInt) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "Convert",
                      "Value=" << V << " to TypeID=" << ToType->getTypeID());

    llvm::Type *FromType = V->getType();
    switch (ToType->getTypeID()) {

        case Type::DoubleTyID:
            if (FromType->isIntegerTy()) { // INT to DOUBLE
                return SignedInt ? CGM->Builder->CreateSIToFP(V, ToType) : CGM->Builder->CreateUIToFP(V, ToType);
            } else if (FromType->isFloatTy()) { // FLOAT to DOUBLE
                return CGM->Builder->CreateFPExt(V, ToType);
            } else if (FromType->isDoubleTy()) { // DOUBLE to DOUBLE
                return V;
            }

        case Type::FloatTyID:
            if (FromType->isIntegerTy()) { // INT to FLOAT
                return SignedInt ? CGM->Builder->CreateSIToFP(V, ToType) : CGM->Builder->CreateUIToFP(V, ToType);
            } else if (FromType->isDoubleTy()) { // DOUBLE to FLOAT
                return CGM->Builder->CreateFPTrunc(V, ToType);
            } else if (FromType->isFloatTy()) { // FLOAT to FLOAT
                return V;
            }

        case Type::IntegerTyID: { // To BOOL INT UINT SHORT USHORT LONG ULONG
            unsigned int ToBit = ToType->getIntegerBitWidth();
            // TO INT
            if (FromType->isIntegerTy()) {
                unsigned int FromBit = FromType->getIntegerBitWidth();
                if (FromBit < ToBit) { // INT TO LONG, BOOL TO INT ...
                    return SignedInt ? CGM->Builder->CreateSExt(V, CGM->Int32Ty) : CGM->Builder->CreateZExt(V, CGM->Int32Ty);
                } else if (FromBit > ToBit) {
                    if (ToBit == 1) { // INT TO BOOL
                        llvm::Value *LHS = llvm::ConstantInt::get(CGM->BoolTy, 0, false);
                        V = CGM->Builder->CreateICmpNE(LHS, V);
                        return CGM->Builder->CreateZExt(V, CGM->Int8Ty);
                    } else { // LONG TO INT
                        V = CGM->Builder->CreateTrunc(V, ToType);
                        return SignedInt ? CGM->Builder->CreateSExt(V, ToType) : CGM->Builder->CreateZExt(V, ToType);
                    }
                } // else do not convert because are equal types
                return V;
            } else if (FromType->isFloatTy() || FromType->isDoubleTy()) { // TO FLOAT or DOUBLE
                if (ToBit == 1) {
                    llvm::Value *LHS = llvm::ConstantInt::get(CGM->BoolTy, 0, false);
                    V = CGM->Builder->CreateFCmpUNE(LHS, V);
                    return CGM->Builder->CreateZExt(V, CGM->Int8Ty);
                } else {
                    return SignedInt ? CGM->Builder->CreateFPToSI(V, ToType) : CGM->Builder->CreateFPToUI(V, ToType);
                }
            }
        }
//        case Type::FunctionTyID:
//            break;
//        case Type::StructTyID:
//            break;
//        case Type::ArrayTyID:
//            break;
//        case Type::PointerTyID:
//            break;
    }
    assert(0 && "Unknown conversion Type");
}

llvm::Value *CodeGenExpr::GenValue(const ASTExpr *Expr, llvm::Value *Pointer) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "Expr=" << Expr->str());
    switch (Expr->getKind()) {

        case EXPR_VALUE: {
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_VALUE");
            return CGM->GenValue(Expr->getType(), &((ASTValueExpr *)Expr)->getValue());
        }
        case EXPR_REF_VAR: {
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_REF_VAR");
            ASTVarRefExpr *VarRefExpr = (ASTVarRefExpr *)Expr;
            assert(VarRefExpr->getVarRef() && "Missing Ref");
            ASTVar *Var = VarRefExpr->getVarRef()->getDecl();
            if (Var == nullptr) {
                CGM->Diag(VarRefExpr->getLocation(), diag::err_unref_var) << VarRefExpr->getVarRef()->getName();
                return nullptr;
            }
            Pointer = Var->getCodeGen()->getPointer(); // TODO what do you do with?
            return Var->getCodeGen()->getValue();
        }
        case EXPR_REF_FUNC: {
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_REF_FUNC");
            ASTFuncCallExpr *CallExpr = (ASTFuncCallExpr *)Expr;
            return CGM->GenCall(Fn, CallExpr->getCall());
        }
        case EXPR_GROUP:
            FLY_DEBUG_MESSAGE("CodeGenExpr", "GenValue", "EXPR_GROUP");
            return GenGroup((ASTGroupExpr *) Expr);
    }
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
    FLY_DEBUG_MESSAGE("CodeGenExpr", "GenGroup", "GroupKind=" + Group->getGroupKind());

    switch (Group->getGroupKind()) {
        case GROUP_UNARY:
            return GenUnary((ASTUnaryGroupExpr *) Group);
        case GROUP_BINARY:
            return GenBinary((ASTBinaryGroupExpr *) Group);
        case GROUP_TERNARY:
            return GenTernary((ASTTernaryGroupExpr *) Group);
    }
}

bool CodeGenExpr::hasOpPrecedence(BinaryOpKind Op) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "hasOpPrecedence", "Op=" + std::to_string(Op));
    return Op == ARITH_MUL || Op == ARITH_DIV;
}

llvm::Value *CodeGenExpr::GenUnary(ASTUnaryGroupExpr *Expr) {
    FLY_DEBUG("CodeGenExpr", "GenUnary");
    assert(Expr->getGroupKind() == GROUP_UNARY  && "Expected Unary Group Expr");
    assert(Expr->getFirst() && "Unary Expr empty");

    llvm::Value *V = Expr->getFirst()->getVarRef()->getDecl()->getCodeGen()->getValue();

    // PRE or POST INCREMENT/DECREMENT
    if (Expr->getOperatorKind() == ARITH_INCR) {
        if (Expr->getOptionKind() == UNARY_PRE) { // PRE INCREMENT ++a
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, 1);
            return CGM->Builder->CreateNSWAdd(V, RHS);
        } else if (Expr->getOptionKind() == UNARY_POST) { // POST INCREMENT a++
            llvm::Value *PostV = BinaryOperator::Create(Instruction::Add, V,
                                                        ConstantInt::get(CGM->Int32Ty, 1));
            PostValues.push_back(PostV);
            return V;
        } else {
            assert(0 && "Invalid Unary Option Kind");
        }
    }

    if (Expr->getOperatorKind() == ARITH_DECR) {
        if (Expr->getOptionKind() == UNARY_PRE) { // PRE DECREMENT --a
            llvm::Value *RHS = llvm::ConstantInt::get(CGM->Int32Ty, -1, true);
            return CGM->Builder->CreateNSWAdd(V, RHS);
        } else if (Expr->getOptionKind() == UNARY_POST) { // POST DECREMENT a--
            llvm::Value *PostV = BinaryOperator::Create(Instruction::Add, V,
                                                        ConstantInt::get(CGM->Int32Ty, -1));
            PostValues.push_back(PostV);
            return V;
        } else {
            assert(0 && "Invalid Unary Option Kind");
        }
    }

    // NOT Operator '!'
    if (Expr->getOperatorKind() == LOGIC_NOT) {
        V = CGM->Builder->CreateTrunc(V, CGM->BoolTy);
        V = CGM->Builder->CreateXor(V, true);
        return CGM->Builder->CreateZExt(V, CGM->Int8Ty);
    }

    assert(0 && "Invalid Unary Operation");
}

llvm::Value *CodeGenExpr::GenBinary(ASTBinaryGroupExpr *Expr) {
    FLY_DEBUG("CodeGenExpr", "GenBinary");
    assert(Expr->getGroupKind() != GROUP_BINARY && "Expected Binary Group Expr");
    assert(Expr->getFirst() && "First Expr is empty");
    assert(Expr->getSecond() && "Second Expr is empty");

    switch (Expr->getOptionKind()) {

        case BINARY_ARITH:
            return GenBinaryArith(Expr->getFirst(), Expr->getOperatorKind(), Expr->getSecond());
        case BINARY_COMPARISON:
            return GenBinaryComparison(Expr->getFirst(), Expr->getOperatorKind(), Expr->getSecond());
        case BINARY_LOGIC:
            return GenBinaryLogic(Expr->getFirst(), Expr->getOperatorKind(), Expr->getSecond());
    }

    assert(0 && "Unknown Operation");
}

Value *CodeGenExpr::GenBinaryArith(const ASTExpr *E1, BinaryOpKind Op, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryArith");
    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);
    V2 = Convert(V2, V1->getType()); // Implicit conversion

    switch (Op) {

        case ARITH_ADD:
            return CGM->Builder->CreateAdd(V1, V2);
        case ARITH_SUB:
            return CGM->Builder->CreateSub(V1, V2);
        case ARITH_MUL:
            return CGM->Builder->CreateMul(V1, V2);
        case ARITH_DIV:
            return CGM->Builder->CreateSDiv(V1, V2);
        case ARITH_MOD:
            return CGM->Builder->CreateSRem(V1, V2);
        case ARITH_AND:
            return CGM->Builder->CreateAnd(V1, V2);
        case ARITH_OR:
            return CGM->Builder->CreateOr(V1, V2);
        case ARITH_XOR:
            return CGM->Builder->CreateXor(V1, V2);
        case ARITH_SHIFT_L:
            return CGM->Builder->CreateShl(V1, V2);
        case ARITH_SHIFT_R:
            return CGM->Builder->CreateAShr(V1, V2);
    }
    assert(0 && "Unknown Arith Operation");
}

Value *CodeGenExpr::GenBinaryComparison(const ASTExpr *E1, BinaryOpKind Op, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryComparison");
    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);

    if (V1->getType()->isIntegerTy() && V2->getType()->isIntegerTy()) {
        switch (Op) {

            case COMP_EQ:
                return CGM->Builder->CreateICmpEQ(V1, V2);
            case COMP_NE:
                return CGM->Builder->CreateICmpNE(V1, V2);
            case COMP_GT:
                return CGM->Builder->CreateICmpSGT(V1, V2);
            case COMP_GTE:
                return CGM->Builder->CreateICmpSGE(V1, V2);
            case COMP_LT:
                return CGM->Builder->CreateICmpSLT(V1, V2);
            case COMP_LTE:
                return CGM->Builder->CreateICmpSLE(V1, V2);
        }
    } else {
        // Convert values to Float if one of them is Float
        if ( (V1->getType()->isFloatTy() || V1->getType()->isDoubleTy()) &&
             (V2->getType()->isIntegerTy() || V2->getType()->isIntegerTy()) ) {
            V2 = Convert(V2, V1->getType()); // Explicit conversion
        } else if ( (V1->getType()->isIntegerTy() || V1->getType()->isIntegerTy()) &&
                    (V2->getType()->isFloatTy() || V2->getType()->isDoubleTy()) ) {
            V1 = Convert(V1, V2->getType()); // Explicit conversion
        }
        switch (Op) {

            case COMP_EQ:
                return CGM->Builder->CreateFCmpOEQ(V1, V2);
            case COMP_NE:
                return CGM->Builder->CreateFCmpONE(V1, V2);
            case COMP_GT:
                return CGM->Builder->CreateFCmpOGT(V1, V2);
            case COMP_GTE:
                return CGM->Builder->CreateFCmpOGE(V1, V2);
            case COMP_LT:
                return CGM->Builder->CreateFCmpOLT(V1, V2);
            case COMP_LTE:
                return CGM->Builder->CreateFCmpOLE(V1, V2);
        }
    }
}

Value *CodeGenExpr::GenBinaryLogic(const ASTExpr *E1, BinaryOpKind Op, const ASTExpr *E2) {
    FLY_DEBUG("CodeGenExpr", "GenBinaryLogic");
    llvm::Value *V1 = GenValue(E1);
    V1 = Convert(V1, CGM->BoolTy);
    BasicBlock *FromBB = CGM->Builder->GetInsertBlock();

    switch (Op) {

        case LOGIC_AND: {
            llvm::BasicBlock *LeftBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", Fn);
            llvm::BasicBlock *RightBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "and", Fn);

            // From Branch
            CGM->Builder->CreateCondBr(V1, LeftBB, RightBB);

            // Left Branch
            CGM->Builder->SetInsertPoint(LeftBB);
            llvm::Value *PtrV2 = nullptr;
            llvm::Value *V2 = GenValue(E2, PtrV2);
            V2 = Convert(V2, CGM->BoolTy);
            llvm::Value *V2Trunc = CGM->Builder->CreateTrunc(V2, CGM->BoolTy);
            CGM->Builder->CreateBr(RightBB);

            // Right Branch
            CGM->Builder->SetInsertPoint(RightBB);
            PHINode *Phi = CGM->Builder->CreatePHI(CGM->BoolTy, 2);
            Phi->addIncoming(llvm::ConstantInt::get(CGM->BoolTy, false, false), FromBB);
            Phi->addIncoming(V2Trunc, LeftBB);
            return CGM->Builder->CreateZExt(Phi, CGM->Int8Ty);
        }
        case LOGIC_OR: {
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
            return CGM->Builder->CreateZExt(Phi, CGM->Int8Ty);
        }
    }
    assert(0 && "Invalid Logic Operator");
}

llvm::Value *CodeGenExpr::GenTernary(ASTTernaryGroupExpr *Expr) {
    assert(Expr->getGroupKind() != GROUP_TERNARY && "Expected Ternary Group Expr");
    assert(Expr->getFirst() && "First Expr is empty");
    assert(Expr->getSecond() && "Second Expr is empty");
    assert(Expr->getFirst() && "Third Expr is empty");

    // TODO
}