//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunc.cpp - AST Function Call implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunctionCall.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "Basic/Debuggable.h"

using namespace fly;

ASTArg::ASTArg(ASTFunctionCall *Call, const SourceLocation &Loc) :
        ASTExprStmt(nullptr, Loc, ASTStmtKind::STMT_ARG), Call(Call) {

}

uint64_t ASTArg::getIndex() const {
    return Index;
}

ASTParam *ASTArg::getDef() const {
    return Def;
}

std::string ASTArg::str() const {
    return Logger("ASTArg").
            Super(ASTExprStmt::str()).
            Attr("Index", Index).
            End();
}

ASTFunctionCall *ASTArg::getCall() const {
    return Call;
}

ASTFunctionCall::ASTFunctionCall(const SourceLocation &Loc, const std::string NameSpace, const std::string Name) :
    Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

const std::string ASTFunctionCall::getName() const {
    return Name;
}

const std::vector<ASTArg*> ASTFunctionCall::getArgs() const {
    return Args;
}

ASTFunctionBase *ASTFunctionCall::getDef() const {
    return Def;
}

CodeGenCall *ASTFunctionCall::getCodeGen() const {
    return CGC;
}

const SourceLocation &ASTFunctionCall::getLocation() const {
    return Loc;
}

const std::string ASTFunctionCall::getNameSpace() const {
    return NameSpace;
}

std::string ASTFunctionCall::str() const {
    return Logger("ASTFunctionCall").
            Attr("Loc", Loc).
            Attr("Expr", Expr).
            Attr("Name", Name).
            Attr("NameSpace", NameSpace).
            AttrList("Args", Args).
            Attr("Def", Def).
            End();
}
