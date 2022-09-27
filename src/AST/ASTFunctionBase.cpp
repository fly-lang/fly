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

ASTFunctionBase::ASTFunctionBase(ASTType *ReturnType, const std::string Name) :
        Type(ReturnType), Name(Name) {

}

const std::string ASTFunctionBase::getName() const {
    return Name;
}

const ASTBlock *ASTFunctionBase::getBody() const {
    return Body;
}

const ASTParams *ASTFunctionBase::getParams() const {
    return Params;
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
    std::string Str = "{ Name=" + Name +
            ", Params=[";
    if(!Params->getList().empty()) {
        for (ASTParam *Param: Params->getList()) {
            Str += Param->str() + ", ";
        }
        Str = Str.substr(0, Str.length()-2);
    }
    Str += "], ReturnType=" + Type->str();
    return Str;
}

ASTReturn::ASTReturn(ASTBlock *Parent, const SourceLocation &Loc) : ASTExprStmt(Parent, Loc, StmtKind::STMT_RETURN) {

}

std::string ASTReturn::str() const {
    return "ASTReturn { Kind=" + std::to_string((int) Kind) +
           " }";
}
