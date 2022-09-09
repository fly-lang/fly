//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunc.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTStmt.h"
#include "AST/ASTBlock.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTContext.h"
#include <string>

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc, ASTNode *Node, ASTType *ReturnType, const std::string Name,
                         VisibilityKind Visibility) :
        ASTTopDef(Loc, Node, TopDeclKind::DECL_FUNCTION, Visibility),
        Type(ReturnType), Name(Name) {

}

const std::string ASTFunction::getName() const {
    return Name;
}

const ASTBlock *ASTFunction::getBody() const {
    return Body;
}

const ASTParams *ASTFunction::getParams() const {
    return Params;
}

ASTType *ASTFunction::getType() const {
    return Type;
}

const std::vector<ASTLocalVar *> &ASTFunction::getLocalVars() const {
    return LocalVars;
}

CodeGenFunction *ASTFunction::getCodeGen() const {
    return CodeGen;
}

void ASTFunction::setCodeGen(CodeGenFunction *CGF) {
    CodeGen = CGF;
}

bool ASTFunction::isVarArg() {
    return Params->getEllipsis();
}

std::string ASTFunction::str() const {
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

ASTReturn::ASTReturn(ASTBlock *Parent, const SourceLocation &Loc) : ASTExprStmt(Parent, Loc, STMT_RETURN) {

}

std::string ASTReturn::str() const {
    return "{ Kind=" + std::to_string(Kind) +
           " }";
}
