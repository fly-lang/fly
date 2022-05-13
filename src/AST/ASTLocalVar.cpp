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

ASTLocalVar::ASTLocalVar(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant,
                         ASTExpr *Expr) :
                        ASTStmt(Loc), ASTVar(VAR_LOCAL, Type, Name, Constant) {

}

StmtKind ASTLocalVar::getKind() const {
    return Kind;
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
