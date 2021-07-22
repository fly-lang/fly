//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExpr.cpp - Expression into a statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTExpr.h"

using namespace fly;

ASTValueExpr::ASTValueExpr(const SourceLocation &Loc, const ASTValue *Val) : Loc(Loc), Val(Val) {}

const SourceLocation &ASTValueExpr::getLocation() const {
    return Loc;
}

ExprKind ASTValueExpr::getKind() const {
    return Kind;
}

const ASTValue &ASTValueExpr::getValue() const {
    return *Val;
}

ExprKind ASTGroupExpr::getKind() const {
return Kind;
}

const std::vector<ASTExpr *> &ASTGroupExpr::getGroup() const {
    return Group;
}

bool ASTGroupExpr::isEmpty() const {
    return Group.empty();
}

void ASTGroupExpr::Add(ASTExpr *Exp) {
    Group.push_back(Exp);
}

ASTVarRefExpr::ASTVarRefExpr(const SourceLocation &Loc, ASTVarRef *Ref) : Loc(Loc), Ref(Ref) {}

const SourceLocation &ASTVarRefExpr::getLocation() const {
    return Loc;
}

ExprKind ASTVarRefExpr::getKind() const {
return Kind;
}

ASTVarRef *ASTVarRefExpr::getVarRef() const {
    return Ref;
}

ASTFuncCallExpr::ASTFuncCallExpr(const SourceLocation &Loc, ASTFuncCall *Ref) : Loc(Loc), Call(Ref) {}

const SourceLocation &ASTFuncCallExpr::getLocation() const {
    return Loc;
}

ExprKind ASTFuncCallExpr::getKind() const {
return Kind;
}

ASTFuncCall *ASTFuncCallExpr::getCall() const {
    return Call;
}
