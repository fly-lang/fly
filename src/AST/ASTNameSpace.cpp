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
    ASTIdentifier(Loc, Name, ASTIdentifierKind::REF_NAMESPACE) {
}

ASTNameSpace::~ASTNameSpace() {

}

ASTModule * ASTNameSpace::getModule() const {
	return Module;
}

std::string ASTNameSpace::str() const {
    return Logger("ASTNameSpace").
           Attr("Name", Name).
           End();
}
