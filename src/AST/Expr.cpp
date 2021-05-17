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

ValueExpr::ValueExpr(const SourceLocation &Loc, const llvm::StringRef Str) : Loc(Loc), Str(Str) {}

const SourceLocation &ValueExpr::getLocation() const {
    return Loc;
}

ExprKind ValueExpr::getKind() const {
    return Kind;
}

const llvm::StringRef &ValueExpr::getString() const {
    return Str;
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

VarRefExpr::VarRefExpr(const SourceLocation &Loc, VarRef *Ref) : Loc(Loc), Ref(Ref) {}

const SourceLocation &VarRefExpr::getLocation() const {
    return Loc;
}

ExprKind VarRefExpr::getKind() const {
return Kind;
}

VarRef *VarRefExpr::getRef() const {
    return Ref;
}

FuncRefExpr::FuncRefExpr(const SourceLocation &Loc, FuncCall *Ref) : Loc(Loc), Ref(Ref) {}

const SourceLocation &FuncRefExpr::getLocation() const {
    return Loc;
}

ExprKind FuncRefExpr::getKind() const {
return Kind;
}

FuncCall *FuncRefExpr::getRef() const {
    return Ref;
}