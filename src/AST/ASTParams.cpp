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
#include "AST/ASTFunctionBase.h"
#include "AST/ASTExpr.h"

#include <string>

using namespace fly;

ASTParam::ASTParam(ASTFunctionBase *Function, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, bool Constant) :
        ASTLocalVar(Function->Body, Loc, Type, Name, Constant) {

}

std::string ASTParam::str() const {
    return Logger("ASTParam").
           Super(ASTLocalVar::str()).
           End();
}

uint64_t ASTParams::getSize() const {
    return List.size();
}

ASTParam *ASTParams::at(unsigned long Index) const {
    return List.at(Index);
}

const std::vector<ASTParam *> &ASTParams::getList() const {
    return List;
}

const ASTParam *ASTParams::getEllipsis() const {
    return Ellipsis;
}

std::string ASTParams::str() const {
    return Logger("ASTParams").
            AttrList("List", List).
            End();
}
