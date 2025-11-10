//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaImport.cpp - The Sema Import implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaImport.h"

#include <AST/ASTAlias.h>
#include <AST/ASTImport.h>

using namespace fly;

SemaImport::SemaImport(ASTImport &AST) : SemaNode(SemaKind::IMPORT), AST(AST) {
}

ASTImport* SemaImport::getAST() const {
	return &AST;
}

llvm::StringRef SemaImport::getName() const {
	return AST.getAlias() == nullptr ? AST.getName() : AST.getAlias()->getName();
}

llvm::StringRef SemaImport::getTarget() const {
	return AST.getName();
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