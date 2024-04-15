//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLocalVar.h"

using namespace fly;

ASTLocalVar::ASTLocalVar(ASTVarKind VarKind, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                         ASTScopes *Scopes) :
        ASTVar(VarKind, Loc, Type, Name, Scopes) {

}

ASTLocalVar::ASTLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes) :
        ASTVar(ASTVarKind::VAR_LOCAL, Loc, Type, Name, Scopes) {

}

CodeGenVarBase *ASTLocalVar::getCodeGen() const {
    return CodeGen;
}

void ASTLocalVar::setCodeGen(CodeGenVarBase *CG) {
    CodeGen = CG;
}

std::string ASTLocalVar::print() const {
    return getName().data();
}

std::string ASTLocalVar::str() const {
    return Logger("ASTLocalVar").
            Super(ASTVar::str()).
            End();
}
