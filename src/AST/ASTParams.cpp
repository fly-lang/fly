//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunc.cpp - AST Params implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTParams.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTStmt.h"
#include "AST/ASTBlock.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTContext.h"
#include <string>

using namespace fly;

ASTParam::ASTParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Costant) :
        ASTLocalVar(Loc, Type, Name, Constant) {

}

std::string ASTParam::str() const {
    return "{ " + ASTLocalVar::str() +
            ", Expr=" + (Expr ? Expr->str() : "{}") +
            " }";
}

const std::vector<ASTParam *> &ASTParams::getList() const {
    return List;
}

const ASTParam *ASTParams::getEllipsis() const {
    return Ellipsis;
}
