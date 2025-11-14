//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTModule.cpp - AST Module implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTModule.h"
#include "AST/ASTVisitor.h"
#include "Basic/Logger.h"

#include <Frontend/InputFile.h>

using namespace fly;

ASTModule::ASTModule(InputFile *F) : ASTNode(SourceLocation(), ASTKind::AST_MODULE),
        File(F), Name(F->getName()), Header(F->getExt() == FileExt::FLY_H) {
}

ASTModule::~ASTModule() = default;

void ASTModule::accept(ASTVisitor& Visitor) {
	Visitor.visit(*this);
}

InputFile *ASTModule::getFile() const {
	return File;
}

bool ASTModule::isHeader() const {
    return Header;
}

std::string ASTModule::getName() {
    return Name;
}

ASTNameSpace *ASTModule::getNameSpace() {
    return NameSpace;
}

void ASTModule::setNameSpace(ASTNameSpace *NS) {
	NameSpace = NS;
}

const llvm::SmallVector<ASTNode *, 8> &ASTModule::getNodes() const {
	return Nodes;
}

void ASTModule::addNode(ASTNode *Node) {
	Nodes.push_back(Node);
}

std::string ASTModule::str() const {
    return Logger("ASTModule").
           Attr("Name", Name).
		   Attr("NameSpace", NameSpace).
           End();
}
