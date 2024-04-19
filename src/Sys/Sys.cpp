//===--------------------------------------------------------------------------------------------------------------===//
// src/Sys/Sys.cpp - The Sys Fail Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sys/Sys.h"
#include "AST/ASTContext.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"

using namespace fly;

void Sys::Build(Sema &Se) {
    Sys *S = new Sys(Se);
    S->AddFailFunctions();
    delete S;
}

Sys::Sys(Sema &S) : S(S) {

}

void Sys::AddFailFunctions() {
    ASTNameSpace *NameSpace = S.getContext()->getDefaultNameSpace();

    const SourceLocation &Loc = SourceLocation();
    ASTVoidType *VoidType = SemaBuilder::CreateVoidType(Loc);

    ASTFunction *Fail0 = SemaBuilder::CreateFunction(nullptr, Loc, VoidType, "fail", SemaBuilder::CreateScopes());
    S.getBuilder()->AddFunction(Fail0);

    ASTFunction *Fail1 = SemaBuilder::CreateFunction(nullptr, Loc, VoidType, "fail", SemaBuilder::CreateScopes());
    ASTParam *ParamFail1 = SemaBuilder::CreateParam(Loc, SemaBuilder::CreateUIntType(Loc), "code");
    S.getBuilder()->AddParam(Fail1, ParamFail1);
    S.getBuilder()->AddFunction(Fail1);

    ASTFunction *Fail2 = SemaBuilder::CreateFunction(nullptr, Loc, VoidType, "fail", SemaBuilder::CreateScopes());
    ASTParam *ParamFail2 = SemaBuilder::CreateParam(Loc, SemaBuilder::CreateStringType(Loc), "message");
    S.getBuilder()->AddParam(Fail2, ParamFail2);
    S.getBuilder()->AddFunction(Fail2);

    ASTFunction *Fail3 = SemaBuilder::CreateFunction(nullptr, Loc, VoidType, "fail", SemaBuilder::CreateScopes());
    // TODO
//    ASTParam *Param = SemaBuilder::CreateParam(Fail, Loc, SemaBuilder::CreateIdentityType("Error"), "enabled", false);
//    S.getBuilder()->AddParam(Fail3, ParamFail3);
//    S.getBuilder()->AddFunction(Fail3);
}

