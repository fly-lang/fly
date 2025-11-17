//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaImport.cpp - The Sema Import implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaImport.h"

#include <AST/ASTImport.h>
#include <AST/ASTIdentifier.h>

using namespace fly;

SemaImport::SemaImport(ASTImport &AST) : SemaNode(SemaKind::IMPORT), AST(AST) {
}

ASTImport* SemaImport::getAST() const {
	return &AST;
}

const llvm::SmallVector<ASTName *, 4> &SemaImport::getNames() const {
	return AST.getAlias().empty() ? AST.getNames() : AST.getAlias();
}

const llvm::SmallVector<ASTName *, 4> &SemaImport::getTarget() const {
	return AST.getNames();
}

SymbolTable * SemaImport::getSymbols() const {
	return Symbols;
}

void SemaImport::setSymbols(SymbolTable *Symbols) {
	this->Symbols = Symbols;
}

void SemaImport::addSymbol(SymbolTable *Symbols) {
	this->Symbols = Symbols;
}