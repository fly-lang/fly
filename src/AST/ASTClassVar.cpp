//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassField.cpp - Class Field implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassVar.h"
#include "AST/ASTClass.h"
#include "AST/ASTType.h"
#include "AST/ASTVar.h"
#include "CodeGen/CodeGenClass.h"

using namespace fly;

ASTClassVar::ASTClassVar(const SourceLocation &Loc, ASTClass *Class, ASTScopes *Scopes, ASTType *Type,
                         llvm::StringRef Name) :
        ASTVar(ASTVarKind::VAR_CLASS, Loc, Type, Name, Scopes), Class(Class) {

}

ASTClass *ASTClassVar::getClass() const {
    return Class;
}

ASTVarStmt *ASTClassVar::getInit() const {
    return Init;
}

void ASTClassVar::setInit(ASTVarStmt *VarDefine) {
    Init = VarDefine;
}

CodeGenVarBase *ASTClassVar::getCodeGen() const {
    return CodeGen;
}

void ASTClassVar::setCodeGen(CodeGenVarBase *CodeGen) {
    this->CodeGen = CodeGen;
}

std::string ASTClassVar::print() const {
    return Class->print() + "." + getName().data();
}

std::string ASTClassVar::str() const {
    return Logger("ASTClassVar").
            Super(ASTVar::str()).
            End();
}
