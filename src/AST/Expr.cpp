//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/Expr.cpp - Expression into a statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/Expr.h"

using namespace fly;

ValueExpr::ValueExpr(const SourceLocation &Loc, const Value *Val) : Loc(Loc), Val(Val) {}

const SourceLocation &ValueExpr::getLocation() const {
    return Loc;
}

ExprKind ValueExpr::getKind() const {
    return Kind;
}

const Value &ValueExpr::getValue() const {
    return *Val;
}

ExprKind GroupExpr::getKind() const {
return Kind;
}

const std::vector<Expr *> &GroupExpr::getGroup() const {
    return Group;
}

bool GroupExpr::isEmpty() const {
    return Group.empty();
}

void GroupExpr::Add(Expr *Exp) {
    Group.push_back(Exp);
}

VarRefExpr::VarRefExpr(const SourceLocation &Loc, VarRef *Ref) : Loc(Loc), Ref(Ref) {}

const SourceLocation &VarRefExpr::getLocation() const {
    return Loc;
}

ExprKind VarRefExpr::getKind() const {
return Kind;
}

VarRef *VarRefExpr::getVarRef() const {
    return Ref;
}

FuncCallExpr::FuncCallExpr(const SourceLocation &Loc, FuncCall *Ref) : Loc(Loc), Call(Ref) {}

const SourceLocation &FuncCallExpr::getLocation() const {
    return Loc;
}

ExprKind FuncCallExpr::getKind() const {
return Kind;
}

FuncCall *FuncCallExpr::getCall() const {
    return Call;
}
