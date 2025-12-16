//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SemaNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaModule.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaEnumType.h"

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

SemaModule::SemaModule(ASTModule &AST) : AST(AST) {

}

ASTModule &SemaModule::getAST() const {
	return AST;
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

const llvm::SmallVector<SemaNode *, 8> &SemaModule::getNodes() const {
	return Nodes;
}
