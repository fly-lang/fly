//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassVar.cpp - Class Attribute implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassAttribute.h"
#include "AST/ASTClass.h"
#include "CodeGen/CodeGenClassVar.h"

using namespace fly;

ASTClassAttribute::ASTClassAttribute(const SourceLocation &Loc, ASTType *Type,
                                     llvm::StringRef Name, ASTScopes *Scopes) :
        ASTVar(ASTVarKind::VAR_CLASS, Loc, Type, Name, Scopes) {

}

ASTClass *ASTClassAttribute::getClass() const {
    return Class;
}

ASTVarStmt *ASTClassAttribute::getInit() const {
    return Init;
}

void ASTClassAttribute::setInit(ASTVarStmt *VarDefine) {
    Init = VarDefine;
}

CodeGenVarBase *ASTClassAttribute::getCodeGen() const {
    return CodeGen;
}

void ASTClassAttribute::setCodeGen(CodeGenVarBase *CG) {
    this->CodeGen = CG;
}

std::string ASTClassAttribute::print() const {
    return Class->print() + "." + getName().data();
}

std::string ASTClassAttribute::str() const {
    return Logger("ASTClassVar").
            Super(ASTVar::str()).
            End();
}