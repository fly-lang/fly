//===--------------------------------------------------------------------------------------------------------------===//
// src/Sys/Sys.cpp - The Sys Fail Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sys/Sys.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "Sema/SemaBuilder.h"

using namespace fly;

void Sys::Build(ASTNameSpace *NameSpace) {
    SemaBuilder::InsertFunction(NameSpace->Functions, getFail0());
    SemaBuilder::InsertFunction(NameSpace->Functions, getFail1());
    SemaBuilder::InsertFunction(NameSpace->Functions, getFail2());
    SemaBuilder::InsertFunction(NameSpace->Functions, getFail3());
}

ASTFunction *Sys::getFail0() {
    const SourceLocation &Loc = SourceLocation();
    ASTVoidType *VoidType = SemaBuilder::CreateVoidType(Loc);
    ASTFunction *Fail = SemaBuilder::CreateFunction(nullptr, Loc, VoidType, "fail", SemaBuilder::CreateScopes());
    return Fail;
}

ASTFunction *Sys::getFail1() {
    const SourceLocation &Loc = SourceLocation();
    ASTVoidType *VoidType = SemaBuilder::CreateVoidType(Loc);
    ASTFunction *Fail = SemaBuilder::CreateFunction(nullptr, Loc, VoidType, "fail", SemaBuilder::CreateScopes());
    ASTParam *Param = SemaBuilder::CreateParam(Fail, Loc, SemaBuilder::CreateUIntType(Loc), "code", false);
    Fail->addParam(Param);
    return Fail;
}

ASTFunction *Sys::getFail2() {
    const SourceLocation &Loc = SourceLocation();
    ASTVoidType *VoidType = SemaBuilder::CreateVoidType(Loc);
    ASTFunction *Fail = SemaBuilder::CreateFunction(nullptr, Loc, VoidType, "fail", SemaBuilder::CreateScopes());
    ASTParam *Param = SemaBuilder::CreateParam(Fail, Loc, SemaBuilder::CreateStringType(Loc), "message", false);
    Fail->addParam(Param);
    return Fail;
}

ASTFunction *Sys::getFail3() {
    const SourceLocation &Loc = SourceLocation();
    ASTVoidType *VoidType = SemaBuilder::CreateVoidType(Loc);
    ASTFunction *Fail = SemaBuilder::CreateFunction(nullptr, Loc, VoidType, "fail", SemaBuilder::CreateScopes());
    // TODO
//    ASTParam *Param = SemaBuilder::CreateParam(Fail, Loc, SemaBuilder::CreateIdentityType("Error"), "enabled", false);
//    Fail->addParam(Param);
    return Fail;
}

ASTVar *Sys::getError() {
    return SemaBuilder::CreateLocalVar(nullptr, SourceLocation(), SemaBuilder::CreateErrorType(), "error");
}
