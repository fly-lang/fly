//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExpr.cpp - Expression into a statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTExpr.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunc.h"

using namespace fly;

ASTExpr::ASTExpr(const SourceLocation &Loc) : Loc(Loc) {

}

const SourceLocation &ASTExpr::getLocation() const {
    return Loc;
}

ASTValueExpr::ASTValueExpr(const SourceLocation &Loc, const ASTValue *Val) : ASTExpr(Loc), Val(Val) {

}

ASTExprKind ASTValueExpr::getKind() const {
    return Kind;
}

const ASTValue &ASTValueExpr::getValue() const {
    return *Val;
}

ASTType *ASTValueExpr::getType() const {
    return Val->getType();
}

std::string ASTValueExpr::str() const {
    return "{ Type=" + getType()->str() +
           ", Kind=" + std::to_string(Kind) +
           ", Value=" + Val->str() +
           " }";
}

ASTVarRefExpr::ASTVarRefExpr(const SourceLocation &Loc, ASTVarRef *Ref) : ASTExpr(Loc), Ref(Ref) {

}

ASTExprKind ASTVarRefExpr::getKind() const {
return Kind;
}

ASTVarRef *ASTVarRefExpr::getVarRef() const {
    return Ref;
}

ASTType *ASTVarRefExpr::getType() const {
    return Ref->getDecl()->getType();
}

std::string ASTVarRefExpr::str() const {
    return "{ Type=" + getType()->str() +
           ", Kind=" + std::to_string(Kind) +
           ", Ref=" + Ref->str() +
           " }";
}

ASTFuncCallExpr::ASTFuncCallExpr(const SourceLocation &Loc, ASTFuncCall *Ref) : ASTExpr(Loc), Call(Ref) {}

ASTExprKind ASTFuncCallExpr::getKind() const {
return Kind;
}

ASTFuncCall *ASTFuncCallExpr::getCall() const {
    return Call;
}

ASTType *ASTFuncCallExpr::getType() const {
    return Call->getDecl()->getType();
}

std::string ASTFuncCallExpr::str() const {
    return "{ Type=" + getType()->str() +
           ", Kind=" + std::to_string(Kind) +
           ", Call=" + Call->str() +
           " }";
}

ASTGroupExpr::ASTGroupExpr(const SourceLocation &Loc) : ASTExpr(Loc) {

}

ASTExprKind ASTGroupExpr::getKind() const {
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

ASTType *ASTGroupExpr::getType() const {
    if (isEmpty()) {
        return nullptr;
    }
    return Group.at(0)->getType();
}

std::string ASTGroupExpr::str() const {
    std::string Str = "{ Type=" + getType()->str() +
                      ", Kind=" + std::to_string(Kind) +
                      ", Group=[";
    if (!Group.empty()) {
        for (ASTExpr *Expr: Group) {
            Str += Expr->str() + ", ";
        }
        Str = Str.substr(0, Str.length()-2);
    }
    Str += "] }";
    return Str;
}