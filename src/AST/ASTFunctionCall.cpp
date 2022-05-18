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
#include "AST/ASTNameSpace.h"

using namespace fly;

ASTFunctionCall::ASTFunctionCall(const SourceLocation &Loc, const std::string &NameSpace, const std::string &Name) :
    Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

const SourceLocation &ASTFunctionCall::getLocation() const {
    return Loc;
}

const std::string &ASTFunctionCall::getName() const {
    return Name;
}

const std::vector<ASTExpr*> ASTFunctionCall::getArgs() const {
    return Args;
}

ASTFunction *ASTFunctionCall::getDef() const {
    return Def;
}

CodeGenCall *ASTFunctionCall::getCodeGen() const {
    return CGC;
}

void ASTFunctionCall::setCodeGen(CodeGenCall *C) {
    CGC = C;
}

const std::string &ASTFunctionCall::getNameSpace() const {
    return NameSpace;
}

void ASTFunctionCall::setNameSpace(const std::string &NS) {
    NameSpace = NS;
}

std::string ASTFunctionCall::str() const {
    std::string Str = "{ NameSpace=" + NameSpace +
           ", Name=" + Name +
           ", Args=[";
    if (!Args.empty()) {
        for (ASTExpr *Arg : Args) {
            Str += Arg->str() + ", ";
        }
        Str = Str.substr(0, Str.length()-2);
    }
    Str += "] }";
    return Str;
}
