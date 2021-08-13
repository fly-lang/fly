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
#include "AST/ASTOperatorExpr.h"
#include "llvm/IR/Value.h"

using namespace fly;

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM, ASTExpr *Expr, const ASTType *Type) : CGM(CGM) {
    Val = Generate(Expr);
    for (auto &Value : PostValues) {
        CGM->Builder->Insert(Value);
    }
    Convert(Val, Type);
}

llvm::Value *CodeGenExpr::getValue() const {
    return Val;
}

llvm::Value *CodeGenExpr::Generate(ASTExpr *Expr) {
    // Generate Values from a Group
    if (Expr->getKind() == EXPR_GROUP) {
        return GenGroup((ASTGroupExpr *) Expr, new ASTGroupExpr(SourceLocation()), 0);
    }
    // Generate Value from a single expr
    return GenValue(Expr);
}

llvm::Value *CodeGenExpr::Convert(llvm::Value *V, const ASTType *ToType) {
    switch (ToType->getKind()) {
        case TYPE_INT: // TO INT 32
            return Convert(V, CGM->Int32Ty);
        case TYPE_FLOAT: // TO FLOAT 32
            return Convert(V, CGM->FloatTy);
        case TYPE_BOOL: // INT 8
            return Convert(V, CGM->BoolTy);
        case TYPE_CLASS:
            return nullptr;
    }
}

llvm::Value *CodeGenExpr::Convert(llvm::Value *V, llvm::Type *ToType) {
    llvm::Type *FromType = V->getType();

    switch (ToType->getTypeID()) {
        
        case Type::FloatTyID:
            if (FromType->isIntegerTy()) { // INT to FLOAT
                return CGM->Builder->CreateSIToFP(V, ToType);
            } else if (FromType->isDoubleTy()) { // INT to DOUBLE
                return CGM->Builder->CreateFPExt(V, ToType); // FIXME
            } // else do not convert because are equal types
        case Type::DoubleTyID:
            // TODO
            break;
        case Type::IntegerTyID: {
            unsigned int ToBit = ToType->getIntegerBitWidth();
            // TO INT
            if (FromType->isIntegerTy()) {
                unsigned int FromBit = ToType->getIntegerBitWidth();
                if (FromBit < ToBit) { // INT TO LONG, BOOL TO INT ...
                    return CGM->Builder->CreateZExt(V, CGM->Int32Ty);
                } else if (FromBit > ToBit) {
                    if (ToBit == 1) { // INT TO BOOL
                        llvm::Value *LHS = llvm::ConstantInt::get(CGM->BoolTy, 0, false);
                        V = CGM->Builder->CreateICmpNE(LHS, V);
                        return CGM->Builder->CreateZExt(V, CGM->Int8Ty);
                    } else { // LONG TO INT
                        V = CGM->Builder->CreateTrunc(V, ToType);
                        return CGM->Builder->CreateZExt(V, ToType);
                    }
                } // else do not convert because are equal types
            } else if (FromType->isFloatTy() || FromType->isDoubleTy()) { // TO FLOAT or DOUBLE
                if (ToBit == 1) {
                    llvm::Value *LHS = llvm::ConstantInt::get(CGM->BoolTy, 0, false);
                    V = CGM->Builder->CreateFCmpUNE(LHS, V);
                    return CGM->Builder->CreateZExt(V, CGM->Int8Ty);
                } else {
                    return CGM->Builder->CreateFPToSI(V, ToType);
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
}

llvm::Value *CodeGenExpr::GenValue(ASTExpr *Expr) {
    switch (Expr->getKind()) {

        case EXPR_VIRTUAL:
            return ((VirtualExpr *)Expr)->getVal();
        case EXPR_VALUE: {
            return CGM->GenValue(Expr->getType(), &((ASTValueExpr *)Expr)->getValue());
        }
        case EXPR_OPERATOR:
            if (((ASTOperatorExpr *) Expr)->isUnary()) {
                return OpUnary((ASTUnaryExpr *) Expr);
            }
            assert(0 && "Operator unexpected here!");
        case EXPR_REF_VAR: {
            ASTVarRefExpr *VarRefExpr = (ASTVarRefExpr *)Expr;
            assert(VarRefExpr->getVarRef() && "Missing Ref");
            ASTVar *VDecl = VarRefExpr->getVarRef()->getDecl();
            assert(VDecl && "Ref to undeclared var");
            if (VDecl->isGlobal()) {
                return ((ASTGlobalVar *) VDecl)->getCodeGen()->getGlobalVar();
            }
            return ((ASTVar *) VDecl)->getCodeGen()->get();
        }
        case EXPR_REF_FUNC: {
            ASTFuncCallExpr *RefExp = (ASTFuncCallExpr *)Expr;
            assert(RefExp->getCall() && "Missing Ref");
            return CGM->GenCall(RefExp->getCall());
        }
        case EXPR_GROUP:
            assert(0 && "Cannot process Group from here");
    }
    return nullptr;
}

llvm::Value *CodeGenExpr::GenGroup(ASTGroupExpr *Origin, ASTGroupExpr *New, int Idx, ASTExpr *E1,
                                   ASTOperatorExpr * OP1) {

    // Starting from: E2
    // Evaluate: E1 OP1 E2 OP2 E3
    // Example: E1 + E2 * E3 -> E1 + (E2 * E3)

    // Take first (no operator expected)
    ASTExpr *E2 = Origin->getGroup().at(Idx++);

    // If E1 is a Binary Operator -> Error
    if (E2->getKind() == EXPR_OPERATOR && ((ASTOperatorExpr *) E2)->isBinary()) {
        CGM->Diag(E2->getLocation(), diag::err_expr_operator_unexpected);
        return nullptr;
    }

    // Check if there is a next (operator expected)
    ASTExpr *OP2 = nullptr;
    if (canIterate(Idx, Origin)) {
        OP2 = Origin->getGroup().at(Idx++);

        // Check unexpected Operator
        if (OP2->getKind() != EXPR_OPERATOR) {
            CGM->Diag(E2->getLocation(), diag::err_expr_operator_expected);
            return nullptr;
        }
    }

    // Generate Operation from last Expr, last Operation and current Expr, adding to the New Group
    if (E1 && OP1) {

        // If E1 is a Group -> Generate Value
        if (E1->getKind() == EXPR_GROUP) {
            llvm::Value *V = GenGroup(((ASTGroupExpr *)E1), new ASTGroupExpr(SourceLocation()), 0);
            E1 = new VirtualExpr(V);
        }

        // If OP is * or / try to process the operation
        if (hasOpPrecedence(OP1)){

            // If E2 is a Group -> Generate Value (always after E1)
            if (E2->getKind() == EXPR_GROUP) {
                llvm::Value *V = GenGroup(((ASTGroupExpr *)E2), new ASTGroupExpr(SourceLocation()), 0);
                E2 = new VirtualExpr(V);
            }

            // E1 * E2
            llvm::Value *V = OpBinary(E1, OP1, E2);
            E2 = new VirtualExpr(V);
        } else {
            if (hasOpPrecedence(OP2)) { // E1 + E2 * E3

                // Or add to a New Group for processing on next cycle and continue recursion
                New->Add(E1);
                New->Add(OP1);
            } else { // E1 + E2 + E3

                // If E2 is a Group -> Generate Value (always after E1)
                if (E2->getKind() == EXPR_GROUP) {
                    llvm::Value *V = GenGroup(((ASTGroupExpr *)E2), new ASTGroupExpr(SourceLocation()), 0);
                    E2 = new VirtualExpr(V);
                }

                // E1 + E2
                llvm::Value *V = OpBinary(E1, OP1, E2);
                E2 = new VirtualExpr(V);
            }
        }
    }

    // All Origin Group are Processed
    if (Idx > Origin->getGroup().size()-1) {

        if (OP2 != nullptr) { // cannot have Operation at end
            CGM->Diag(OP2->getLocation(), diag::err_expr_operator_unexpected);
            return nullptr;
        }

        // If New Group is not empty need other recursions, reset Idx to 0 and restart generator
        if (!New->isEmpty()) {
            New->Add(E2);
            return GenGroup(New, new ASTGroupExpr(SourceLocation()), 0);
        }

        // Remain only one Expr into New Group, the last, need to be returned
        if (E2->getKind() == EXPR_GROUP) { // If E1 is a Group -> Generate Value
            return GenGroup(((ASTGroupExpr *)E2), new ASTGroupExpr(SourceLocation()), 0);
        } else if (E2->getKind() == EXPR_VIRTUAL) {
            return ((VirtualExpr *)E2)->getVal();
        } else {
            return GenValue(E2);
        }
    }

    // Continue with recursion
    return GenGroup(Origin, New, Idx, E2, (ASTOperatorExpr *) OP2);
}

bool CodeGenExpr::hasOpPrecedence(ASTExpr *OP) {
    return OP != nullptr && ((ASTOperatorExpr *)OP)->getOpKind() == OP_ARITH &&
           ( ((ASTArithExpr *)OP)->getArithKind() == ARITH_MUL || ((ASTArithExpr *)OP)->getArithKind() == ARITH_DIV);
}

bool CodeGenExpr::canIterate(int Idx, ASTGroupExpr *Group) {
    return Idx < Group->getGroup().size();
}

llvm::Value *CodeGenExpr::OpUnary(ASTUnaryExpr *E) {
    assert(E->getKind() != EXPR_OPERATOR && E->getKind() != EXPR_GROUP && "Expr Error");

    llvm::Value *V = GenValue(E);
    ASTOperatorExpr *OP = E->getOperatorExpr();

    // PRE or POST INCREMENT/DECREMENT
    if (OP->getOpKind() == OP_ARITH) {
        switch(((ASTArithExpr *)OP)->getArithKind()) {
            case ARITH_INCR:
                if (E->getUnaryKind() == UNARY_PRE) { // PRE INCREMENT ++a
                    llvm::Value *RHS = llvm::ConstantInt::get(CGM->BoolTy, 1);
                    return CGM->Builder->CreateNSWAdd(V, RHS);
                } else { // POST INCREMENT a++
                    llvm::Value *PostV = BinaryOperator::Create(Instruction::Add, V,
                                                                ConstantInt::get(CGM->BoolTy, 1));
                    PostValues.push_back(PostV);
                    return V;
                }
                break;
            case ARITH_DECR:
                if (E->getUnaryKind() == UNARY_PRE) { // PRE DECREMENT --a
                    llvm::Value *RHS = llvm::ConstantInt::get(CGM->BoolTy, -1, true);
                    return CGM->Builder->CreateNSWAdd(V, RHS);
                } else { // POST DECREMENT a--
                    llvm::Value *PostV = BinaryOperator::Create(Instruction::Add, V,
                                                                ConstantInt::get(CGM->BoolTy, 1));
                    PostValues.push_back(PostV);
                    return V;
                }
                break;
        }
    }

    // NOT Operator '!'
    if (OP->getOpKind() == OP_LOGIC && ((ASTLogicExpr *)OP)->getLogicKind() == LOGIC_NOT) {
        V = CGM->Builder->CreateTrunc(V, CGM->BoolTy);
        V = CGM->Builder->CreateXor(V, true);
        return CGM->Builder->CreateZExt(V, CGM->Int8Ty);
    }

    assert(0 && "Not Unary Operation");
}

llvm::Value *CodeGenExpr::OpBinary(ASTExpr *E1, ASTOperatorExpr *OP, ASTExpr *E2) {
    assert(E1->getKind() != EXPR_OPERATOR && E1->getKind() != EXPR_GROUP && "E1 Error");
    assert(E2->getKind() != EXPR_OPERATOR && E2->getKind() != EXPR_GROUP && "E2 Error");

    llvm::Value *V1 = GenValue(E1);
    llvm::Value *V2 = GenValue(E2);
    
    switch (OP->getOpKind()) {

        case OP_ARITH:
            return OpArith(V1, (ASTArithExpr *) OP, V2);
        case OP_COMPARISON:
            return OpComparison(V1, (ASTComparisonExpr *) OP, V2);
        case OP_LOGIC:
            return OpLogic(V1, (ASTLogicExpr *) OP, V2);
        case OP_COND:
//            return OpCond(); // TODO
            break;
    }

    return nullptr;
}

Value *CodeGenExpr::OpArith(llvm::Value *V1, ASTArithExpr *OP, llvm::Value *V2) {
    V2 = Convert(V2, V1->getType()); // Implicit conversion

    switch (OP->getArithKind()) {

        case ARITH_ADD:
            return CGM->Builder->CreateAdd(V1, V2);
            break;
        case ARITH_SUB:
            return CGM->Builder->CreateSub(V1, V2);
            break;
        case ARITH_MUL:
            return CGM->Builder->CreateMul(V1, V2);
            break;
        case ARITH_DIV:
            return CGM->Builder->CreateSDiv(V1, V2);
            break;
        case ARITH_MOD:
            return CGM->Builder->CreateSRem(V1, V2);
            break;
        case ARITH_AND:
            return CGM->Builder->CreateAnd(V1, V2);
            break;
        case ARITH_OR:
            return CGM->Builder->CreateOr(V1, V2);
            break;
        case ARITH_XOR:
            return CGM->Builder->CreateXor(V1, V2);
            break;
        case ARITH_SHIFT_L:
            return CGM->Builder->CreateShl(V1, V2);
            break;
        case ARITH_SHIFT_R:
            return CGM->Builder->CreateAShr(V1, V2);
            break;
        case ARITH_INCR:
        case ARITH_DECR:
            // done into GenIncDec() on start
            break;
    }
    return nullptr;
}

Value *CodeGenExpr::OpComparison(llvm::Value *V1, fly::ASTComparisonExpr *OP, llvm::Value *V2) {
    if (V1->getType()->isIntegerTy() && V2->getType()->isIntegerTy()) {
        switch (OP->getComparisonKind()) {

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
        switch (OP->getComparisonKind()) {

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

Value *CodeGenExpr::OpLogic(llvm::Value *V1, ASTLogicExpr *OP, llvm::Value *V2) {
    switch (OP->getLogicKind()) {

        case LOGIC_AND:

            break;
        case LOGIC_OR:
            break;
    }
    return nullptr;
}
