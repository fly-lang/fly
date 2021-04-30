//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/FunctionDecl.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/FunctionDecl.h"

using namespace fly;

FunctionDecl::FunctionDecl(const SourceLocation &Loc, const TypeDecl *Type, const StringRef &Name) :
        DeclBase(Loc), Type(Type), Name(Name), Params(new ParamsFunc) {}

const StringRef &FunctionDecl::getName() const {
    return Name;
}

bool FunctionDecl::isConstant() const {
    return Constant;
}

const Stmt *FunctionDecl::getBody() const {
    return Body;
}

const ParamsFunc *FunctionDecl::getParams() const {
    return Params;
}

const TypeDecl *FunctionDecl::getType() const {
    return Type;
}

const std::vector<VarDecl *> &ParamsFunc::getVars() const {
    return Vars;
}

const VarDecl *ParamsFunc::getVarArg() const {
    return VarArg;
}

ReturnDecl::ReturnDecl(SourceLocation &Loc, class Expr *E) : DeclBase(Loc), Exp(E) {}

Expr *ReturnDecl::getExpr() const {
    return Exp;
}

DeclKind ReturnDecl::getKind() {
    return Kind;
}
