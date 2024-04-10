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
        ASTLocalVar(Loc, Type, Name, Constant), VarKind(ASTVarKind::VAR_PARAM),
        Function(Function) {

}

ASTFunctionBase *ASTParam::getFunction() {
    return Function;
}

ASTVarKind ASTParam::getVarKind() {
    return VarKind;
}

ASTType *ASTParam::getType() const {
    return Type;
}

llvm::StringRef ASTParam::getName() const {
    return Name;
}

ASTValue *ASTParam::getDefaultValue() const {
    return DefaultValue;
}

void ASTParam::setDefaultValue(ASTValue *Value) {
    DefaultValue = Value;
}

CodeGenVarBase *ASTParam::getCodeGen() const {
    return CodeGen;
}

void ASTParam::setCodeGen(CodeGenVarBase *CG) {
    CodeGen = CG;
}

std::string ASTParam::print() const {
    return Name.data();
}

std::string ASTParam::str() const {
    return Logger("ASTParam").
            Super(ASTLocalVar::str()).
            Attr("Type", Type).
            Attr("Name", Name).
            Attr("VarKind", (uint64_t) VarKind).
            Attr("Constant", Constant).
            End();
}

uint64_t ASTParams::getSize() const {
    return List.size();
}

ASTParam *ASTParams::at(unsigned long Index) const {
    return List.at(Index);
}

const bool ASTParams::isEmpty() const {
    List.empty() && Ellipsis == nullptr;
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
