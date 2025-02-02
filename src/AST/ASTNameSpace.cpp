//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTNameSpace.h"

using namespace fly;

ASTNameSpace::ASTNameSpace(const SourceLocation &Loc, llvm::StringRef Name) :
    ASTBase(Loc, ASTKind::AST_NAMESPACE), Name(Name) {
}

ASTNameSpace::~ASTNameSpace() {

}

llvm::StringRef ASTNameSpace::getName() const {
	return Name;
}

std::string ASTNameSpace::str() const {
    return Logger("ASTNameSpaceRe").
           Attr("Name", Name).
           End();
}
