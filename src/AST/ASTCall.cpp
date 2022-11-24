//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunc.cpp - AST Function Call implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTCall.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "Basic/Debuggable.h"

using namespace fly;

ASTArg::ASTArg(ASTCall *Call, ASTExpr *Expr) : Expr(Expr), Call(Call) {

}

uint64_t ASTArg::getIndex() const {
    return Index;
}

ASTParam *ASTArg::getDef() const {
    return Def;
}

std::string ASTArg::str() const {
    return Logger("ASTArg").
            Attr("Expr", Expr).
            Attr("Index", Index).
            End();
}

ASTCall *ASTArg::getCall() const {
    return Call;
}

ASTExpr *ASTArg::getExpr() const {
    return Expr;
}

ASTCall::ASTCall(const SourceLocation &Loc, llvm::StringRef NameSpace, llvm::StringRef Name) :
        ASTIdentifier(Loc, NameSpace, Name) {
}

ASTCall::ASTCall(const SourceLocation &Loc, llvm::StringRef NameSpace, llvm::StringRef ClassName, llvm::StringRef Name) :
        ASTIdentifier(Loc, ClassName, Name) {
    setNameSpace(NameSpace);
}

const std::vector<ASTArg*> ASTCall::getArgs() const {
    return Args;
}

ASTFunctionBase *ASTCall::getDef() const {
    return Def;
}

CodeGenCall *ASTCall::getCodeGen() const {
    return CGC;
}

std::string ASTCall::str() const {
    return Logger("ASTFunctionCall").
            Attr("Loc", Loc).
            Attr("Name", Name).
            Attr("NameSpace", NameSpace).
            AttrList("Args", Args).
            Attr("Def", Def).
            End();
}
