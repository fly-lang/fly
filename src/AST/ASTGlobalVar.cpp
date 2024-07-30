// src/AST/ASTGlobalVar.cpp - AST Global Var implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTGlobalVar.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"

using namespace fly;

ASTGlobalVar::ASTGlobalVar(ASTModule *Module, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes) :
        ASTVar(ASTVarKind::VAR_GLOBAL, Loc, Type, Name, Scopes), Module(Module) {

}

ASTTopDefKind ASTGlobalVar::getTopDefKind() const {
    return TopDefKind;
}

ASTVisibilityKind ASTGlobalVar::getVisibility() const {
    return Visibility;
}

bool ASTGlobalVar::isConstant() const {
    return Constant;
}

ASTModule *ASTGlobalVar::getModule() const {
    return Module;
}

ASTNameSpace *ASTGlobalVar::getNameSpace() const {
    return Module->getNameSpace();
}

llvm::StringRef ASTGlobalVar::getName() const {
    return ASTVar::getName();
}

ASTExpr *ASTGlobalVar::getExpr() const {
    return Expr;
}

CodeGenGlobalVar *ASTGlobalVar::getCodeGen() const {
    return CodeGen;
}

void ASTGlobalVar::setCodeGen(CodeGenGlobalVar *CG) {
    CodeGen = CG;
}

std::string ASTGlobalVar::str() const {
    return Logger("ASTGlobalVar").
            Super(ASTVar::str()).
            End();
}
