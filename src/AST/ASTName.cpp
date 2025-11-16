//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTName.cpp - AST Name implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTName.h"

using namespace fly;

ASTName::ASTName(const SourceLocation &Loc, llvm::StringRef Name) : ASTBase(Loc, ASTKind::AST_NAME), Name(Name) {
}

llvm::StringRef ASTName::getName() const {
	return Name;
}

std::string ASTName::str() const {
	return ASTBase::str();
}
