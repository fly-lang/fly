//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTAlias.cpp - AST Alias implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTAlias.h"
#include "Basic/Logger.h"

using namespace fly;

ASTAlias::ASTAlias(const SourceLocation &Loc, llvm::StringRef Name) : Name(Name), Loc(Loc) {

}

llvm::StringRef ASTAlias::getName() {
	return Name;
}
