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
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenVar.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTExpr.h"
#include "AST/ASTOperatorExpr.h"
#include "llvm/IR/Value.h"

using namespace fly;

CodeGenExpr::CodeGenExpr(CodeGenModule *CGM, ASTExpr *Expr, const ASTType *Type) : CGM(CGM), Type(Type) {
    Val = Generate(Expr);
    for (auto &Inc : PostIncrements) {

    }
}

llvm::Value *CodeGenExpr::getValue() const {
    return Val;
}

llvm::Value *CodeGenExpr::Generate(ASTExpr *Expr) {
    if (Expr->getKind() == EXPR_GROUP) {
        // Only for the first time generate post and pre operations
        GenIncDec((ASTGroupExpr *) Expr);

        return GenGroup((ASTGroupExpr *) Expr, new ASTGroupExpr(SourceLocation()), 0);
    }

    return GenValue(Expr);
}

llvm::Value *CodeGenExpr::GenValue(ASTExpr *Expr) {
    switch (Expr->getKind()) {

        case EXPR_VALUE: {
            ASTValueExpr *ValExpr = (ASTValueExpr *)Expr;
            return CGM->GenValue(Type, &ValExpr->getValue());
        }
        case EXPR_OPERATOR:
            CGM->Diag(Expr->getLocation(), diag::err_expr_operator_unexpected);
            return nullptr;
        case EXPR_REF_VAR: {
            ASTVarRefExpr *VarRefExpr = (ASTVarRefExpr *)Expr;
            assert(VarRefExpr->getVarRef() && "Missing Ref");
            ASTVar *VDecl = VarRefExpr->getVarRef()->getDecl();
            assert(VDecl && "Ref to undeclared var");
            if (VDecl->isGlobal()) {
                return ((ASTGlobalVar *) VDecl)->getCodeGen()->getGlobalVar();
            }
            return ((ASTLocalVar *) VDecl)->getCodeGen()->get();
        }
        case EXPR_REF_FUNC: {
            ASTFuncCallExpr *RefExp = (ASTFuncCallExpr *)Expr;
            assert(RefExp->getCall() && "Missing Ref");
            return CGM->GenCall(RefExp->getCall());
        }
        case EXPR_GROUP:
            assert(0 && "Cannot process Group from here");
    }
}

void CodeGenExpr::GenIncDec(ASTGroupExpr *Group) {
    // On first: identify all pre-increment or pre-decrement
    for (auto &Expr : Group->getGroup()) {
        if (Expr->getKind() == EXPR_GROUP) {
            GenIncDec((ASTGroupExpr *) Expr);
        } else if (Expr->getKind() == EXPR_OPERATOR && ((ASTOperatorExpr *)Expr)->getOpKind() == OP_INCDEC) {
            switch(((ASTIncDecExpr *)Expr)->getIncDecKind()) {

                case PRE_INCR:
                    break;
                case PRE_DECR:
                    break;
                case POST_INCR:
                    break;
                case POST_DECR:
                    break;
            }
        }
    }
}

llvm::Value *CodeGenExpr::GenGroup(ASTGroupExpr *Origin, ASTGroupExpr *New, int Idx, ASTExpr *E1,
                                   ASTOperatorExpr * OP1) {

    // Starting from: E2
    // Evaluate: E1 OP1 E2 OP2 E3
    // Example: E1 + E2 * E3 -> E1 + (E2 * E3)

    // Take first (no operator expected)
    auto E2 = Origin->getGroup().at(Idx++);

    // If E1 is Operation -> Error
    if (E2->getKind() == EXPR_OPERATOR) {
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
            llvm::Value *V = GenOperation(E1, OP1, E2);
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
                llvm::Value *V = GenOperation(E1, OP1, E2);
                E2 = new VirtualExpr(V);
            }
        }
    }

    // All Origin Group are Processed
    if (Idx > Origin->getGroup().size()-1) {

        if (OP2 != nullptr) {
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
            return GenGroup(((ASTGroupExpr *)E1), new ASTGroupExpr(SourceLocation()), 0);
        } else if (E2->getKind() == EXPR_VIRTUAL) {
            return ((VirtualExpr *)E1)->getVal();
        } else {
            return GenValue(E1);
        }
    }

    // Continue with recursion
    return GenGroup(Origin, New, Idx, E2, (ASTOperatorExpr *) OP2);
}

llvm::Value *CodeGenExpr::GenOperation(ASTExpr *E1, ASTOperatorExpr *Op, ASTExpr *E2) {
    assert(E1->getKind() != EXPR_OPERATOR && E1->getKind() != EXPR_GROUP && "Expr1 Error");
    assert(E2->getKind() != EXPR_OPERATOR && E2->getKind() != EXPR_GROUP && "Expr2 Error");

    llvm::Value *V = nullptr;
    switch (Op->getOpKind()) {

        case OP_ARITH:
            break;
        case OP_LOGIC:
            break;
        case OP_BOOL:
            break;
        case OP_INCDEC:
            // Already Done into GenIncDec
            break;
        case OP_COND:
            break;
    }

    return V;
}

bool CodeGenExpr::hasOpPrecedence(ASTExpr *OP) {
    return OP != nullptr && ((ASTOperatorExpr *)OP)->getOpKind() == OP_ARITH &&
           ( ((ASTArithExpr *)OP)->getArithKind() == ARITH_MUL || ((ASTArithExpr *)OP)->getArithKind() == ARITH_DIV);
}

bool CodeGenExpr::canIterate(int Idx, ASTGroupExpr *Group) {
    return Idx < Group->getGroup().size();
}
