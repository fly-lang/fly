//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTLocalVar.cpp - AST Local Var Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTLocalVar.h"
#include "AST/ASTValue.h"

using namespace fly;

ASTLocalVar::ASTLocalVar(const SourceLocation &Loc, ASTBlock *Block, ASTType *Type, const std::string &Name,
                         bool Constant) :
        ASTStmt(Loc, Block), ASTVar(VAR_LOCAL, Type, Name, Constant) {
    if (Type->getKind() == TYPE_ARRAY) {
        setExpr(new ASTValueExpr(new ASTArrayValue(Loc, ((ASTArrayType *)Type)->getType())));
    }
}

StmtKind ASTLocalVar::getKind() const {
    return Kind;
}

ASTVarRef *ASTLocalVar::CreateVarRef() {
    ASTVarRef *VarRef = new ASTVarRef(Location, Name);
    VarRef->setDecl(this);
    return VarRef;
}

CodeGenLocalVar *ASTLocalVar::getCodeGen() const {
    return CodeGen;
}

void ASTLocalVar::setCodeGen(CodeGenLocalVar *CG) {
    CodeGen = CG;
}

std::string ASTLocalVar::str() const {
    return "{ " +
           ASTVar::str() +
           ", Kind: " + std::to_string(Kind) +
           " }";
}
