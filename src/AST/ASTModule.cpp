//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTModule.cpp - AST Module implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTModule.h"

using namespace fly;

ASTModule::ASTModule(uint64_t &Id, const std::string Name, bool isHeader) :
        Id(Id), Name(Name), Header(isHeader) {
}

ASTModule::~ASTModule() = default;

const uint64_t ASTModule::getId() const {
    return Id;
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

const llvm::SmallVector<ASTBase *, 8> &ASTModule::getDefinitions() const {
	return Definitions;
}

std::string ASTModule::str() const {
    return Logger("ASTModule").
           Attr("Name", Name).
		   Attr("NameSpace", NameSpace).
           End();
}
