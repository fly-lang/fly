//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SemaNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaModule.h"
#include "Sema/SemaVisitor.h"
#include <AST/ASTModule.h>
#include <llvm/IR/Module.h>

using namespace fly;

SemaModule::~SemaModule() {
	// Delete all owned Imports
	for (auto *Import : Imports) {
		delete Import;
	}

	// Delete all owned Nodes (Functions, Classes, Enums)
	for (auto *Node : Nodes) {
		delete Node;
	}
}

SemaModule::SemaModule(ASTModule &AST, SymbolTable *Symbols) : AST(AST), Symbols(Symbols) {

}

ASTModule &SemaModule::getAST() const {
	return AST;
}

SymbolTable *SemaModule::getSymbols() const {
	return Symbols;
}

llvm::StringRef SemaModule::getName() const {
	return AST.getName();
}

SemaNameSpace *SemaModule::getNameSpace() const {
	return NameSpace;
}

void SemaModule::setNameSpace(SemaNameSpace *NameSpace) {
	this->NameSpace = NameSpace;
}

const llvm::SmallVector<SemaImport *, 8> &SemaModule::getImports() const {
	return Imports;
}

void SemaModule::addImport(SemaImport *Import) {
	Imports.push_back(Import);
}

const llvm::SmallVector<SemaNode *, 8> &SemaModule::getNodes() const {
	return Nodes;
}

void SemaModule::addNode(SemaNode *Node) {
	Nodes.push_back(Node);
}

void SemaModule::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

