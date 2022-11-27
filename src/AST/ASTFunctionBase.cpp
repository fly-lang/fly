//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunctionBase.cpp - Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunctionBase.h"
#include "AST/ASTBlock.h"
#include "AST/ASTParams.h"
#include "AST/ASTType.h"

#include <string>

using namespace fly;

ASTFunctionBase::ASTFunctionBase(const SourceLocation &Loc, ASTFunctionKind Kind, ASTType *ReturnType, llvm::StringRef Name)
        : Kind(Kind), Type(ReturnType), Name(Name), Location(Loc) {

}

llvm::StringRef ASTFunctionBase::getName() const {
    return Name;
}

const SourceLocation &ASTFunctionBase::getLocation() const {
    return Location;
}

const ASTBlock *ASTFunctionBase::getBody() const {
    return Body;
}

const ASTParams *ASTFunctionBase::getParams() const {
    return Params;
}

ASTFunctionKind ASTFunctionBase::getKind() {
    return Kind;
}

ASTType *ASTFunctionBase::getType() const {
    return Type;
}

const std::vector<ASTLocalVar *> &ASTFunctionBase::getLocalVars() const {
    return LocalVars;
}

CodeGenFunction *ASTFunctionBase::getCodeGen() const {
    return CodeGen;
}

void ASTFunctionBase::setCodeGen(CodeGenFunction *CGF) {
    CodeGen = CGF;
}

bool ASTFunctionBase::isVarArg() {
    return Params->getEllipsis();
}

std::string ASTFunctionBase::str() const {
    return Logger("ASTFunctionBase").
           Attr("Name", Name).
           Attr("Params", Params).
           Attr("ReturnType", Type).
           End();
}

ASTReturn::ASTReturn(ASTBlock *Parent, const SourceLocation &Loc) :
        ASTExprStmt(Parent, Loc, ASTStmtKind::STMT_RETURN) {

}

std::string ASTReturn::str() const {
    return Logger("ASTReturn").
            Attr("Kind", (uint64_t) Kind).
            End();
}
