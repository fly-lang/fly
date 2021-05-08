//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/VarDecl.cpp - Var implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/VarDecl.h"

using namespace fly;

VarDecl::VarDecl(const SourceLocation &Loc, TypeBase *Type, const StringRef Name) :
        Decl(Loc), Type(Type), Name(Name) {}

DeclKind VarDecl::getKind() const {
    return Kind;
}

bool VarDecl::isConstant() const {
    return Constant;
}

TypeBase* VarDecl::getType() const {
    return Type;
}

const llvm::StringRef &VarDecl::getName() const {
    return Name;
}

GroupExpr *VarDecl::getExpr() const {
    return Expression;
}

VarDecl::~VarDecl() {
    delete Type;
}

VarRef::VarRef(const SourceLocation &Loc, const StringRef &Name) : Refer(Loc), Name(Name) {

}

VarRef::VarRef(const SourceLocation &Loc, VarDecl *D) : Refer(Loc), Name(D->getName()), Var(D) {

}

const llvm::StringRef &VarRef::getName() const {
    return Name;
}

VarDecl *VarRef::getDecl() const {
    return Var;
}

VarRefDecl::VarRefDecl(const SourceLocation &Loc, const StringRef &Name) : Decl(Loc), VarRef(Loc, Name),
                                                                           Expr(new GroupExpr) {

}

VarRefDecl::VarRefDecl(const SourceLocation &Loc, VarDecl *D) : Decl(Loc), VarRef(Loc, D), Expr(new GroupExpr) {

}

DeclKind VarRefDecl::getKind() const {
    return R_VAR;
}

GroupExpr *VarRefDecl::getExpr() const {
    return Expr;
}
