//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTAlias.cpp - AST Alias implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTAlias.h"

using namespace fly;

ASTAlias::ASTAlias(const SourceLocation &Loc, llvm::StringRef Name) : ASTBase(Loc, ASTKind::AST_ALIAS), Name(Name) {

}

llvm::StringRef ASTAlias::getName() const {
	return Name;
}

std::string ASTAlias::str() const {
    return Logger("ASTAlias").
            Super(ASTBase::str()).
            Attr("Name", Name).
            End();
}