//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNameSpaceRef.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTNameSpaceRef.h"
#include "llvm/ADT/StringExtras.h"

using namespace fly;

ASTNameSpaceRef::ASTNameSpaceRef(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> Names) :
	ASTIdentifier(Loc, llvm::join(Names, "."), ASTRefKind::REF_NAMESPACE), Names(Names) {
}

ASTNameSpaceRef::~ASTNameSpaceRef() {

}

const llvm::SmallVector<llvm::StringRef, 4> &ASTNameSpaceRef::getNames() const {
	return Names;
}

std::string ASTNameSpaceRef::str() const {
	return Logger("ASTNameSpaceRe").
		   Attr("Name", Name).
		   End();
}