//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/VarDecl.cpp - Var declaration Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/VarDeclStmt.h"

using namespace fly;

VarDeclStmt::VarDeclStmt(const SourceLocation &Loc, BlockStmt *CurrStmt, TypeBase *Type, const StringRef Name) :
        Stmt(Loc, CurrStmt), VarDecl(Type, Name) {}

StmtKind VarDeclStmt::getKind() const {
    return Kind;
}

CodeGenVar *VarDeclStmt::getCodeGen() const {
    return CodeGen;
}

void VarDeclStmt::setCodeGen(CodeGenVar *CG) {
    CodeGen = CG;
}